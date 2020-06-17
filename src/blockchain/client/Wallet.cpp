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
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/client/HDStateData.hpp"
#include "core/Executor.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
    const api::internal::Core& api,
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
        default: {
            OT_FAIL;
        }
    }
}
}  // namespace opentxs::blockchain::client::internal

namespace opentxs::blockchain::client::implementation
{
Wallet::Wallet(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& parent,
    const Type chain,
    const std::string& shutdown) noexcept
    : Executor(api)
    , parent_(parent)
    , db_(parent_.DB())
    , blockchain_api_(blockchain)
    , chain_(chain)
    , init_promise_()
    , init_(init_promise_.get_future())
    , socket_(api_.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Connect))
    , accounts_(api_, blockchain_api_, parent_, db_, socket_, chain_)
{
    auto zmq = socket_->Start(blockchain.ThreadPool().Endpoint());

    OT_ASSERT(zmq);

    init_executor({
        shutdown,
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().NymCreated(),
        api.Endpoints().InternalBlockchainFilterUpdated(chain),
    });
}

Wallet::Account::Account(
    const api::Core& api,
    const BalanceTree& ref,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& socket) noexcept
    : api_(api)
    , ref_(ref)
    , network_(network)
    , db_(db)
    , filter_type_(network.FilterOracle().DefaultType())
    , socket_(socket)
    , internal_()
    , external_()
{
    for (const auto& subaccount : ref_.GetHD()) {
        const auto& id = subaccount.ID();
        internal_.try_emplace(
            id, network_, db_, subaccount, filter_type_, Subchain::Internal);
        external_.try_emplace(
            id, network_, db_, subaccount, filter_type_, Subchain::External);
    }
}

Wallet::Account::Account(Account&& rhs) noexcept
    : api_(rhs.api_)
    , ref_(rhs.ref_)
    , network_(rhs.network_)
    , db_(rhs.db_)
    , filter_type_(rhs.filter_type_)
    , socket_(rhs.socket_)
    , internal_(std::move(rhs.internal_))
    , external_(std::move(rhs.external_))
{
}

Wallet::Accounts::Accounts(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& socket,
    const Type chain) noexcept
    : api_(api)
    , blockchain_api_(blockchain)
    , network_(network)
    , db_(db)
    , socket_(socket)
    , chain_(chain)
    , map_(init(api, blockchain, network, db_, socket_, chain))
{
}

auto Wallet::Account::queue_work(
    const Task task,
    const HDStateData& data) noexcept -> void
{
    auto work = api_.ZeroMQ().Message(network_.Chain());
    work->AddFrame(internal::ThreadPool::Work::Wallet);
    work->AddFrame();
    work->AddFrame(reinterpret_cast<std::uintptr_t>(&data));
    work->AddFrame(task);
    socket_.Send(work);
}

auto Wallet::Account::state_machine() noexcept -> bool
{
    auto output{false};

    for (const auto& subaccount : ref_.GetHD()) {
        const auto& id = subaccount.ID();
        LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
            ": Processing account ")(id)
            .Flush();

        {
            auto it = internal_.find(id);

            if (internal_.end() == it) {
                auto [it2, added] = internal_.try_emplace(
                    id,
                    network_,
                    db_,
                    subaccount,
                    filter_type_,
                    Subchain::Internal);
                it = it2;
            }

            output |= state_machine_hd(it->second);
        }

        {
            auto it = external_.find(id);

            if (external_.end() == it) {
                auto [it2, added] = external_.try_emplace(
                    id,
                    network_,
                    db_,
                    subaccount,
                    filter_type_,
                    Subchain::External);
                it = it2;
            }

            output |= state_machine_hd(it->second);
        }
    }

    return output;
}

