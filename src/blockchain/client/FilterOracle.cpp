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

#include "blockchain/client/filteroracle/FilterCheckpoints.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::FilterOracle::"

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::client::Manager& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::HeaderOracle& header,
    const blockchain::client::internal::FilterDatabase& database,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::FilterOracle>
{
    using ReturnType = blockchain::client::implementation::FilterOracle;

    return std::make_unique<ReturnType>(
        api, network, header, database, type, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
constexpr std::size_t max_block_requests_{16};

FilterOracle::FilterOracle(
    const api::client::Manager& api,
    const internal::Network& network,
    const internal::HeaderOracle& header,
    const internal::FilterDatabase& database,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , Worker(api, std::chrono::milliseconds{0})
    , network_(network)
    , header_(header)
    , database_(database)
    , chain_(chain)
    , full_mode_(
          api::client::blockchain::BlockStorage::All == database_.BlockPolicy())
    , default_type_(
          full_mode_ ? filter::Type::Extended_opentxs
                     : blockchain::internal::DefaultFilter(chain_))
    , header_requests_(api_)
    , outstanding_filters_(api_)
    , block_requests_(
          api,
          database_,
          header_,
          *this,
          chain_,
          default_type_,
          max_block_requests_)
    , socket_(api.ZeroMQ().PublishSocket())
    , init_promise_()
    , init_(init_promise_.get_future())
{
    auto zmq =
        socket_->Start(api.Endpoints().InternalBlockchainFilterUpdated(chain_));

    OT_ASSERT(zmq);

    init_executor(
        {shutdown,
         api.Endpoints().BlockchainReorg(),
         api.Endpoints().BlockchainPeer()});
}

auto FilterOracle::AddFilter(zmq::Message& work) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto body = work.Body();

    if (1 > body.size()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Invalid work").Flush();

        OT_FAIL;
    }

    body.Replace(0, api_.ZeroMQ().Frame(Work::cfilter));
    pipeline_->Push(work);
}

auto FilterOracle::AddHeaders(zmq::Message& work) const noexcept -> void
{
    if (false == running_.get()) { return; }

    auto body = work.Body();

    if (1 > body.size()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Invalid work").Flush();

        OT_FAIL;
    }

    body.Replace(0, api_.ZeroMQ().Frame(Work::cfheader));
    pipeline_->Push(work);
}

auto FilterOracle::check_blocks(
    const filter::Type type,
    const block::Height maxRequests,
    bool& repeat) noexcept -> void
{
    const auto& headers = header_;
    const auto current = database_.FilterTip(type);
    const auto [start, best] = headers.CommonParent(current);

    if (start == best) {
        LogVerbose(blockchain::internal::DisplayString(chain_))(
            " filter chain is caught up to block header chain")
            .Flush();
        return;
    }

    // repeat = true;
    const auto capacity = block_requests_.Capacity();

    if (0 == capacity) { return; }

    const auto startHeight{
        std::max<block::Height>(
            start.first, block_requests_.HighestRequested()) +
        static_cast<block::Height>(1)};
    const auto target{
        startHeight + std::min<block::Height>(maxRequests, capacity) -
        static_cast<block::Height>(1)};
    const auto stopHeight = std::min(target, best.first);

    OT_ASSERT(startHeight <= stopHeight);
    OT_ASSERT(startHeight > 0);

    LogVerbose("Requesting ")(blockchain::internal::DisplayString(chain_))(
        " blocks from ")(startHeight)(" to ")(stopHeight)
        .Flush();
    const std::size_t limit = 1u + stopHeight - startHeight;
    const auto hashes = headers.BestHashes(startHeight, limit);
    auto counter{startHeight - 1};

    for (const auto& hash : hashes) {
        block_requests_.Add(
            block::Position{++counter, hash},
            network_.BlockOracle().LoadBitcoin(hash));
    }
}

void FilterOracle::check_filters(
    const filter::Type type,
    const block::Height maxRequests,
    bool& repeat) noexcept
{
    const auto& headers = header_;
    const auto current = database_.FilterTip(type);
    const auto floor = oldest_checkpoint_before(current.first);
    const auto [start, best] = headers.CommonParent(current);

    if (start == best) {
        LogVerbose(blockchain::internal::DisplayString(chain_))(
            " filter chain is caught up to block header chain")
            .Flush();
        return;
    }

    if (current.first >= floor) {
        OT_ASSERT(start.first >= floor);  // TODO
    }

    // repeat = true;

    if (outstanding_filters_.IsRunning()) {
        if (outstanding_filters_.IsFull()) { flush_filters(); }

        return;
    }

    const auto headerTip = database_.FilterHeaderTip(type).first;
    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight =
        outstanding_filters_.error_
            ? begin
            : std::min(std::min(target, headerTip), best.first);
    outstanding_filters_.error_ = false;

    if (0 > (stopHeight - begin)) { return; }

    OT_ASSERT(0 < stopHeight);
    OT_ASSERT(stopHeight >= begin);

    LogVerbose("Requesting ")(blockchain::internal::DisplayString(chain_))(
        " filters from ")(begin)(" to ")(stopHeight)
        .Flush();
    const auto stopHash = headers.BestHash(stopHeight);
    outstanding_filters_.Queue(type, begin, stopHash, headers);
    network_.RequestFilters(type, begin, stopHash);
}

void FilterOracle::check_headers(
    const filter::Type type,
    const block::Height maxRequests,
    bool& repeat) noexcept
{
    const auto& headers = header_;
    const auto [start, best] =
        headers.CommonParent(database_.FilterHeaderTip(type));

    if (start == best) {
        LogVerbose(blockchain::internal::DisplayString(chain_))(
            " filter header chain is caught up to block header chain")
            .Flush();

        return;
    }

    // repeat = true;
    const auto begin{start.first + static_cast<block::Height>(1)};
    const auto target{begin + maxRequests - static_cast<block::Height>(1)};
    const auto stopHeight = std::min(target, best.first);
    const auto stopHash = headers.BestHash(stopHeight);

    if (header_requests_.IsRunning(stopHash)) { return; }

    OT_ASSERT(0 < stopHeight);
    OT_ASSERT(stopHeight >= begin);

    LogVerbose("Requesting ")(blockchain::internal::DisplayString(chain_))(
        " filter headers from ")(begin)(" to ")(stopHeight)
        .Flush();
    header_requests_.Start(stopHash);
    network_.RequestFilterHeaders(type, begin, stopHash);
}

auto FilterOracle::compare_tips_to_checkpoint() noexcept -> void
{
    const auto& cp = filter_checkpoints_.at(chain_);
    const auto headerTip = database_.FilterHeaderTip(default_type_);
    auto checkPosition{headerTip};
    auto changed{false};

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        const auto& cpHeight = i->first;

        if (cpHeight > checkPosition.first) { continue; }

        checkPosition = block::Position{cpHeight, header_.BestHash(cpHeight)};
        const auto existingHeader = database_.LoadFilterHeader(
            default_type_, checkPosition.second->Bytes());
        const auto& cpHeader = i->second.at(default_type_);
        const auto cpBytes =
            api_.Factory().Data(ReadView{cpHeader.data(), cpHeader.size()});

        if (existingHeader == cpBytes) { break; }

        changed = true;
    }

    if (changed) {
        LogNormal(blockchain::internal::DisplayString(chain_))(
            " filter header chain did not match checkpoint. Resetting to last "
            "known good position")
            .Flush();
        reset_tips_to(default_type_, headerTip, checkPosition, changed);
    } else {
        LogNormal(blockchain::internal::DisplayString(chain_))(
            " filter header chain matched checkpoint")
            .Flush();
    }
}

auto FilterOracle::compare_tips_to_header_chain() noexcept -> bool
{
    const auto current = database_.FilterHeaderTip(default_type_);
    const auto [parent, best] = header_.CommonParent(current);

    if ((parent.first == current.first) && (parent.second == current.second)) {
        LogNormal(blockchain::internal::DisplayString(chain_))(
            " filter header chain is following the best chain")
            .Flush();

        return false;
    }

    LogNormal(blockchain::internal::DisplayString(chain_))(
        " filter header chain is following a sibling chain. Resetting to "
        "common ancestor at height ")(parent.first)
        .Flush();
    reset_tips_to(default_type_, current, parent);

    return true;
}

auto FilterOracle::flush_filters() noexcept -> void
{
    auto postcondition = ScopeGuard{[&] { outstanding_filters_.Reset(); }};
    auto filters = std::vector<internal::FilterDatabase::Filter>{};
    auto position = outstanding_filters_.Flush(filters);
    const auto& type = outstanding_filters_.type_;

    if (false == database_.StoreFilters(type, std::move(filters))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();
        trigger();

        return;
    }

    database_.SetFilterTip(type, position);
    notify_new_filter(type, position);
    LogNormal(blockchain::internal::DisplayString(chain_))(
        " filter chain updated to height ")(position.first)
        .Flush();
}

auto FilterOracle::LoadFilterOrResetTip(
    const filter::Type type,
    const block::Position& position) const noexcept
    -> std::unique_ptr<const blockchain::internal::GCS>
{
    auto output = LoadFilter(type, position.second);

    if (output) { return output; }

    auto work = MakeWork(Work::reset_filter_tip);
    work->AddFrame(type);
    work->AddFrame(position.first);
    pipeline_->Push(work);

    return {};
}

auto FilterOracle::notify_new_filter(
    const filter::Type type,
    const block::Position& position) const noexcept -> void
{
    auto work = MakeWork(OT_ZMQ_NEW_FILTER_SIGNAL);
    work->AddFrame(type);
    work->AddFrame(position.first);
    work->AddFrame(position.second);
    socket_->Send(work);
}

auto FilterOracle::oldest_checkpoint_before(
    const block::Height height) const noexcept -> block::Height
{
    const auto& cp = filter_checkpoints_.at(chain_);

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        if (i->first < height) { return i->first; }
    }

    return height;
}

