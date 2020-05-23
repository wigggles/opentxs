// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "Factory.hpp"
#include "blockchain/client/FilterCheckpoints.hpp"
#include "core/Executor.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::FilterOracle::"

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::FilterDatabase& database,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::FilterOracle>
{
    using ReturnType = blockchain::client::implementation::FilterOracle;

    return std::make_unique<ReturnType>(api, network, database, type, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
const std::chrono::seconds FilterOracle::FilterQueue::timeout_{20};
const std::chrono::seconds FilterOracle::RequestQueue::limit_{15};

FilterOracle::FilterOracle(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::FilterDatabase& database,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , Executor(api)
    , network_(network)
    , database_(database)
    , default_type_(blockchain::internal::DefaultFilter(type))
    , header_requests_(api_)
    , outstanding_filters_(api_)
    , socket_(api.ZeroMQ().PublishSocket())
    , init_promise_()
    , init_(init_promise_.get_future())
{
    auto zmq =
        socket_->Start(api.Endpoints().InternalBlockchainFilterUpdated(type));

    OT_ASSERT(zmq);

    init_executor({shutdown, api.Endpoints().BlockchainReorg()});
}

FilterOracle::FilterQueue::FilterQueue(const api::Core& api) noexcept
    : error_(false)
    , api_(api)
    , running_(false)
    , queued_(0)
    , filters_()
    , last_received_()
    , target_(make_blank<block::Position>::value(api))
{
    filters_.reserve(1000);
}

FilterOracle::RequestQueue::RequestQueue(const api::Core& api) noexcept
    : hashes_()
{
}

auto FilterOracle::FilterQueue::AddFilter(
    const block::Height height,
    const block::Hash& hash,
    std::unique_ptr<const blockchain::internal::GCS> filter) noexcept -> void
{
    OT_ASSERT(filter);

    const auto& target = target_.first;

    if (height > target) {
        LogVerbose(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Filter height (")(height)(") is after requested range")
            .Flush();

        return;
    }

    const auto offset = static_cast<std::size_t>(target - height);

    if (offset >= filters_.size()) {
        LogVerbose(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Filter height (")(height)(") is before requested range")
            .Flush();

        return;
    }

    auto it{filters_.begin()};
    std::advance(it, offset);

    if (hash != it->first) {
        LogVerbose(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Wrong block for filter at height ")(height)
            .Flush();

        return;
    }

    auto& cachedFilter = it->second;

    if (false == bool(cachedFilter)) {
        cachedFilter.reset(filter.release());
        ++queued_;
    }

    OT_ASSERT(cachedFilter);

    last_received_ = Clock::now();

    if ((height == target) && (queued_ < filters_.size())) {
        LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
            ": Last filter in range received, however only ")(queued_)(" of ")(
            filters_.size())(" filters are queued.")
            .Flush();

        if (filters_.rbegin()->second) {
            LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
                ": Accepting partial filter batch.")
                .Flush();
            queued_ = filters_.size();
        } else {
            error_ = true;
        }
    }
}

auto FilterOracle::FilterQueue::Flush(Filters& filters) noexcept
    -> block::Position
{
    auto counter = target_.first - filters_.size();

    for (auto it{filters_.rbegin()}; it != filters_.rend(); ++it) {
        ++counter;

        if (it->second) {
            filters.emplace_back(internal::FilterDatabase::Filter{
                it->first->Bytes(), std::move(it->second)});
        } else {
            LogOutput(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
                ": Filter for block ")(counter)(" is missing.")
                .Flush();

            OT_ASSERT(filters_.rbegin() != it);

            std::advance(it, -1);

            return {counter - 1, it->first};
        }
    }

    return target_;
}

auto FilterOracle::FilterQueue::IsFull() noexcept -> bool
{
    return filters_.size() == queued_;
}

auto FilterOracle::FilterQueue::IsRunning() noexcept -> bool
{
    if (error_) { Reset(); }

    if (running_) {
        if ((Clock::now() - last_received_) > timeout_) {
            LogVerbose(OT_METHOD)("FilterQueue::")(__FUNCTION__)(
                ": Filter request timed out after ")(timeout_.count())(
                " seconds ")
                .Flush();
            Reset();
        }
    }

    return running_;
}

auto FilterOracle::FilterQueue::Queue(
    const block::Height startHeight,
    const block::Hash& stopHash,
    const client::HeaderOracle& headers) noexcept -> void
{
    OT_ASSERT(false == running_);
    OT_ASSERT(0 == queued_);
    OT_ASSERT(0 == filters_.size());

    auto header = headers.LoadHeader(stopHash);

    OT_ASSERT(header);

    target_ = header->Position();
    filters_.emplace_back(FilterData{header->Hash(), nullptr});

    while (header->Height() > startHeight) {
        header = headers.LoadHeader(header->ParentHash());

        OT_ASSERT(header);

        filters_.emplace_back(FilterData{header->Hash(), nullptr});
    }

    running_ = true;
    last_received_ = Clock::now();
}

auto FilterOracle::FilterQueue::Reset() noexcept -> void
{
    running_ = false;
    queued_ = 0;
    filters_.clear();
    last_received_ = Time{};
    target_ = make_blank<block::Position>::value(api_);
}

auto FilterOracle::RequestQueue::Finish(const block::Hash& block) noexcept
    -> void
{
    prune();
    hashes_.erase(block);
}

auto FilterOracle::RequestQueue::IsRunning(const block::Hash& block) noexcept
    -> bool
{
    prune();

    return 0 < hashes_.count(block);
}

auto FilterOracle::RequestQueue::prune() const noexcept -> void
{
    for (auto i = hashes_.begin(); i != hashes_.end();) {
        const auto duration = Clock::now() - i->second;

        if ((duration > limit_) || (std::chrono::seconds(0) > duration)) {
            i = hashes_.erase(i);
        } else {
            ++i;
        }
    }
}

auto FilterOracle::RequestQueue::Start(const block::Hash& block) noexcept
    -> void
{
    prune();
    hashes_.emplace(block, Clock::now());
}

auto FilterOracle::AddFilter(zmq::Message& work) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto header = work.Header();

    if (1 > header.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Invalid work header").Flush();

        return;
    }

    header.Replace(0, api_.ZeroMQ().Frame(Work::cfilter));
    pipeline_->Push(work);
}

auto FilterOracle::AddHeaders(zmq::Message& work) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto header = work.Header();

    if (1 > header.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Invalid work header").Flush();

        return;
    }

    header.Replace(0, api_.ZeroMQ().Frame(Work::cfheader));
    pipeline_->Push(work);
}

auto FilterOracle::CheckBlocks() const noexcept -> void
{
    if (false == running_.get()) { return; }

    Trigger();
}

void FilterOracle::check_filters(
    const filter::Type type,
    const block::Height maxRequests,
    Cleanup& repeat) noexcept
{
    const auto& headers = network_.HeaderOracle();
    const auto current = database_.FilterTip(type);
    const auto floor = oldest_checkpoint_before(current.first);
    const auto [start, best] = headers.CommonParent(current);

    if (start == best) {
        LogInsane(blockchain::internal::DisplayString(network_.Chain()))(
            " filter chain is caught up to block header chain")
            .Flush();
        return;
    }

    if (current.first >= floor) {
        OT_ASSERT(start.first >= floor);  // TODO
    }

    repeat.On();

    if (outstanding_filters_.IsRunning()) { return; }

    const auto headerTip = database_.FilterHeaderTip(type).first;
    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight =
        outstanding_filters_.error_
            ? begin
            : std::min(std::min(target, headerTip), best.first);
    outstanding_filters_.error_ = false;

    if (0 > (stopHeight - begin)) { return; }

    LogNormal("Requesting ")(blockchain::internal::DisplayString(
        network_.Chain()))(" filters from ")(begin)(" to ")(stopHeight)
        .Flush();
    const auto stopHash = headers.BestHash(stopHeight);
    outstanding_filters_.Queue(begin, stopHash, headers);
    network_.RequestFilters(type, begin, stopHash);
}

void FilterOracle::check_headers(
    const filter::Type type,
    const block::Height maxRequests,
    Cleanup& repeat) noexcept
{
    const auto& headers = network_.HeaderOracle();
    const auto [start, best] =
        headers.CommonParent(database_.FilterHeaderTip(type));

    if (start == best) {
        LogInsane(blockchain::internal::DisplayString(network_.Chain()))(
            " filter header chain is caught up to block header chain")
            .Flush();
        repeat.Off();

        return;
    }

    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight = std::min(target, best.first);
    const auto stopHash = headers.BestHash(stopHeight);

    if (header_requests_.IsRunning(stopHash)) { return; }

    LogNormal("Requesting ")(blockchain::internal::DisplayString(
        network_.Chain()))(" filter headers from ")(begin)(" to ")(stopHeight)
        .Flush();
    header_requests_.Start(stopHash);
    network_.RequestFilterHeaders(type, begin, stopHash);
}

auto FilterOracle::compare_tips_to_checkpoint() noexcept -> void
{
    const auto& cp = filter_checkpoints_.at(network_.Chain());
    auto checkPosition{database_.FilterHeaderTip(default_type_)};
    auto changed{false};

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        const auto& cpHeight = i->first;

        if (cpHeight > checkPosition.first) { continue; }

        checkPosition = block::Position{
            cpHeight, network_.HeaderOracle().BestHash(cpHeight)};
        const auto existingHeader = database_.LoadFilterHeader(
            default_type_, checkPosition.second->Bytes());
        const auto& cpHeader = i->second.at(default_type_);
        const auto cpBytes =
            api_.Factory().Data(ReadView{cpHeader.data(), cpHeader.size()});

        if (existingHeader == cpBytes) { break; }

        changed = true;
    }

    if (changed) {
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter header chain did not match checkpoint. Resetting to last "
            "known good position")
            .Flush();
        reset_tips_to(checkPosition);
    } else {
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter header chain matched checkpoint")
            .Flush();
    }
}

