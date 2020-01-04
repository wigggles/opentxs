// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "core/Shutdown.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <algorithm>
#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <thread>

#include "PeerManager.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::PeerManager::"

namespace opentxs
{
blockchain::client::internal::PeerManager* Factory::BlockchainPeerManager(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::PeerDatabase& database,
    const blockchain::Type type,
    const std::string& seednode,
    const std::string& shutdown)
{
    using ReturnType = blockchain::client::implementation::PeerManager;

    return new ReturnType{api, network, database, type, seednode, shutdown};
}
}  // namespace opentxs

namespace opentxs::blockchain::client::implementation
{
const std::map<Type, std::uint16_t> PeerManager::default_port_map_{
    {Type::Unknown, 0},
    {Type::Bitcoin, 8333},
    {Type::Bitcoin_testnet3, 18333},
    {Type::BitcoinCash, 8333},
    {Type::BitcoinCash_testnet3, 18333},
    {Type::Ethereum_frontier, 30303},
    {Type::Ethereum_ropsten, 30303},
};
const std::map<Type, p2p::Protocol> PeerManager::protocol_map_{
    {Type::Unknown, p2p::Protocol::bitcoin},
    {Type::Bitcoin, p2p::Protocol::bitcoin},
    {Type::Bitcoin_testnet3, p2p::Protocol::bitcoin},
    {Type::BitcoinCash, p2p::Protocol::bitcoin},
    {Type::BitcoinCash_testnet3, p2p::Protocol::bitcoin},
    {Type::Ethereum_frontier, p2p::Protocol::ethereum},
    {Type::Ethereum_ropsten, p2p::Protocol::ethereum},
};

PeerManager::PeerManager(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::PeerDatabase& database,
    const Type chain,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    : internal::PeerManager()
    , StateMachine(std::bind(&PeerManager::state_machine, this))
    , api_(api)
    , database_(database)
    , running_(Flag::Factory(true))
    , io_context_()
    , jobs_(api)
    , peers_(
          api,
          network,
          database_,
          *this,
          running_,
          jobs_,
          shutdown,
          chain,
          seednode,
          io_context_)
    , heartbeat_task_()
    , shutdown_(
          api.ZeroMQ(),
          {api.Endpoints().Shutdown(), shutdown},
          [this](auto& promise) { this->shutdown(promise); })
{
}

PeerManager::IO::IO() noexcept
    : context_()
    , work_(std::make_unique<boost::asio::io_context::work>(context_))
    , thread_pool_()
{
    OT_ASSERT(work_);

    for (unsigned int i{0}; i < std::thread::hardware_concurrency(); ++i) {
        thread_pool_.create_thread(
            boost::bind(&boost::asio::io_context::run, &context_));
    }
}

PeerManager::Jobs::Jobs(const api::internal::Core& api) noexcept
    : getheaders_(api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , getcfilters_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , heartbeat_(api.ZeroMQ().PublishSocket())
    , endpoint_map_()
    , socket_map_({
          {Task::Getheaders, &getheaders_.get()},
          {Task::Getcfilters, &getcfilters_.get()},
          {Task::Heartbeat, &heartbeat_.get()},
      })
{
    // NOTE endpoint_map_ should never be modified after construction
    listen(Task::Getheaders, getheaders_);
    listen(Task::Getcfilters, getcfilters_);
    listen(Task::Heartbeat, heartbeat_);
}

PeerManager::Peers::Peers(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::PeerDatabase& database,
    const internal::PeerManager& parent,
    const Flag& running,
    Jobs& jobs,
    const std::string& shutdown,
    const Type chain,
    const std::string& seednode,
    boost::asio::io_context& context) noexcept
    : api_(api)
    , network_(network)
    , database_(database)
    , parent_(parent)
    , running_(running)
    , jobs_(jobs)
    , shutdown_endpoint_(shutdown)
    , chain_(chain)
    , default_peer_(set_default_peer(seednode))
    , context_(context)
    , lock_()
    , queue_lock_()
    , next_id_(0)
    , minimum_peers_(2)
    , disconnect_peers_()
    , peers_()
    , active_()
    , count_()
{
    database_.AddOrUpdate(Endpoint{opentxs::Factory().BlockchainAddress(
        api_,
        protocol_map_.at(chain_),
        p2p::Network::ipv4,
        default_peer_,
        default_port_map_.at(chain_),
        chain_,
        Time{},
        {})});
}

void PeerManager::IO::Shutdown() noexcept
{
    work_.reset();
    thread_pool_.join_all();
}

void PeerManager::Jobs::Dispatch(const Task type, zmq::Message& work) noexcept
{
    socket_map_.at(type)->Send(work);
}

std::string PeerManager::Jobs::Endpoint(const Task type) const noexcept
{
    try {

        return endpoint_map_.at(type);
    } catch (...) {

        return {};
    }
}

void PeerManager::Jobs::listen(
    const Task type,
    const zmq::socket::Sender& socket) noexcept
{
    auto& map = const_cast<EndpointMap&>(endpoint_map_);
    auto [it, added] = map.emplace(
        type,
        std::string{"inproc://opentxs//blockchain/peer_tasks/"} +
            Identifier::Random()->str());

    OT_ASSERT(added);

    const auto listen = socket.Start(it->second);

    OT_ASSERT(listen);
}

void PeerManager::Jobs::Shutdown() noexcept
{
    for (auto [type, socket] : socket_map_) { socket->Close(); }
}

void PeerManager::Peers::add_peer(const Lock& lock, Endpoint endpoint) noexcept
{
    OT_ASSERT(endpoint);

    const auto address = OTIdentifier{endpoint->ID()};
    auto& count = active_[address];

    if (0 < count) {
        // Wait for more peers
        Sleep(std::chrono::milliseconds(10));
    } else {
        const auto id = ++next_id_;
        const auto [it, added] =
            peers_.emplace(id, peer_factory(std::move(endpoint), id));

        if (added) {
            ++count;
            ++count_;
        }
    }
}

bool PeerManager::Peers::AddPeer(const p2p::Address& address) noexcept
{
    if (false == running_) { return false; }

    if (address.Chain() != chain_) { return false; }

    Endpoint endpoint{Factory::BlockchainAddress(
        api_,
        address.Style(),
        address.Type(),
        address.Bytes(),
        address.Port(),
        address.Chain(),
        address.LastConnected(),
        address.Services())};

    OT_ASSERT(endpoint);

    Lock lock(lock_);
    add_peer(lock, std::move(endpoint));

    return true;
}

PeerManager::Peers::Endpoint PeerManager::Peers::get_peer() const noexcept
{
    const auto protocol = protocol_map_.at(chain_);

    auto pAddress =
        database_.Get(protocol, {p2p::Network::ipv4, p2p::Network::ipv6}, {});

    if (pAddress) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to fallback peer")
            .Flush();

        return Endpoint{opentxs::Factory().BlockchainAddress(
            api_,
            protocol,
            p2p::Network::ipv4,
            default_peer_,
            default_port_map_.at(chain_),
            chain_,
            Time{},
            {})};
    }
}

void PeerManager::Peers::Heartbeat() const noexcept
{
    auto message = api_.ZeroMQ().Message();
    jobs_.Dispatch(Task::Heartbeat, message);
}

p2p::internal::Peer* PeerManager::Peers::peer_factory(
    Endpoint endpoint,
    const int id) noexcept
{
    switch (chain_) {
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3: {
            return opentxs::Factory::BitcoinP2PPeerLegacy(
                api_,
                network_,
                parent_,
                id,
                std::move(endpoint),
                context_,
                shutdown_endpoint_);
        }
        default: {
            OT_FAIL;
        }
    }
}

OTData PeerManager::Peers::set_default_peer(const std::string node) noexcept
{
    if (false == node.empty()) {
        try {
            const auto bytes = ip::make_address_v4(node).to_bytes();

            return Data::Factory(bytes.data(), bytes.size());
        } catch (...) {
        }
    }

    return Data::Factory("0x7f000001", Data::Mode::Hex);
}

void PeerManager::Peers::QueueDisconnect(const int id) noexcept
{
    if (false == running_) { return; }

    Lock lock(queue_lock_);
    disconnect_peers_.emplace_back(id);
}

bool PeerManager::Peers::Run() noexcept
{
    auto disconnect = std::vector<int>{};

    {
        Lock lock(queue_lock_);
        disconnect.swap(disconnect_peers_);
    }

    Lock lock(lock_);

    for (const auto& id : disconnect) {
        auto it = peers_.find(id);

        if (peers_.end() == it) { continue; }

        auto& peer = *it->second;
        --active_.at(peer.AddressID());
        peer.Shutdown().get();
        it->second.reset();
        peers_.erase(it);
        --count_;
    }

    if (false == running_) { return false; }

    const auto target = minimum_peers_.load();

    if (target > peers_.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Fewer peers (")(peers_.size())(
            ") than desired (")(target)(")")
            .Flush();
        add_peer(lock, get_peer());
    }

    return target > peers_.size();
}

void PeerManager::Peers::Shutdown() noexcept
{
    OT_ASSERT(false == running_);

    Lock peerLock(lock_, std::defer_lock);
    Lock queueLock(queue_lock_, std::defer_lock);
    std::lock(peerLock, queueLock);

    for (auto& [id, peer] : peers_) {
        peer->Shutdown().get();
        peer.reset();
    }

    peers_.clear();
    count_.store(0);
    active_.clear();
    disconnect_peers_.clear();
}

bool PeerManager::AddPeer(const p2p::Address& address) const noexcept
{
    if (false == running_.get()) { return false; }

    const auto output = peers_.AddPeer(address);
    Trigger();

    return output;
}

bool PeerManager::Connect() noexcept
{
    if (false == running_.get()) { return false; }

    return Trigger();
}

void PeerManager::Disconnect(const int id) const noexcept
{
    peers_.QueueDisconnect(id);
    Trigger();
}

void PeerManager::init() noexcept
{
    heartbeat_task_ = api_.Schedule(
        std::chrono::seconds(1), [this]() -> void { this->Heartbeat(); });
    Trigger();
}

void PeerManager::RequestFilters(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept
{
    if (false == running_.get()) { return; }

    if (0 == peers_.Count()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(Data::Factory(&start, sizeof(start)));
    work->AddFrame(stop);
    jobs_.Dispatch(Task::Getcfilters, work);
}

void PeerManager::RequestHeaders() const noexcept
{
    if (false == running_.get()) { return; }

    if (0 == peers_.Count()) { return; }

    auto work = zmq::Message::Factory();
    jobs_.Dispatch(Task::Getheaders, work);
}

std::shared_future<void> PeerManager::Shutdown() noexcept
{
    shutdown_.Close();

    if (running_.get()) { shutdown(shutdown_.promise_); }

    return shutdown_.future_;
}

void PeerManager::shutdown(std::promise<void>& promise) noexcept
{
    running_->Off();
    api_.Cancel(heartbeat_task_);
    Stop().get();
    jobs_.Shutdown();
    peers_.Shutdown();

    try {
        promise.set_value();
    } catch (...) {
    }
}

bool PeerManager::state_machine() noexcept { return peers_.Run(); }

PeerManager::~PeerManager()
{
    Shutdown().get();
    shutdown_.Close();
    io_context_.Shutdown();
}
}  // namespace opentxs::blockchain::client::implementation
