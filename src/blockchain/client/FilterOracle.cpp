// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/Proto.tpp"

#include "core/Shutdown.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <mutex>

#include "FilterOracle.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::FilterOracle::"

namespace opentxs
{
blockchain::client::internal::FilterOracle* Factory::BlockchainFilterOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::FilterDatabase& database,
    const std::string& shutdown)
{
    using ReturnType = blockchain::client::implementation::FilterOracle;

    return new ReturnType(api, network, database, shutdown);
}
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
const std::chrono::seconds FilterOracle::RequestQueue::limit_{10};

FilterOracle::FilterOracle(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::FilterDatabase& database,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , StateMachine(std::bind(&FilterOracle::state_machine, this))
    , api_(api)
    , network_(network)
    , database_(database)
    , running_(Flag::Factory(true))
    , lock_()
    , requests_(api_)
    , new_filters_(api_.ZeroMQ().Pipeline(
          api_,
          [this](auto& in) { process_cfilter(in); }))
    , shutdown_(
          api.ZeroMQ(),
          {api.Endpoints().Shutdown(), shutdown},
          [this](auto& promise) { this->shutdown(promise); })
{
}

FilterOracle::RequestQueue::RequestQueue(const api::Core& api) noexcept
    : lock_()
    , highest_(make_blank<block::Position>::value(api))
    , hashes_()
{
}

void FilterOracle::RequestQueue::Finish(const block::Hash& block) noexcept
{
    Lock lock(lock_);
    prune(lock);
    hashes_.erase(block);
}

bool FilterOracle::RequestQueue::IsRunning(const block::Hash& block) const
    noexcept
{
    Lock lock(lock_);
    prune(lock);

    return 0 < hashes_.count(block);
}

void FilterOracle::RequestQueue::prune(const Lock& lock) const noexcept
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

std::size_t FilterOracle::RequestQueue::size() const noexcept
{
    Lock lock(lock_);
    prune(lock);

    return hashes_.size();
}

void FilterOracle::RequestQueue::Start(const block::Hash& block) noexcept
{
    Lock lock(lock_);
    prune(lock);
    hashes_.emplace(block, Clock::now());
}

void FilterOracle::AddFilter(
    const filter::Type type,
    const block::Hash& block,
    const Data& filter) const noexcept
{
    if (false == running_.get()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(block);
    work->AddFrame(filter);
    new_filters_->Push(work);
}

void FilterOracle::CheckBlocks() const noexcept
{
    if (false == running_.get()) { return; }

    Trigger();
}

void FilterOracle::Start() noexcept
{
    if (false == running_.get()) { return; }

    Trigger();
}

bool FilterOracle::have_all_filters(
    const filter::Type type,
    const block::Position checkpoint,
    const block::Hash& block) const noexcept
{
    const auto pHeader = network_.HeaderOracle().LoadHeader(block);

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load block header ")(
            block.asHex())
            .Flush();

        return false;
    }

    const auto& header = *pHeader;

    if (false == database_.HaveFilter(type, block)) { return false; }

    const auto& [targetHeight, requredHash] = checkpoint;
    const bool specialCase = (0 > checkpoint.first) && (0 == header.Height());

    if (specialCase) { return true; }

    if (header.Height() > targetHeight) {

        return have_all_filters(type, checkpoint, header.ParentHash());
    } else {
        return requredHash == block;
    }
}

void FilterOracle::process_cfilter(const zmq::Message& in) noexcept
{
    if (false == running_.get()) { return; }

    const auto& body = in.Body();

    if (3 != body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto type = body.at(0).as<filter::Type>();
    const auto block = Data::Factory(body.at(1));
    const auto serializedFilter = proto::Factory<proto::GCS>(body.at(2));
    Lock lock(lock_);
    requests_.Finish(block);

    std::unique_ptr<const blockchain::internal::GCS> gcs{
        Factory::GCS(api_, serializedFilter)};

    if (false == bool(gcs)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid GCS").Flush();

        return;
    }

    const auto pHeader = network_.HeaderOracle().LoadHeader(block);

    if (false == bool(pHeader)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load block header ")(
            block->asHex())
            .Flush();

        return;
    }

    if (false == database_.StoreFilter(type, block, std::move(gcs))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error").Flush();

        return;
    }

    const auto& header = *pHeader;
    const auto checkpoint = database_.CurrentTip(type);

    if (have_all_filters(type, checkpoint, block)) {
        database_.SetTip(type, header.Position());
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter chain updated to block ")(header.Hash().asHex())(
            " at height ")(header.Height())
            .Flush();
    } else {
        LogNormal(blockchain::internal::DisplayString(network_.Chain()))(
            " filter for block ")(header.Hash().asHex())(" at height ")(
            header.Height())(" saved")
            .Flush();
    }

    Trigger();
}

std::shared_future<void> FilterOracle::Shutdown() noexcept
{
    shutdown_.Close();

    if (running_.get()) { shutdown(shutdown_.promise_); }

    return shutdown_.future_;
}

void FilterOracle::shutdown(std::promise<void>& promise) noexcept
{
    running_->Off();
    Stop().get();
    new_filters_->Close();

    try {
        promise.set_value();
    } catch (...) {
    }
}

bool FilterOracle::state_machine() noexcept
{
    struct Cleanup {
        operator bool() { return repeat_; }

        void Off() { repeat_ = false; }

        Cleanup(std::mutex& lock)
            : rate_limit_(10)
            , lock_(lock)
            , repeat_(true)
        {
        }

        ~Cleanup()
        {
            lock_.unlock();

            if (repeat_) { Sleep(rate_limit_); }
        }

    private:
        const std::chrono::milliseconds rate_limit_;
        Lock lock_;
        bool repeat_;
    };

    Cleanup repeat(lock_);
    static const std::size_t requestLimit{99};
    static const std::size_t outstandingLimit{2000};
    static const auto type = filter::Type::Basic;
    const auto waiting = requests_.size();
    const auto current = database_.CurrentTip(type);

    if (outstandingLimit <= waiting) { return repeat; }

    const auto& headers = network_.HeaderOracle();
    const auto [start, best] = headers.CommonParent(current);

    if (start == best) {
        repeat.Off();

        return repeat;
    }

    if (start != current) { database_.SetTip(type, start); }

    OT_ASSERT(best.first > start.first);

    std::size_t requestCount{0};
    auto requestHeight{start.first};
    auto requestHash{make_blank<block::pHash>::value(api_)};
    auto nextHash{requestHash};

    for (auto i{requestHeight}; i <= best.first; ++i) {
        nextHash = headers.BestHash(i);

        if (nextHash->empty()) { return repeat; }

        const bool running = requests_.IsRunning(nextHash);
        const bool exists = database_.HaveFilter(type, nextHash);
        const bool skip = running || exists;
        const bool haveRequests = 0 < requestCount;

        if (skip) {
            if (false == haveRequests) { requestHeight = i + 1; }
        } else {
            ++requestCount;
            requestHash = nextHash;
            requests_.Start(requestHash);
        }

        const bool requestFull = requestCount == requestLimit;
        const bool send = requestFull || (haveRequests && skip);

        if (send) { break; }
    }

    if (0 < requestCount) {
        network_.RequestFilters(
            filter::Type::Basic, requestHeight, requestHash);
    }

    return repeat;
}

FilterOracle::~FilterOracle()
{
    Shutdown().get();
    shutdown_.Close();
}
}  // namespace opentxs::blockchain::client::implementation