auto FilterOracle::compare_tips_to_header_chain() noexcept -> bool
{
    const auto current = database_.FilterHeaderTip(default_type_);
    const auto [parent, best] = network_.HeaderOracle().CommonParent(current);

    if ((parent.first == current.first) && (parent.second == current.second)) {
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter header chain is following the best chain")
            .Flush();

        return false;
    }

    LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
        " filter header chain is following a sibling chain. Resetting to "
        "common ancestor at height ")(best.first)
        .Flush();
    reset_tips_to(parent);

    return true;
}

auto FilterOracle::oldest_checkpoint_before(const block::Height height) const
    noexcept -> block::Height
{
    const auto& cp = filter_checkpoints_.at(network_.Chain());

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        if (i->first < height) { return i->first; }
    }

    return height;
}

auto FilterOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    init_.get();

    if (false == running_.get()) { return; }

    const auto header = in.Header();

    if (1 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = header.at(0).as<Work>();

    switch (work) {
        case Work::cfilter: {
            process_cfilter(in);
        } break;
        case Work::cfheader: {
            process_cfheader(in);
        } break;
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::statemachine: {
            try {
                state_machine_.set_value(request());
            } catch (...) {
            }
        } break;
        case Work::shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto FilterOracle::process_cfheader(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    auto type = filter::Type{};
    auto stopBlock = ReadView{};
    auto previousHeader = ReadView{};
    auto hashes = std::vector<ReadView>{};

    {
        auto counter{0};
        const auto body = in.Body();

        if (body.size() < 3) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cfheader").Flush();

            return;
        }

        for (const auto& frame : in.Body()) {
            switch (++counter) {
                case 1: {
                    type = frame.as<filter::Type>();
                } break;
                case 2: {
                    stopBlock = frame.Bytes();
                } break;
                case 3: {
                    previousHeader = frame.Bytes();
                } break;
                default: {
                    hashes.emplace_back(frame.Bytes());
                }
            }
        }
    }

    const auto stopHash = api_.Factory().Data(stopBlock);
    header_requests_.Finish(stopHash);
    const auto& headers = network_.HeaderOracle();
    const auto pHeader = headers.LoadHeader(stopHash);

    if (false == bool(pHeader)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Block header for ")(
            stopHash->asHex())(" not found")
            .Flush();

        return;
    }

    const auto& header = *pHeader;

    if (0 > header.Height()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Block ")(stopHash->asHex())(
            " is disconnected")
            .Flush();

        return;
    }

    const auto start =
        header.Height() - (static_cast<block::Height>(hashes.size()) - 1);

    if (0 > start) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Too many hashes provided")
            .Flush();

        return;
    }

    const auto [previousHeight, previousHash] = database_.FilterHeaderTip(type);

    if (header.Height() <= previousHeight) { return; }

    const auto previous =
        database_.LoadFilterHeader(type, previousHash->Bytes());
    auto priorFilter = previous->Bytes();

    if (0 == start) {
        priorFilter = {};
    } else {
        const auto compare = api_.Factory().Data(previousHeader);

        if (compare.get() != previous) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Invalid previous header")
                .Flush();

            return;
        }
    }

    auto output = std::vector<internal::FilterDatabase::Header>{};
    auto n = int{-1};
    const auto& cp = filter_checkpoints_.at(network_.Chain());

    for (auto i{start}; i <= header.Height(); ++i) {
        const auto hash = hashes.at(++n);
        auto receivedHeader =
            blockchain::internal::FilterHashToHeader(api_, hash, priorFilter);

        {
            auto it = cp.find(i);

            if (cp.end() != it) {
                const auto& bytes = it->second.at(default_type_);
                const auto expectedHeader =
                    api_.Factory().Data(ReadView{bytes.data(), bytes.size()});

                if (expectedHeader == receivedHeader) {
                    LogNormal(blockchain::internal::DisplayString(
                        network_.Chain()))(" filter header at height ")(i)(
                        " verified against checkpoint")
                        .Flush();
                } else {
                    std::advance(it, -1);
                    const auto rollback = block::Position{
                        it->first, network_.HeaderOracle().BestHash(it->first)};
                    LogNormal(blockchain::internal::DisplayString(
                        network_.Chain()))(" filter header at height ")(i)(
                        " does not match checkpoint. Resetting to previous "
                        "checkpoint at height ")(rollback.first)
                        .Flush();
                    header_requests_.Reset();
                    outstanding_filters_.Reset();
                    reset_tips_to(rollback);

                    return;
                }
            }
        }

        const auto& row = output.emplace_back(internal::FilterDatabase::Header{
            headers.BestHash(i), std::move(receivedHeader), hash});
        priorFilter = std::get<1>(row)->Bytes();
    }

    if (false == headers.IsInBestChain(stopHash)) { return; }

    if (database_.StoreFilterHeaders(type, previousHeader, std::move(output))) {
        if (database_.SetFilterHeaderTip(type, header.Position())) {
            LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
                " filter header chain updated to height ")(header.Height())
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed updating filter header tip")
                .Flush();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed saving filter headers")
            .Flush();
    }
}

