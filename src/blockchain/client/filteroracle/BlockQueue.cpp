// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <type_traits>

#include "internal/blockchain/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/ScopeGuard.hpp"

// #define OT_METHOD
// "opentxs::blockchain::client::implementation::FilterOracle::BlockQueue::"

namespace opentxs::blockchain::client::implementation
{
FilterOracle::BlockQueue::BlockQueue(
    const api::client::Manager& api,
    const internal::FilterDatabase& db,
    const internal::HeaderOracle& header,
    const FilterOracle& parent,
    const blockchain::Type chain,
    const filter::Type type,
    const std::size_t limit) noexcept
    : api_(api)
    , db_(db)
    , header_(header)
    , parent_(parent)
    , chain_(chain)
    , type_(type)
    , limit_(limit)
    , cached_()
    , highest_(-1)
    , queue_()
{
}

auto FilterOracle::BlockQueue::Capacity() const noexcept -> std::size_t
{
    if (queue_.size() >= limit_) { return 0; }

    return limit_ - queue_.size();
}

auto FilterOracle::BlockQueue::Add(
    const block::Position& position,
    Future block) noexcept -> void
{
    highest_ = std::max(highest_, position.first);
    queue_.emplace(position, block);
}

auto FilterOracle::BlockQueue::previous_filter_header(
    const block::Position& current) const noexcept -> const Data&
{
    const auto& [currentHeight, currentHash] = current;

    if (cached_.has_value()) {
        const auto& [height, filter] = cached_.value();

        if ((height + 1) == currentHeight) {

            return filter;
        } else {
            cached_ = std::nullopt;
        }
    }

    const auto pHeader = header_.LoadHeader(currentHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    cached_ = Cached{
        currentHeight - 1,
        db_.LoadFilterHeader(type_, header.ParentHash().Bytes())};

    OT_ASSERT(cached_.has_value());

    return cached_.value().second;
}

auto FilterOracle::BlockQueue::process(
    const block::Position& position,
    Block blockP) const noexcept -> void
{
    // TODO error handling should consist of telling the parent object to
    // retry rather than crashing

    OT_ASSERT(blockP);

    const auto& block = *blockP;
    auto gcsP = factory::GCS(api_, type_, block);

    OT_ASSERT(gcsP);

    const auto& gcs = *gcsP;
    const auto& blockHash = position.second;
    const auto filterHash = gcs.Hash();
    const auto& previousHeader = previous_filter_header(position);
    auto header = blockchain::internal::FilterHashToHeader(
        api_, filterHash->Bytes(), previousHeader.Bytes());
    auto headers = std::vector<internal::FilterDatabase::Header>{};
    headers.emplace_back(blockHash, header, filterHash->Bytes());
    auto filters = std::vector<internal::FilterDatabase::Filter>{};
    filters.emplace_back(blockHash->Bytes(), std::move(gcsP));

    auto saved = db_.StoreFilterHeaders(
        type_, previousHeader.Bytes(), std::move(headers));

    OT_ASSERT(saved);

    saved = db_.StoreFilters(type_, std::move(filters));

    OT_ASSERT(saved);

    saved = db_.SetFilterHeaderTip(type_, position);

    OT_ASSERT(saved);

    saved = db_.SetFilterTip(type_, position);

    OT_ASSERT(saved);

    cached_ = Cached{position.first, std::move(header)};
    parent_.notify_new_filter(type_, position);
}

auto FilterOracle::BlockQueue::Reset() noexcept -> void
{
    highest_ = -1;
    cached_ = std::nullopt;
    auto blank = Queue{};
    queue_.swap(blank);
}

auto FilterOracle::BlockQueue::Run(bool& repeat) noexcept -> void
{
    constexpr auto time = std::chrono::milliseconds{1};
    auto postcondition = ScopeGuard{[&]() { repeat |= !queue_.empty(); }};
    auto finalPosition = make_blank<block::Position>::value(api_);

    while (false == queue_.empty()) {
        auto& [position, future] = queue_.front();

        if (std::future_status::ready != future.wait_for(time)) { return; }

        process(position, future.get());
        finalPosition = std::move(position);
        queue_.pop();
    }

    highest_ = -1;
    const auto& finalHeight = finalPosition.first;

    if (-1 != finalHeight) {
        LogNormal(blockchain::internal::DisplayString(chain_))(
            " filter chain and filter header chain updated to height ")(
            finalHeight)
            .Flush();
    }
}
}  // namespace opentxs::blockchain::client::implementation
