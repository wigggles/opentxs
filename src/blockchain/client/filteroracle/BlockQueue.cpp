// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <type_traits>

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::client::implementation::FilterOracle::BlockQueue::"

namespace opentxs::blockchain::client::implementation
{
constexpr auto database_batch_{100u};

FilterOracle::BlockQueue::BlockQueue(
    const api::Core& api,
    const internal::FilterDatabase& db,
    const internal::HeaderOracle& header,
    const FilterOracle& parent,
    const blockchain::Type chain,
    const filter::Type type) noexcept
    : have_received_blocks_(false)
    , jobs_(0)
    , calculate_headers_job_(false)
    , api_(api)
    , db_(db)
    , header_(header)
    , parent_(parent)
    , chain_(chain)
    , type_(type)
    , download_limit_(download_batch_ * 2u)
    , buffer_lock_()
    , running_(true)
    , downloading_(0)
    , downloaded_(0)
    , completed_(0)
    , highest_requested_(-1)
    , highest_completed_(make_blank<block::Position>::value(api_))
    , buffer_(std::max(download_batch_, database_batch_) * 2u)
{
}

auto FilterOracle::BlockQueue::Add(
    std::vector<block::pHash>& hashes,
    BlockOracle::BitcoinBlockFutures&& futures,
    block::Height height) noexcept -> void
{
    Lock lock(buffer_lock_);

    if (false == running_) { return; }

    OT_ASSERT(hashes.size() == futures.size());
    OT_ASSERT(buffer_.size() < buffer_.capacity());

    auto previousHeader = IndexBlockJob::HeaderFuture{};

    if (0 == buffer_.size()) {
        const auto& firstHash = *hashes.begin();
        const auto pBlock = header_.LoadHeader(firstHash);

        OT_ASSERT(pBlock);

        const auto& blockHeader = *pBlock;
        auto promise = std::promise<block::pHash>{};
        promise.set_value(
            db_.LoadFilterHeader(type_, blockHeader.ParentHash().Bytes()));
        previousHeader = promise.get_future();
    } else {
        const auto& previous = *buffer_.rbegin();

        OT_ASSERT(previous.position_.first + 1 == height);

        previousHeader = previous.future();
    }

    auto hash = hashes.begin();
    auto future = futures.begin();

    for (; hash != hashes.end(); ++hash, ++future, ++height) {
        buffer_.push_back(IndexBlockJob{
            this,
            {height, std::move(*hash)},
            std::move(previousHeader),
            std::move(*future)});
        ++downloading_;
        auto& last = *buffer_.rbegin();
        previousHeader = last.future();
    }

    highest_requested_ = std::max(highest_requested_, height);
}

auto FilterOracle::BlockQueue::begin(const std::size_t index) noexcept
    -> Buffer::iterator
{
    Lock lock(buffer_lock_);

    return begin(lock, index);
}

auto FilterOracle::BlockQueue::begin(
    const Lock& lock,
    const std::size_t index) noexcept -> Buffer::iterator
{
    auto output = buffer_.begin();
    std::advance(output, index);

    return output;
}

auto FilterOracle::BlockQueue::CalculateHeaders(const zmq::Message& in) noexcept
    -> void
{
    Lock lock(buffer_lock_);

    if (0 < buffer_.size()) {
        calculate_headers(lock, begin(lock), std::nullopt);
    }
}

auto FilterOracle::BlockQueue::calculate_headers(
    const Lock& lock,
    Buffer::iterator it,
    std::optional<Buffer::iterator> lastGood) noexcept -> void
{
    for (; end(lock, it); ++it) {
        if (it->running_.exchange(true)) {
            // Another thread is already working on this job

            break;
        }

        auto postcondition = ScopeGuard{[&]() { it->running_.store(false); }};

        if (it->hasHeader()) {
            lastGood = std::make_optional<>(it);
        } else if (it->Evaluate(completed_)) {
            lastGood = std::make_optional<>(it);
        } else {
            break;
        }
    }

    if (lastGood.has_value()) {
        const auto& position = lastGood.value()->position_;

        if (position.first > highest_completed_.first) {
            highest_completed_ = lastGood.value()->position_;
            parent_.trigger();
        }
    }
}

auto FilterOracle::BlockQueue::check_block_futures() noexcept -> void
{
    auto it = buffer_.begin();
    auto index{downloaded_};
    std::advance(it, index);
    auto contiguous{true};

    for (; it != buffer_.end(); ++it, ++index) {
        auto& job = *it;
        LogTrace(OT_METHOD)(__FUNCTION__)(": Examining block at height: ")(
            job.position_.first)
            .Flush();

        if (job.running_) {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Block is currently being indexed")
                .Flush();
        } else if (false == job.hasBlock()) {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Block is downloading").Flush();
            contiguous = false;
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Queuing filter calculation job for block at height ")(
                it->position_.first)(" and index ")(index)
                .Flush();
            have_received_blocks_ = true;
            job.running_ = true;
            queue_process_block(index);
        }

        if (contiguous) {
            ++downloaded_;
            LogTrace(OT_METHOD)(__FUNCTION__)(": ")(downloaded_)(
                " contiguous blocks downloaded")
                .Flush();
        }
    }
}

auto FilterOracle::BlockQueue::DownloadCapacity() const noexcept -> std::size_t
{
    if (downloading_ >= download_limit_) { return 0; }

    return std::min(
        (download_limit_ - downloading_),
        (buffer_.capacity() - buffer_.size()));
}

auto FilterOracle::BlockQueue::end(const Buffer::iterator& it) noexcept -> bool
{
    Lock lock(buffer_lock_);

    return end(lock, it);
}

auto FilterOracle::BlockQueue::end(
    const Lock& lock,
    const Buffer::iterator& it) noexcept -> bool
{
    return it != buffer_.end();
}

