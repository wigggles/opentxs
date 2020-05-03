// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <future>
#include <iosfwd>
#include <memory>
#include <string>

#include "core/Executor.hpp"
#include "core/Shutdown.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceTree;
class PaymentCode;
}  // namespace blockchain
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class Network : virtual public internal::Network, public Executor<Network>
{
public:
    auto AddPeer(const p2p::Address& address) const noexcept -> bool final;
    auto BlockOracle() const noexcept -> const internal::BlockOracle& final
    {
        return *block_p_;
    }
    auto API() const noexcept -> const api::internal::Core& final
    {
        return api_;
    }
    auto Chain() const noexcept -> Type final { return chain_; }
    auto DB() const noexcept -> blockchain::internal::Database& final
    {
        return *database_p_;
    }
    auto GetConfirmations(const std::string& txid) const noexcept
        -> ChainHeight final;
    auto GetHeight() const noexcept -> ChainHeight final
    {
        return local_chain_height_.load();
    }
    auto GetPeerCount() const noexcept -> std::size_t final;
    auto GetType() const noexcept -> Type final { return chain_; }
    auto FilterOracle() const noexcept -> const internal::FilterOracle& final
    {
        return *filter_p_;
    }
    auto HeaderOracle() const noexcept -> const internal::HeaderOracle& final
    {
        return header_;
    }
    auto IsSynchronized() const noexcept -> bool final
    {
        return local_chain_height_.load() >= remote_chain_height_.load();
    }
    auto Reorg() const noexcept -> const network::zeromq::socket::Publish& final
    {
        return parent_.Reorg();
    }
    auto RequestBlock(const block::Hash& block) const noexcept -> bool final;
    auto RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool final;
    auto RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept -> bool final;
    auto SendToAddress(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::BalanceTree& source) const noexcept
        -> std::string final;
    auto SendToPaymentCode(
        const std::string& address,
        const Amount amount,
        const api::client::blockchain::PaymentCode& source) const noexcept
        -> std::string final;
    auto Submit(network::zeromq::Message& work) const noexcept -> void final;
    auto UpdateHeight(const block::Height height) const noexcept -> void final;
    auto UpdateLocalHeight(const block::Position position) const noexcept
        -> void final;
    auto Work(const Task type) const noexcept -> OTZMQMessage final;

    auto Connect() noexcept -> bool final;
    auto Disconnect() noexcept -> bool final;
    auto HeaderOracle() noexcept -> internal::HeaderOracle& final
    {
        return header_;
    }
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_executor();
    }

    ~Network() override;

private:
    opentxs::internal::ShutdownSender shutdown_sender_;
    std::unique_ptr<blockchain::internal::Database> database_p_;
    std::unique_ptr<internal::HeaderOracle> header_p_;
    std::unique_ptr<internal::PeerManager> peer_p_;
    std::unique_ptr<internal::BlockOracle> block_p_;
    std::unique_ptr<internal::FilterOracle> filter_p_;
    std::unique_ptr<internal::Wallet> wallet_p_;

protected:
    const api::client::internal::Blockchain& blockchain_;
    const Type chain_;
    blockchain::internal::Database& database_;
    internal::FilterOracle& filters_;
    internal::HeaderOracle& header_;
    internal::PeerManager& peer_;
    internal::BlockOracle& block_;
    internal::Wallet& wallet_;

    // NOTE call init in every final constructor body
    auto init() noexcept -> void;

    Network(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const Type type,
        const std::string& seednode,
        const std::string& shutdown) noexcept;

private:
    friend Executor<Network>;

    const api::client::internal::Blockchain& parent_;
    mutable std::atomic<block::Height> local_chain_height_;
    mutable std::atomic<block::Height> remote_chain_height_;
    OTFlag processing_headers_;
    int task_id_;

    static auto shutdown_endpoint() noexcept -> std::string;

    virtual auto instantiate_header(const ReadView payload) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    auto pipeline(zmq::Message& in) noexcept -> void;
    auto process_block(zmq::Message& in) noexcept -> void;
    auto process_cfheader(zmq::Message& in) noexcept -> void;
    auto process_filter(zmq::Message& in) noexcept -> void;
    auto process_header(zmq::Message& in) noexcept -> void;
    auto process_state_machine() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;

    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) = delete;
};
}  // namespace opentxs::blockchain::client::implementation
