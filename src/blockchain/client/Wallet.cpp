// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/client/Wallet.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <memory>

#include "blockchain/client/wallet/HDStateData.hpp"
#include "core/Worker.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::Wallet::"

namespace opentxs::factory
{
auto BlockchainWallet(
    const api::client::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::client::internal::Network& parent,
    const blockchain::Type chain,
    const std::string& shutdown)
    -> std::unique_ptr<blockchain::client::internal::Wallet>
{
    using ReturnType = blockchain::client::implementation::Wallet;

    return std::make_unique<ReturnType>(
        api, blockchain, parent, chain, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::internal
{
auto Wallet::ProcessTask(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogOutput("opentxs::blockchain::client::internal:Wallet:::")(
            __FUNCTION__)(": Invalid message")
            .Flush();

        OT_FAIL;
    }

    auto* pData = reinterpret_cast<implementation::HDStateData*>(
        body.at(0).as<std::uintptr_t>());

    OT_ASSERT(nullptr != pData);

    auto& data = *pData;
    auto postcondition = ScopeGuard{[&] { data.running_.store(false); }};

    switch (body.at(1).as<Task>()) {
        case Task::index: {
            data.index();
        } break;
        case Task::scan: {
            data.scan();
        } break;
        case Task::process: {
            data.process();
        } break;
        case Task::reorg: {
            data.reorg();
        } break;
        default: {
            OT_FAIL;
        }
    }
}
}  // namespace opentxs::blockchain::client::internal

namespace opentxs::blockchain::client::implementation
{
Wallet::Wallet(
    const api::client::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& parent,
    const Type chain,
    const std::string& shutdown) noexcept
    : Worker(api, std::chrono::milliseconds(10))
    , parent_(parent)
    , db_(parent_.DB())
    , blockchain_api_(blockchain)
    , chain_(chain)
    , init_promise_()
    , init_(init_promise_.get_future())
    , socket_(api_.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Connect))
    , accounts_(api, blockchain_api_, parent_, db_, socket_, chain_)
    , proposals_(api, parent_, db_, chain_)
{
    auto zmq = socket_->Start(blockchain.ThreadPool().Endpoint());

    OT_ASSERT(zmq);

    init_executor({
        shutdown,
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().NymCreated(),
        api.Endpoints().InternalBlockchainFilterUpdated(chain),
        blockchain.KeyEndpoint(),
    });
}

auto Wallet::ConstructTransaction(
    const proto::BlockchainTransactionProposal& tx) const noexcept
    -> std::future<block::pTxid>
{
    auto output = proposals_.Add(tx);
    trigger();

    return output;
}

auto Wallet::Init() noexcept -> void
{
    init_promise_.set_value();
    trigger();
}

auto Wallet::pipeline(const zmq::Message& in) noexcept -> void
{
    init_.get();

    if (false == running_.get()) { return; }

    const auto header = in.Header();

    if (1 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = header.at(0).as<Work>();
    const auto body = in.Body();

    switch (work) {
        case Work::block: {
            // nothing to do until the filter is downloaded
        } break;
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::nym: {
            OT_ASSERT(0 < body.size());

            accounts_.Add(body.at(0));
            [[fallthrough]];
        }
        case Work::key:
        case Work::filter:
        case Work::statemachine: {
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

auto Wallet::process_reorg(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (3 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto chain = body.at(0).as<blockchain::Type>();

    if (chain_ != chain) { return; }

    const auto parent = block::Position{
        body.at(2).as<block::Height>(),
        api_.Factory().Data(body.at(1).Bytes())};
    accounts_.Reorg(parent);
    do_work();
}

auto Wallet::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto Wallet::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    auto repeat = accounts_.state_machine();
    repeat |= proposals_.Run();

    return repeat;
}

Wallet::~Wallet() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
