// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/BlockOracle.hpp"  // IWYU pragma: associated

#include <memory>

#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
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
namespace client
{
class Manager;
}  // namespace client
}  // namespace api
}  // namespace opentxs

#define OT_METHOD "opentxs::blockchain::client::implementation::BlockOracle::"

namespace opentxs::factory
{
auto BlockOracle(
    const api::client::Manager& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::BlockOracle>
{
    using ReturnType = blockchain::client::implementation::BlockOracle;

    return std::make_unique<ReturnType>(api, network, db, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
BlockOracle::BlockOracle(
    const api::client::Manager& api,
    const internal::Network& network,
    const internal::BlockDatabase& db,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    : Worker(api, std::chrono::milliseconds{25})
    , network_(network)
    , init_promise_()
    , init_(init_promise_.get_future())
    , cache_(network, db, chain)
{
    init_executor({shutdown});
}

auto BlockOracle::Init() noexcept -> void { init_promise_.set_value(); }

auto BlockOracle::LoadBitcoin(const block::Hash& block) const noexcept
    -> BitcoinBlockFuture
{
    auto output = cache_.Request(block);
    trigger();

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
            [[fallthrough]];
        }
        case Task::StateMachine: {
            do_work();
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
        cache_.Shutdown();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto BlockOracle::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    return cache_.StateMachine();
}

auto BlockOracle::SubmitBlock(const zmq::Frame& in) const noexcept -> void
{
    auto work = MakeWork(Task::ProcessBlock);
    work->AddFrame(in);
    pipeline_->Push(work);
}

BlockOracle::~BlockOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