auto FilterOracle::BlockQueue::flush(const std::size_t items) noexcept
    -> block::Position
{
    wait_for_jobs();
    Lock lock(buffer_lock_);
    auto blockHashes = std::vector<OTData>{};
    auto filterHashes = std::vector<OTData>{};
    auto headers = std::vector<internal::FilterDatabase::Header>{};
    auto filters = std::vector<internal::FilterDatabase::Filter>{};
    auto it{buffer_.begin()};
    auto position{it->position_};

    for (; blockHashes.size() < items; it = buffer_.erase(it)) {
        OT_ASSERT(it != buffer_.end());

        position = it->position_;

        OT_ASSERT(it->complete_);

        const auto& blockHash =
            blockHashes.emplace_back(std::move(it->position_.second)).get();
        auto& gcs = it->GetFilter();
        const auto& filterHash = filterHashes.emplace_back(gcs->Hash());
        const auto& header = it->GetHeader();
        headers.emplace_back(blockHash, header, filterHash->Bytes());
        filters.emplace_back(blockHash.Bytes(), std::move(gcs));
    }

    OT_ASSERT(blockHashes.size() == items);
    OT_ASSERT(filterHashes.size() == items);
    OT_ASSERT(headers.size() == items);
    OT_ASSERT(filters.size() == items);

    const auto saved = db_.StoreFilters(type_, headers, filters, position);

    OT_ASSERT(saved);

    OT_ASSERT(downloaded_ >= items);
    OT_ASSERT(completed_ >= items);

    downloaded_ -= items;
    completed_ -= items;
    parent_.notify_new_filter(type_, position);

    return position;
}

auto FilterOracle::BlockQueue::IndexBlock(const std::size_t index) noexcept
    -> void
{
    auto it = begin(index);
    auto postCondition = ScopeGuard{[it]() { it->running_ = false; }};
    LogTrace(OT_METHOD)(__FUNCTION__)(
        ": Calculating filter for block at height ")(it->position_.first)(
        " and index ")(index)
        .Flush();

    OT_ASSERT(it->hasBlock());

    // This function can sometimes be called more than one for the same job
    // apparently. Make sure downloading_ is only decremented once per job.

    if (false == it->hasFilter()) {
        OT_ASSERT(0 < downloading_);

        --downloading_;
    }

    if (false == it->Evaluate(completed_)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": Filter calculation complete for block at height ")(
            it->position_.first)(" but previous header is not available yet")
            .Flush();

        return;
    }

    // NOTE blocks might be downloaded out of order. Calculating the GCS filter
    // can happen in any order, but filter headers depend on the value of the
    // previous filter header so they must be calculated in strict block height
    // sequence.
    //
    // After each block is downloaded check to see if the next queued block
    // was waiting on the filter header that was just calculated and if so
    // calculate its header. Continue until no more blocks are available.

    Lock lock(buffer_lock_);
    auto lastGood{it};
    ++it;
    calculate_headers(lock, it, lastGood);
}

auto FilterOracle::BlockQueue::queue_calculate_headers() noexcept -> void
{
    if (false == calculate_headers_job_.exchange(true)) {
        queue_work(Work::calculate_headers);
    }
}

auto FilterOracle::BlockQueue::queue_process_block(
    const std::size_t index) noexcept -> void
{
    queue_work(Work::index_block, index);
}

auto FilterOracle::BlockQueue::queue_work(
    const Work type,
    const std::size_t index) noexcept -> void
{
    using Pool = internal::ThreadPool;

    auto work = Pool::MakeWork(api_, chain_, Pool::Work::FilterOracle);
    work->AddFrame(type);
    work->AddFrame(reinterpret_cast<std::uintptr_t>(this));
    work->AddFrame(index);
    parent_.thread_pool().Send(work);
    ++jobs_;
}

auto FilterOracle::BlockQueue::ready() const noexcept
    -> std::pair<std::size_t, block::Position>
{
    Lock lock(buffer_lock_);

    return std::make_pair<>(
        std::min(downloaded_, completed_.load()), highest_completed_);
}

auto FilterOracle::BlockQueue::Reset() noexcept -> void
{
    wait_for_jobs();
    Lock lock(buffer_lock_);
    downloading_.store(0);
    downloaded_ = 0;
    completed_ = 0;
    highest_requested_ = -1;
    highest_completed_ = make_blank<block::Position>::value(api_);
    buffer_.clear();
}

auto FilterOracle::BlockQueue::Run() noexcept -> void
{
    {
        Lock lock(buffer_lock_);

        if (false == running_) { return; }
    }

    check_block_futures();
    write_buffer();
}

auto FilterOracle::BlockQueue::wait_for_jobs() const noexcept -> void
{
    while (0 < jobs_.load()) { Sleep(std::chrono::milliseconds(1)); }
}

auto FilterOracle::BlockQueue::write_buffer() noexcept -> void
{
    const auto [count, finalPosition] = ready();
    const auto best = parent_.header_.BestChain();
    const auto batchComplete = (count >= database_batch_);
    const auto caughtUp = finalPosition.first >= best.first;
    const auto write = batchComplete || caughtUp;

    if (write) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Flushing ")(count)(" blocks")
            .Flush();
        const auto position = flush(count);
        LogNormal(DisplayString(chain_))(
            " filter chain and filter header chain updated to height ")(
            position.first)
            .Flush();
    }

    if (0 < buffer_.size()) { queue_calculate_headers(); }
}

FilterOracle::BlockQueue::~BlockQueue()
{
    {
        Lock lock(buffer_lock_);
        running_ = false;
    }

    wait_for_jobs();
}
}  // namespace opentxs::blockchain::client::implementation