auto FilterOracle::process_cfilter(const zmq::Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (6 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = body.at(0).as<filter::Type>();
    const auto block = Data::Factory(body.at(1));
    const auto bits = body.at(2).as<std::uint8_t>();
    const auto fpRate = body.at(3).as<std::uint32_t>();
    const auto count = body.at(4).as<std::uint32_t>();
    const auto bytes = body.at(5).Bytes();
    auto gcs = std::unique_ptr<const blockchain::internal::GCS>{Factory::GCS(
        api_,
        bits,
        fpRate,
        blockchain::internal::BlockHashToFilterKey(block->Bytes()),
        count,
        bytes)};

    if (false == bool(gcs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid GCS").Flush();

        return;
    }

    const auto pHeader = network_.HeaderOracle().LoadHeader(block);
    const auto& header = *pHeader;

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load block header ")(
            block->asHex())
            .Flush();

        return;
    }

    const auto hash = gcs->Hash();
    const auto expected = database_.LoadFilterHash(type, block->Bytes());

    if (hash != expected) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Filter for block ")(
            block->asHex())(" at height ")(header.Height())(
            " does not match header. Received: ")(hash->asHex())(" expected: ")(
            expected->asHex())
            .Flush();
        outstanding_filters_.Reset();

        return;
    }

    outstanding_filters_.AddFilter(
        header.Height(), header.Hash(), std::move(gcs));

    if (outstanding_filters_.IsFull()) {
        struct Cleanup {
            FilterOracle& this_;

            Cleanup(FilterOracle& parent)
                : this_(parent)
            {
            }

            ~Cleanup() { this_.outstanding_filters_.Reset(); }
        };

        auto cleanup = Cleanup{*this};
        auto filters = std::vector<internal::FilterDatabase::Filter>{};
        auto position = outstanding_filters_.Flush(filters);

        if (false == database_.StoreFilters(type, std::move(filters))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();
            Trigger();

            return;
        }

        database_.SetFilterTip(type, position);
        auto work = MakeWork(OTZMQWorkType{OT_ZMQ_NEW_FILTER_SIGNAL});
        work->AddFrame(default_type_);
        work->AddFrame(position.first);
        work->AddFrame(position.second);
        socket_->Send(work);
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter chain updated to height ")(header.Height())
            .Flush();
    } else {
        LogVerbose(blockchain::internal::DisplayString(network_.Chain()))(
            " filter for block ")(header.Hash().asHex())(" at height ")(
            header.Height())(" cached")
            .Flush();
    }

    Trigger();
}