auto Wallet::Account::state_machine_hd(HDStateData& data) noexcept -> bool
{
    const auto& node = data.node_;
    const auto subchain = data.subchain_;
    auto& running = data.running_;
    auto& lastIndexed = data.last_indexed_;
    auto& lastScanned = data.last_scanned_;
    auto& requestBlocks = data.blocks_to_request_;
    auto& outstanding = data.outstanding_blocks_;
    auto& queue = data.process_block_queue_;

    if (running) { return true; }

    {
        for (const auto& hash : requestBlocks) {
            LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                ": Requesting block ")(hash->asHex())(" queue position: ")(
                outstanding.size())
                .Flush();

            if (0 == outstanding.count(hash)) {
                auto [it, added] = outstanding.emplace(
                    hash, network_.BlockOracle().LoadBitcoin(hash));

                OT_ASSERT(added);

                queue.push(it);
            }
        }

        requestBlocks.clear();
    }

    {
        lastIndexed =
            db_.SubchainLastIndexed(node.ID(), subchain, filter_type_);
        const auto generated = node.LastGenerated(subchain);

        if (generated.has_value()) {
            if ((false == lastIndexed.has_value()) ||
                (lastIndexed.value() != generated.value())) {
                LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                    ": Subchain has ")(generated.value() + 1)(
                    " keys generated, but only ")(lastIndexed.value_or(0))(
                    " have been indexed.")
                    .Flush();
                running.store(true);
                queue_work(Task::index, data);

                return true;
            } else {
                LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(": All ")(
                    generated.value() + 1)(" generated keys have been indexed.")
                    .Flush();
            }
        }
    }

    {
        auto needScan{false};

        if (lastScanned.has_value()) {
            const auto [ancestor, best] =
                network_.HeaderOracle().CommonParent(lastScanned.value());
            lastScanned = ancestor;

            if (lastScanned == best) {
                LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                    ": Subchain has been scanned to current best block ")(
                    best.second->asHex())(" at height ")(best.first)
                    .Flush();
            } else {
                needScan = true;
                LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                    ": Subchain scanning progress: ")(lastScanned.value().first)
                    .Flush();
            }

        } else {
            needScan = true;
            LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                ": Subchain scanning progress: ")(0)
                .Flush();
        }

        if (needScan) {
            running.store(true);
            queue_work(Task::scan, data);

            return true;
        }
    }

    if (false == queue.empty()) {
        const auto& [id, future] = *queue.front();

        if (std::future_status::ready ==
            future.wait_for(std::chrono::milliseconds(1))) {
            LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                ": Ready to process block")
                .Flush();
            running.store(true);
            queue_work(Task::process, data);

            return true;
        } else {
            LogVerbose(OT_METHOD)("Account::")(__FUNCTION__)(
                ": Waiting for block download")
                .Flush();
        }
    }

    return false;
}

auto Wallet::Accounts::Add(const identifier::Nym& nym) noexcept -> bool
{
    auto [it, added] = map_.try_emplace(
        nym,
        api_,
        blockchain_api_.BalanceTree(nym, chain_),
        network_,
        db_,
        socket_);

    if (added) {
        LogNormal("Initializing ")(blockchain::internal::DisplayString(chain_))(
            " wallet for ")(nym)
            .Flush();
    }

    return added;
}

auto Wallet::Accounts::Add(const zmq::Frame& message) noexcept -> bool
{
    auto id = api_.Factory().NymID();
    id->SetString(message);

    if (0 == id->size()) { return false; }

    return Add(id);
}

auto Wallet::Accounts::init(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& socket,
    const Type chain) noexcept -> AccountMap
{
    auto output = AccountMap{};

    for (const auto& nym : api.Wallet().LocalNyms()) {
        LogNormal("Initializing ")(blockchain::internal::DisplayString(chain))(
            " wallet for ")(nym)
            .Flush();
        output.try_emplace(
            nym, api, blockchain.BalanceTree(nym, chain), network, db, socket);
    }

    return output;
}

auto Wallet::Accounts::state_machine() noexcept -> bool
{
    auto output{false};

    for (auto& [nym, account] : map_) { output |= account.state_machine(); }

    return output;
}

auto Wallet::Init() noexcept -> void
{
    init_promise_.set_value();
    Trigger();
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
        case Work::filter: {
            Trigger();
        } break;
        case Work::nym: {
            OT_ASSERT(0 < body.size());

            accounts_.Add(body.at(0));
            Trigger();
        } break;
        case Work::reorg: {
            // TODO
            Trigger();
        } break;
        case Work::statemachine: {
            state_machine();
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

auto Wallet::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

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

auto Wallet::state_machine() noexcept -> void
{
    static const auto rateLimit = std::chrono::milliseconds{1};

    auto repeat = accounts_.state_machine();
    Sleep(rateLimit);

    if (repeat) { Sleep(rateLimit); }

    try {
        state_machine_.set_value(repeat);
    } catch (...) {
    }
}

Wallet::~Wallet() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