auto FilterOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    init_.get();

    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = body.at(0).as<Work>();

    switch (work) {
        case Work::cfilter: {
            if (process_cfilter(in)) { do_work(); }
        } break;
        case Work::cfheader: {
            process_cfheader(in);
            do_work();
        } break;
        case Work::reorg: {
            process_reorg(in);
            do_work();
        } break;
        case Work::peer:
        case Work::block:
        case Work::statemachine: {
            do_work();
        } break;
        case Work::reset_filter_tip: {
            process_reset_filter_tip(in);
            do_work();
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
        auto counter{-1};
        const auto body = in.Body();

        if (body.size() < 4) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid cfheader").Flush();

            return;
        }

        for (const auto& frame : in.Body()) {
            switch (++counter) {
                case 0: {
                } break;
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
    const auto pHeader = header_.LoadHeader(stopHash);

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
    const auto& cp = filter_checkpoints_.at(chain_);

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
                    LogNormal(blockchain::internal::DisplayString(chain_))(
                        " filter header at height ")(i)(
                        " verified against checkpoint")
                        .Flush();
                } else {
                    std::advance(it, -1);
                    const auto rollback =
                        block::Position{it->first, header_.BestHash(it->first)};
                    LogNormal(blockchain::internal::DisplayString(chain_))(
                        " filter header at height ")(i)(
                        " does not match checkpoint. Resetting to previous "
                        "checkpoint at height ")(rollback.first)
                        .Flush();
                    reset_tips_to(default_type_, rollback);

                    return;
                }
            }
        }

        const auto& row = output.emplace_back(internal::FilterDatabase::Header{
            header_.BestHash(i), std::move(receivedHeader), hash});
        priorFilter = std::get<1>(row)->Bytes();
    }

    if (false == header_.IsInBestChain(stopHash)) { return; }

    if (database_.StoreFilterHeaders(type, previousHeader, std::move(output))) {
        if (database_.SetFilterHeaderTip(type, header.Position())) {
            LogNormal(blockchain::internal::DisplayString(chain_))(
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

auto FilterOracle::process_cfilter(const zmq::Message& in) noexcept -> bool
{
    if (false == running_.get()) { return false; }

    const auto body = in.Body();

    if (7 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = body.at(1).as<filter::Type>();
    const auto block = Data::Factory(body.at(2));
    const auto bits = body.at(3).as<std::uint8_t>();
    const auto fpRate = body.at(4).as<std::uint32_t>();
    const auto count = body.at(5).as<std::uint32_t>();
    const auto bytes = body.at(6).Bytes();
    auto gcs = std::unique_ptr<const blockchain::internal::GCS>{factory::GCS(
        api_,
        bits,
        fpRate,
        blockchain::internal::BlockHashToFilterKey(block->Bytes()),
        count,
        bytes)};

    if (false == bool(gcs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid GCS").Flush();

        return false;
    }

    const auto pHeader = header_.LoadHeader(block);
    const auto& header = *pHeader;

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load block header ")(
            block->asHex())
            .Flush();

        return false;
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

        return false;
    }

    outstanding_filters_.AddFilter(
        type, header.Height(), header.Hash(), std::move(gcs));

    if (outstanding_filters_.IsFull()) {
        flush_filters();

        return true;
    } else {
        LogVerbose(blockchain::internal::DisplayString(chain_))(
            " filter for block ")(header.Hash().asHex())(" at height ")(
            header.Height())(" cached")
            .Flush();
    }

    return false;
}

auto FilterOracle::process_reorg(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (4 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto parent = block::Position{
        body.at(3).as<block::Height>(),
        api_.Factory().Data(body.at(2).Bytes())};
    process_reorg(parent);
}

auto FilterOracle::process_reorg(const block::Position& parent) noexcept -> void
{
    const auto headerTip = database_.FilterHeaderTip(default_type_);
    const auto resetHeaders = headerTip > parent;

    if (false == resetHeaders) { return; }

    const auto filterTip = database_.FilterTip(default_type_);
    const auto resetFilters = filterTip > parent;

    if (resetFilters) {
        reset_tips_to(
            default_type_,
            headerTip,
            filterTip,
            parent,
            resetHeaders,
            resetFilters);
        LogNormal(blockchain::internal::DisplayString(chain_))(
            " filter chain tips reset to reorg parent ")(
            parent.second->asHex())(" at height ")(parent.first)
            .Flush();
    }
}

auto FilterOracle::process_reset_filter_tip(const zmq::Message& in) noexcept
    -> void
{
    const auto body = in.Body();

    if (3 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = body.at(1).as<filter::Type>();
    const auto missing = body.at(2).as<block::Height>();

    OT_ASSERT(0 < missing);

    const auto parent = missing - 1;
    const auto hash = header_.BestHash(parent);

    OT_ASSERT(false == hash->empty());

    reset_tips_to(type, block::Position{parent, hash}, false, true);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& position,
    const std::optional<bool> resetHeader,
    const std::optional<bool> resetfilter) noexcept -> bool
{
    return reset_tips_to(
        type,
        database_.FilterHeaderTip(default_type_),
        database_.FilterTip(default_type_),
        position,
        resetHeader,
        resetfilter);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& position,
    const std::optional<bool> resetHeader) noexcept -> bool
{
    return reset_tips_to(
        type,
        headerTip,
        database_.FilterTip(default_type_),
        position,
        resetHeader);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& filterTip,
    const block::Position& position,
    std::optional<bool> resetHeader,
    std::optional<bool> resetfilter) noexcept -> bool
{
    auto counter{0};

    if (false == resetHeader.has_value()) {
        resetHeader = headerTip > position;
    }

    if (false == resetfilter.has_value()) {
        resetfilter = filterTip > position;
    }

    OT_ASSERT(resetHeader.has_value());
    OT_ASSERT(resetfilter.has_value());

    if (resetHeader.value()) {
        block_requests_.Reset();
        header_requests_.Reset();
        database_.SetFilterHeaderTip(type, position);
        ++counter;
    }

    if (resetfilter.value()) {
        block_requests_.Reset();
        outstanding_filters_.Reset();
        database_.SetFilterTip(type, position);
        notify_new_filter(type, position);
        ++counter;
    }

    return 0 < counter;
}

auto FilterOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        header_requests_.Reset();
        outstanding_filters_.Reset();
        block_requests_.Reset();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto FilterOracle::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    auto repeat{false};

    if (full_mode_) {
        block_requests_.Run(repeat);
        check_blocks(default_type_, max_block_requests_, repeat);
    } else {
        check_headers(default_type_, 2000, repeat);
        check_filters(default_type_, 1000, repeat);
    }

    // return repeat;

    return false;
}

auto FilterOracle::Start() noexcept -> void
{
    compare_tips_to_header_chain();
    compare_tips_to_checkpoint();
    init_promise_.set_value();
}

FilterOracle::~FilterOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
