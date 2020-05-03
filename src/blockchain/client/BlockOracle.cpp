// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/BlockOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <map>
#include <memory>
#include <type_traits>

#include "Factory.hpp"
#include "core/Executor.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api
}  // namespace opentxs

#define OT_METHOD "opentxs::blockchain::client::implementation::BlockOracle::"

namespace opentxs
{
auto Factory::BlockOracle(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::BlockOracle>
{
    using ReturnType = blockchain::client::implementation::BlockOracle;

    return std::make_unique<ReturnType>(api, network, type, shutdown);
}
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
const std::size_t BlockOracle::Cache::cache_limit_{16};
const std::chrono::seconds BlockOracle::Cache::download_timeout_{30};

BlockOracle::BlockOracle(
    const api::internal::Core& api,
    const internal::Network& network,
    [[maybe_unused]] const blockchain::Type type,
    const std::string& shutdown) noexcept
    : Executor(api)
    , network_(network)
    , init_promise_()
    , init_(init_promise_.get_future())
    , cache_(network_)
{
    init_executor({shutdown});
}

BlockOracle::Cache::Cache(const internal::Network& network) noexcept
    : network_(network)
    , lock_()
    , pending_()
    , completed_()
    , running_(true)
{
}

auto BlockOracle::Cache::download(const block::Hash& block) const noexcept
    -> bool
{
    return network_.RequestBlock(block);
}

auto BlockOracle::Cache::ReceiveBlock(const zmq::Frame& in) const noexcept
    -> void
{
    auto pBlock =
        Factory::BitcoinBlock(network_.API(), network_.Chain(), in.Bytes());

    if (false == bool(pBlock)) {
        LogOutput(OT_METHOD)("Cache::")(__FUNCTION__)(": Invalid block")
            .Flush();

        return;
    }

    Lock lock{lock_};
    auto& block = *pBlock;
    const auto& db = network_.DB();

    if (api::client::blockchain::BlockStorage::None != db.BlockPolicy()) {
        db.BlockStore(block);
    }

    const auto& id = block.ID();
    auto pending = pending_.find(id);

    if (pending_.end() == pending) {
        LogOutput(OT_METHOD)("Cache::")(__FUNCTION__)(
            ": Received block not in request list")
            .Flush();

        return;
    }

    auto& [time, promise, future, queued] = pending->second;
    promise.set_value(std::move(pBlock));
    completed_.emplace_back(id, std::move(future));
    pending_.erase(pending);
    LogVerbose(OT_METHOD)("Cache::")(__FUNCTION__)(": Cached block ")(
        id.asHex())
        .Flush();
}

auto BlockOracle::Cache::Request(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    Lock lock{lock_};

    if (false == running_) {
        auto promise = Promise{};
        promise.set_value(nullptr);

        return promise.get_future();
    }

    for (const auto& [hash, future] : completed_) {
        if (block == hash) { return future; }
    }

    {
        auto it = pending_.find(block);

        if (pending_.end() != it) {
            const auto& [time, promise, future, queued] = it->second;

            return future;
        }
    }

    const auto& db = network_.DB();

    if (auto pBlock = db.BlockLoadBitcoin(block); bool(pBlock)) {
        auto promise = Promise{};
        promise.set_value(std::move(pBlock));

        return completed_.emplace_back(block, promise.get_future()).second;
    }

    auto& [time, promise, future, queued] = pending_[block];
    time = Clock::now();
    future = promise.get_future();
    queued = download(block);

    return future;
}

auto BlockOracle::Cache::Shutdown() noexcept -> void
{
    Lock lock{lock_};

    if (running_) {
        running_ = false;
        completed_.clear();

        for (auto& [hash, item] : pending_) {
            auto& [time, promise, future, queued] = item;
            promise.set_value(nullptr);
        }

        pending_.clear();
    }
}

auto BlockOracle::Cache::StateMachine() const noexcept -> bool
{
    Lock lock{lock_};

    if (false == running_) { return false; }

    while (completed_.size() > cache_limit_) { completed_.pop_front(); }

    for (auto& [hash, item] : pending_) {
        auto& [time, promise, future, queued] = item;
        const auto now = Clock::now();
        const auto timeout = download_timeout_ <= (now - time);

        if (timeout || (false == queued)) {
            queued = download(hash);
            time = now;
        }
    }

    return 0 < pending_.size();
}

auto BlockOracle::Init() noexcept -> void
{
    init_promise_.set_value();
    Trigger();
}

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    auto output = cache_.Request(block);
    Trigger();

    return output;
}

auto BlockOracle::pipeline(const zmq::Message& in) noexcept -> void
{
    init_.get();

    if (false == running_.get()) { return; }

    const auto header = in.Header();
    const auto body = in.Body();

    if (1 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    switch (header.at(0).as<Task>()) {
        case Task::ProcessBlock: {
            if (1 > body.size()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": No block").Flush();

                OT_FAIL;
            }

            cache_.ReceiveBlock(body.at(0));
        } break;
        case Task::StateMachine: {
            state_machine();
        } break;
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockOracle::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        try {
            state_machine_.set_value(false);
        } catch (...) {
        }

        cache_.Shutdown();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto BlockOracle::state_machine() noexcept -> void
{
    static const auto rateLimit = std::chrono::milliseconds{10};

    auto repeat = cache_.StateMachine();
    Sleep(rateLimit);

    if (repeat) { Sleep(rateLimit); }

    try {
        state_machine_.set_value(repeat);
    } catch (...) {
    }
}

auto BlockOracle::SubmitBlock(const zmq::Frame& in) const noexcept -> void
{
    auto work = MakeWork(Task::ProcessBlock);
    work->AddFrame(in);
    pipeline_->Push(work);
}

BlockOracle::~BlockOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
