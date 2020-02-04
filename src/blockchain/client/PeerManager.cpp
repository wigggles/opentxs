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
#include <random>
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
const std::map<Type, std::vector<std::string>> PeerManager::dns_seeds_{
    {Type::Bitcoin,
     {"seed.bitcoin.sipa.be",
      "dnsseed.bluematt.me",
      "dnsseed.bitcoin.dashjr.org",
      "seed.bitcoinstats.com",
      "seed.bitcoin.jonasschnelli.ch",
      "seed.btc.petertodd.org",
      "seed.bitcoin.sprovoost.nl",
      "dnsseed.emzy.de"}},
    {Type::Bitcoin_testnet3,
     {"testnet-seed.bitcoin.jonasschnelli.ch",
      "seed.tbtc.petertodd.org",
      "seed.testnet.bitcoin.sprovoost.nl",
      "testnet-seed.bluematt.me"}},
    {Type::BitcoinCash,
     {"seed.bitcoinabc.org",
      "seed-abc.bitcoinforks.org",
      "btccash-seeder.bitcoinunlimited.info",
      "seed.bitprim.org",
      "seed.deadalnix.me",
      "seed.bchd.cash"}},
    {Type::BitcoinCash_testnet3,
     {"testnet-seed.bitcoinabc.org",
      "testnet-seed-abc.bitcoinforks.org",
      "testnet-seed.bitprim.org",
      "testnet-seed.deadalnix.me",
      "testnet-seed.bchd.cash"}},
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
    , getcfheaders_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , getcfilters_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , heartbeat_(api.ZeroMQ().PublishSocket())
    , endpoint_map_()
    , socket_map_({
          {Task::Getheaders, &getheaders_.get()},
          {Task::Getcfheaders, &getcfheaders_.get()},
          {Task::Getcfilters, &getcfilters_.get()},
          {Task::Heartbeat, &heartbeat_.get()},
      })
{
    // NOTE endpoint_map_ should never be modified after construction
    listen(Task::Getheaders, getheaders_);
    listen(Task::Getcfheaders, getcfheaders_);
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
    , localhost_peer_(api.Factory().Data("0x7f000001", StringStyle::Hex))
    , default_peer_(set_default_peer(seednode, localhost_peer_))
    , context_(context)
    , resolver_(context_)
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

auto PeerManager::IO::Shutdown() noexcept -> void
{
    work_.reset();
    thread_pool_.join_all();
}

auto PeerManager::Jobs::Dispatch(const Task type, zmq::Message& work) noexcept
    -> void
{
    socket_map_.at(type)->Send(work);
}

auto PeerManager::Jobs::Endpoint(const Task type) const noexcept -> std::string
{
    try {

        return endpoint_map_.at(type);
    } catch (...) {

        return {};
    }
}

auto PeerManager::Jobs::listen(
    const Task type,
    const zmq::socket::Sender& socket) noexcept -> void
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

auto PeerManager::Jobs::Shutdown() noexcept -> void
{
    for (auto [type, socket] : socket_map_) { socket->Close(); }
}

auto PeerManager::Peers::add_peer(const Lock& lock, Endpoint endpoint) noexcept
    -> void
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

auto PeerManager::Peers::AddPeer(const p2p::Address& address) noexcept -> bool
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

auto PeerManager::Peers::get_default_peer() const noexcept -> Endpoint
{
    if (localhost_peer_.get() == default_peer_) { return {}; }

    return Endpoint{opentxs::Factory().BlockchainAddress(
        api_,
        protocol_map_.at(chain_),
        p2p::Network::ipv4,
        default_peer_,
        default_port_map_.at(chain_),
        chain_,
        Time{},
        {})};
}

auto PeerManager::Peers::get_dns_peer() const noexcept -> Endpoint
{
    try {
        const auto& dns = dns_seeds_.at(chain_);
        auto seeds = std::vector<std::string>{};
        const auto count = std::size_t{1};
        std::sample(
            std::begin(dns),
            std::end(dns),
            std::back_inserter(seeds),
            count,
            std::mt19937{std::random_device{}()});

        if (0 == seeds.size()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": No dns seeds available")
                .Flush();

            return {};
        }

        const auto& seed = *seeds.cbegin();
        const auto port = default_port_map_.at(chain_);
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Using DNS seed: ")(seed).Flush();
        const auto results = resolver_.resolve(
            seed, std::to_string(port), Resolver::query::numeric_service);

        for (const auto& result : results) {
            const auto address = result.endpoint().address();
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Found address: ")(
                address.to_string())
                .Flush();
            auto output = Endpoint{};

            if (address.is_v4()) {
                const auto bytes = address.to_v4().to_bytes();
                output = opentxs::Factory().BlockchainAddress(
                    api_,
                    protocol_map_.at(chain_),
                    p2p::Network::ipv4,
                    api_.Factory().Data(
                        ReadView{reinterpret_cast<const char*>(bytes.data()),
                                 bytes.size()}),
                    port,
                    chain_,
                    Time{},
                    {});
            } else if (address.is_v6()) {
                const auto bytes = address.to_v6().to_bytes();
                output = opentxs::Factory().BlockchainAddress(
                    api_,
                    protocol_map_.at(chain_),
                    p2p::Network::ipv6,
                    api_.Factory().Data(
                        ReadView{reinterpret_cast<const char*>(bytes.data()),
                                 bytes.size()}),
                    port,
                    chain_,
                    Time{},
                    {});
            }

            if (output) {
                database_.AddOrUpdate(output->clone_internal());

                return output;
            }
        }

        LogVerbose(OT_METHOD)(__FUNCTION__)(": No addresses found").Flush();

        return {};
    } catch (const boost::system::system_error& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return {};
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No dns seeds defined").Flush();

        return {};
    }
}

