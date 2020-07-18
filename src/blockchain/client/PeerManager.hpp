// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/container/flat_set.hpp>
#include <atomic>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace p2p
{
namespace internal
{
struct Address;
struct Peer;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Sender;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class PeerManager final : virtual public internal::PeerManager,
                          public Worker<PeerManager>
{
public:
    static const std::map<Type, std::uint16_t> default_port_map_;
    static const std::map<Type, std::vector<std::string>> dns_seeds_;
    static const std::map<Type, p2p::Protocol> protocol_map_;

    auto AddPeer(const p2p::Address& address) const noexcept -> bool final;
    auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool final;
    auto Database() const noexcept -> const internal::PeerDatabase& final
    {
        return database_;
    }
    auto Connect() noexcept -> bool;
    auto Disconnect(const int id) const noexcept -> void final;
    auto Endpoint(const Task type) const noexcept -> std::string final
    {
        return jobs_.Endpoint(type);
    }
    auto GetPeerCount() const noexcept -> std::size_t final
    {
        return peers_.Count();
    }
    auto Heartbeat() const noexcept -> void final
    {
        jobs_.Dispatch(Task::Heartbeat);
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
    auto RequestHeaders() const noexcept -> bool final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }

    auto init() noexcept -> void final;

    PeerManager(
        const api::client::Manager& api,
        const internal::Network& network,
        const internal::PeerDatabase& database,
        const blockchain::client::internal::IO& io,
        const Type chain,
        const std::string& seednode,
        const std::string& shutdown) noexcept;

    ~PeerManager() final;

private:
    friend Worker<PeerManager>;

    struct Jobs {
        auto Endpoint(const Task type) const noexcept -> std::string;
        auto Work(const Task task, std::promise<void>* promise = nullptr)
            const noexcept -> OTZMQMessage;

        auto Dispatch(const Task type) noexcept -> void;
        auto Dispatch(zmq::Message& work) noexcept -> void;
        auto Shutdown() noexcept -> void;

        Jobs(const api::client::Manager& api) noexcept;

    private:
        using EndpointMap = std::map<Task, std::string>;
        using SocketMap = std::map<Task, zmq::socket::Sender*>;

        const zmq::Context& zmq_;
        OTZMQPushSocket getheaders_;
        OTZMQPushSocket getcfheaders_;
        OTZMQPushSocket getcfilters_;
        OTZMQPublishSocket heartbeat_;
        OTZMQPushSocket getblock_;
        OTZMQPushSocket broadcast_transaction_;
        const EndpointMap endpoint_map_;
        const SocketMap socket_map_;

        auto listen(const Task type, const zmq::socket::Sender& socket) noexcept
            -> void;

        Jobs() = delete;
    };

    struct Peers {
        auto Count() const noexcept -> std::size_t { return count_.load(); }

        auto AddPeer(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto Disconnect(const int id) noexcept -> void;
        auto Run() noexcept -> bool;
        auto Shutdown() noexcept -> void;

        Peers(
            const api::client::Manager& api,
            const internal::Network& network,
            const internal::PeerDatabase& database,
            const internal::PeerManager& parent,
            const Flag& running,
            const std::string& shutdown,
            const Type chain,
            const std::string& seednode,
            const blockchain::client::internal::IO& context) noexcept;

    private:
        using Endpoint = std::unique_ptr<p2p::internal::Address>;
        using Peer = std::unique_ptr<p2p::internal::Peer>;
        using Resolver = boost::asio::ip::tcp::resolver;
        using Addresses = boost::container::flat_set<OTIdentifier>;

        const api::client::Manager& api_;
        const internal::Network& network_;
        const internal::PeerDatabase& database_;
        const internal::PeerManager& parent_;
        const Flag& running_;
        const std::string& shutdown_endpoint_;
        const Type chain_;
        const bool invalid_peer_;
        const OTData localhost_peer_;
        const OTData default_peer_;
        const blockchain::client::internal::IO& context_;
        mutable Resolver resolver_;
        std::atomic<int> next_id_;
        std::atomic<std::size_t> minimum_peers_;
        std::map<int, Peer> peers_;
        std::map<OTIdentifier, int> active_;
        std::atomic<std::size_t> count_;
        Addresses connected_;

        static auto set_default_peer(
            const std::string node,
            const Data& localhost,
            bool& invalidPeer) noexcept -> OTData;

        auto get_default_peer() const noexcept -> Endpoint;
        auto get_dns_peer() const noexcept -> Endpoint;
        auto get_fallback_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto get_peer() const noexcept -> Endpoint;
        auto get_preferred_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto is_not_connected(const p2p::Address& endpoint) const noexcept
            -> bool;

        auto add_peer(Endpoint endpoint) noexcept -> void;
        auto peer_factory(Endpoint endpoint, const int id) noexcept
            -> p2p::internal::Peer*;
    };

    enum class Work : OTZMQWorkType {
        Disconnect = OT_ZMQ_INTERNAL_SIGNAL + 0,
        AddPeer = OT_ZMQ_INTERNAL_SIGNAL + 1,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = value(WorkType::Shutdown),
    };

    static const unsigned int peer_target_{2};

    const internal::PeerDatabase& database_;
    const internal::IO& io_context_;
    mutable Jobs jobs_;
    mutable Peers peers_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto pipeline(zmq::Message& message) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    PeerManager() = delete;
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager&&) = delete;
    auto operator=(const PeerManager&) -> PeerManager& = delete;
    auto operator=(PeerManager &&) -> PeerManager& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
