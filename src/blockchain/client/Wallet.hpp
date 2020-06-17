// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <deque>
#include <future>
#include <map>
#include <queue>
#include <string>

#include "1_Internal.hpp"
#include "blockchain/client/HDStateData.hpp"
#include "core/Executor.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
class Wallet final : virtual public internal::Wallet, Executor<Wallet>
{
public:
    struct Account {
        using Subchain = internal::WalletDatabase::Subchain;
        using Task = internal::Wallet::Task;
        using BalanceTree = api::client::blockchain::internal::BalanceTree;

        auto queue_work(const Task task, const HDStateData& data) noexcept
            -> void;
        auto reorg(const block::Position& parent) noexcept -> bool;
        auto state_machine() noexcept -> bool;

        Account(
            const api::Core& api,
            const BalanceTree& ref,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket) noexcept;
        Account(Account&&) noexcept;

    private:
        const api::Core& api_;
        const BalanceTree& ref_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const filter::Type filter_type_;
        const zmq::socket::Push& socket_;
        std::map<OTIdentifier, HDStateData> internal_;
        std::map<OTIdentifier, HDStateData> external_;

        auto state_machine_hd(HDStateData& data) noexcept -> bool;
        auto reorg_hd(HDStateData& data, const block::Position& parent) noexcept
            -> bool;

        Account(const Account&) = delete;
    };

    auto Init() noexcept -> void final;
    auto Run() noexcept -> void final { Trigger(); }
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_executor();
    }

    Wallet(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const internal::Network& parent,
        const Type chain,
        const std::string& shutdown) noexcept;

    ~Wallet() final;

private:
    friend Executor<Wallet>;

    enum class Work : OTZMQWorkType {
        filter = OT_ZMQ_NEW_FILTER_SIGNAL,
        nym = OT_ZMQ_NEW_NYM_SIGNAL,
        reorg = OT_ZMQ_REORG_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = OT_ZMQ_SHUTDOWN_SIGNAL,
    };

    struct Accounts {
        auto Add(const identifier::Nym& nym) noexcept -> bool;
        auto Add(const zmq::Frame& message) noexcept -> bool;
        auto Reorg(const block::Position& parent) noexcept -> bool;

        auto state_machine() noexcept -> bool;

        Accounts(
            const api::Core& api,
            const api::client::internal::Blockchain& blockchain,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket,
            const Type chain) noexcept;

    private:
        using AccountMap = std::map<OTNymID, Account>;

        const api::Core& api_;
        const api::client::internal::Blockchain& blockchain_api_;
        const internal::Network& network_;
        const internal::WalletDatabase& db_;
        const zmq::socket::Push& socket_;
        const Type chain_;
        AccountMap map_;

        static auto init(
            const api::Core& api,
            const api::client::internal::Blockchain& blockchain,
            const internal::Network& network,
            const internal::WalletDatabase& db,
            const zmq::socket::Push& socket,
            const Type chain) noexcept -> AccountMap;
    };

    const internal::Network& parent_;
    const internal::WalletDatabase& db_;
    const api::client::internal::Blockchain& blockchain_api_;
    const Type chain_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    OTZMQPushSocket socket_;
    Accounts accounts_;

    auto pipeline(const zmq::Message& in) noexcept -> void;
    auto process_reorg(const zmq::Message& in) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> void;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet &&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