auto PeerManager::Peers::get_fallback_peer(const p2p::Protocol protocol) const
    noexcept -> Endpoint
{
    return database_.Get(
        protocol, {p2p::Network::ipv4, p2p::Network::ipv6}, {});
}

auto PeerManager::Peers::get_peer() const noexcept -> Endpoint
{
    const auto protocol = protocol_map_.at(chain_);
    auto pAddress = get_default_peer();

    if (pAddress) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_preferred_peer(protocol);

    if (pAddress) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_dns_peer();

    if (pAddress) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_fallback_peer(protocol);

    OT_ASSERT(pAddress);

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Attempting to connect to peer: ")(
        pAddress->Display())
        .Flush();

    return pAddress;
}

auto PeerManager::Peers::get_preferred_peer(const p2p::Protocol protocol) const
    noexcept -> Endpoint
{
    return database_.Get(
        protocol,
        {p2p::Network::ipv4, p2p::Network::ipv6},
        {p2p::Service::CompactFilters});
}

auto PeerManager::Peers::Heartbeat() const noexcept -> void
{
    auto message = api_.ZeroMQ().Message();
    jobs_.Dispatch(Task::Heartbeat, message);
}

auto PeerManager::Peers::peer_factory(Endpoint endpoint, const int id) noexcept
    -> p2p::internal::Peer*
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

auto PeerManager::Peers::set_default_peer(
    const std::string node,
    const Data& localhost) noexcept -> OTData
{
    if (false == node.empty()) {
        try {
            const auto bytes = ip::make_address_v4(node).to_bytes();

            return Data::Factory(bytes.data(), bytes.size());
        } catch (...) {
        }
    }

    return localhost;
}

auto PeerManager::Peers::QueueDisconnect(const int id) noexcept -> void
{
    if (false == running_) { return; }

    Lock lock(queue_lock_);
    disconnect_peers_.emplace_back(id);
}

auto PeerManager::Peers::Run() noexcept -> bool
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

auto PeerManager::Peers::Shutdown() noexcept -> void
{
    OT_ASSERT(false == running_);

    Lock peerLock(lock_, std::defer_lock);
    Lock queueLock(queue_lock_, std::defer_lock);
    std::lock(peerLock, queueLock);

    for (auto& [id, peer] : peers_) {
        peer->Shutdown().wait_for(std::chrono::seconds(1));
        peer.reset();
    }

    peers_.clear();
    count_.store(0);
    active_.clear();
    disconnect_peers_.clear();
}

auto PeerManager::AddPeer(const p2p::Address& address) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    const auto output = peers_.AddPeer(address);
    Trigger();

    return output;
}

auto PeerManager::Connect() noexcept -> bool
{
    if (false == running_.get()) { return false; }

    return Trigger();
}

auto PeerManager::Disconnect(const int id) const noexcept -> void
{
    peers_.QueueDisconnect(id);
    Trigger();
}

auto PeerManager::init() noexcept -> void
{
    heartbeat_task_ = api_.Schedule(
        std::chrono::seconds(1), [this]() -> void { this->Heartbeat(); });
    Trigger();
}

auto PeerManager::RequestFilterHeaders(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept -> void
{
    if (false == running_.get()) { return; }

    if (0 == peers_.Count()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(Data::Factory(&start, sizeof(start)));
    work->AddFrame(stop);
    jobs_.Dispatch(Task::Getcfheaders, work);
}

auto PeerManager::RequestFilters(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept -> void
{
    if (false == running_.get()) { return; }

    if (0 == peers_.Count()) { return; }

    auto work = zmq::Message::Factory();
    work->AddFrame(Data::Factory(&type, sizeof(type)));
    work->AddFrame(Data::Factory(&start, sizeof(start)));
    work->AddFrame(stop);
    jobs_.Dispatch(Task::Getcfilters, work);
}

auto PeerManager::RequestHeaders() const noexcept -> void
{
    if (false == running_.get()) { return; }

    if (0 == peers_.Count()) { return; }

    auto work = zmq::Message::Factory();
    jobs_.Dispatch(Task::Getheaders, work);
}

auto PeerManager::Shutdown() noexcept -> std::shared_future<void>
{
    shutdown_.Close();

    if (running_.get()) { shutdown(shutdown_.promise_); }

    return shutdown_.future_;
}

auto PeerManager::shutdown(std::promise<void>& promise) noexcept -> void
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

auto PeerManager::state_machine() noexcept -> bool { return peers_.Run(); }

PeerManager::~PeerManager()
{
    Shutdown().get();
    shutdown_.Close();
    io_context_.Shutdown();
}
}  // namespace opentxs::blockchain::client::implementation