auto FilterOracle::process_reorg(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (3 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto chain = body.at(0).as<blockchain::Type>();

    if (network_.Chain() != chain) { return; }

    header_requests_.Reset();
    outstanding_filters_.Reset();
    const auto parent =
        block::Position{body.at(2).as<block::Height>(),
                        api_.Factory().Data(body.at(1).Bytes())};
    reset_tips_to(parent);
    LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
        " filter chain tips reset to reorg parent ")(parent.second->asHex())(
        " at height ")(parent.first)
        .Flush();
    request();
}

auto FilterOracle::request() noexcept -> bool
{
    auto repeat = Cleanup{};
    check_headers(default_type_, 2000, repeat);
    check_filters(default_type_, 1000, repeat);

    return repeat;
}

auto FilterOracle::reset_tips_to(const block::Position position) noexcept
    -> bool
{
    const auto& [targetHeight, targetHash] = position;
    database_.SetFilterHeaderTip(default_type_, position);
    const auto [currentHeight, currentHash] =
        database_.FilterTip(default_type_);

    if (currentHeight >= targetHeight) {
        database_.SetFilterTip(default_type_, position);
        auto work = MakeWork(OTZMQWorkType{OT_ZMQ_NEW_FILTER_SIGNAL});
        work->AddFrame(default_type_);
        work->AddFrame(targetHeight);
        work->AddFrame(targetHash);
        socket_->Send(work);

        return true;
    }

    return false;
}

auto FilterOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    if (running_->Off()) {
        try {
            state_machine_.set_value(false);
        } catch (...) {
        }

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto FilterOracle::Start() noexcept -> void
{
    compare_tips_to_header_chain();
    compare_tips_to_checkpoint();
    Trigger();
    init_promise_.set_value();
}

FilterOracle::~FilterOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
