// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/client/PeerManager.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string_view>
#include <utility>
#include <vector>

#include "core/Worker.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::PeerManager::"

namespace opentxs::factory
{
auto BlockchainPeerManager(
    const api::client::Manager& api,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::PeerDatabase& database,
    const blockchain::client::internal::IO& io,
    const blockchain::Type type,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::PeerManager>
{
    using ReturnType = blockchain::client::implementation::PeerManager;

    return std::make_unique<ReturnType>(
        api, network, database, io, type, seednode, shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
PeerManager::PeerManager(
    const api::client::Manager& api,
    const internal::Network& network,
    const internal::PeerDatabase& database,
    const blockchain::client::internal::IO& io,
    const Type chain,
    const std::string& seednode,
    const std::string& shutdown) noexcept
    : internal::PeerManager()
    , Worker(api, std::chrono::milliseconds(100))
    , database_(database)
    , io_context_(io)
    , jobs_(api)
    , peers_(
          api,
          network,
          database_,
          *this,
          running_,
          shutdown,
          chain,
          seednode,
          io_context_)
    , init_promise_()
    , init_(init_promise_.get_future())
{
    init_executor({shutdown});
}

PeerManager::Jobs::Jobs(const api::client::Manager& api) noexcept
    : zmq_(api.ZeroMQ())
    , getheaders_(api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , getcfheaders_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , getcfilters_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , heartbeat_(api.ZeroMQ().PublishSocket())
    , getblock_(api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , broadcast_transaction_(
          api.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , endpoint_map_()
    , socket_map_({
          {Task::Getheaders, &getheaders_.get()},
          {Task::Getcfheaders, &getcfheaders_.get()},
          {Task::Getcfilters, &getcfilters_.get()},
          {Task::Heartbeat, &heartbeat_.get()},
          {Task::Getblock, &getblock_.get()},
          {Task::BroadcastTransaction, &broadcast_transaction_.get()},
      })
{
    // NOTE endpoint_map_ should never be modified after construction
    listen(Task::Getheaders, getheaders_);
    listen(Task::Getcfheaders, getcfheaders_);
    listen(Task::Getcfilters, getcfilters_);
    listen(Task::Heartbeat, heartbeat_);
    listen(Task::Getblock, getblock_);
    listen(Task::BroadcastTransaction, broadcast_transaction_);
}

PeerManager::Peers::Peers(
    const api::client::Manager& api,
    const internal::Network& network,
    const internal::PeerDatabase& database,
    const internal::PeerManager& parent,
    const Flag& running,
    const std::string& shutdown,
    const Type chain,
    const std::string& seednode,
    const blockchain::client::internal::IO& context) noexcept
    : api_(api)
    , network_(network)
    , database_(database)
    , parent_(parent)
    , running_(running)
    , shutdown_endpoint_(shutdown)
    , chain_(chain)
    , invalid_peer_(false)
    , localhost_peer_(api.Factory().Data("0x7f000001", StringStyle::Hex))
    , default_peer_(set_default_peer(
          seednode,
          localhost_peer_,
          const_cast<bool&>(invalid_peer_)))
    , context_(context)
    , resolver_(context_.operator boost::asio::io_context &())
    , next_id_(0)
    , minimum_peers_(peer_target_)
    , peers_()
    , active_()
    , count_()
    , connected_()
{
    const auto& data = params::Data::chains_.at(chain_);
    database_.AddOrUpdate(Endpoint{factory::BlockchainAddress(
        api_,
        data.p2p_protocol_,
        p2p::Network::ipv4,
        default_peer_,
        data.default_port_,
        chain_,
        Time{},
        {})});
}

auto PeerManager::Jobs::Dispatch(const Task type) noexcept -> void
{
    Dispatch(Work(type));
}

auto PeerManager::Jobs::Dispatch(zmq::Message& work) noexcept -> void
{
    const auto body = work.Body();

    OT_ASSERT(0 < body.size());

    socket_map_.at(body.at(0).as<Task>())->Send(work);
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

auto PeerManager::Jobs::Work(const Task task, std::promise<void>* promise)
    const noexcept -> OTZMQMessage
{
    if (nullptr != promise) {
        return zmq_.TaggedReply(
            ReadView{reinterpret_cast<char*>(promise), sizeof(promise)}, task);
    } else {
        return zmq_.TaggedMessage(task);
    }
}

auto PeerManager::Peers::add_peer(Endpoint endpoint) noexcept -> void
{
    OT_ASSERT(endpoint);

    const auto address = OTIdentifier{endpoint->ID()};
    auto& count = active_[address];

    if (0 == count) {
        auto addressID = OTIdentifier{endpoint->ID()};
        const auto id = ++next_id_;
        const auto [it, added] =
            peers_.emplace(id, peer_factory(std::move(endpoint), id));

        if (added) {
            ++count;
            ++count_;
            connected_.emplace(std::move(addressID));
        }
    }
}

auto PeerManager::Peers::AddPeer(
    const p2p::Address& address,
    std::promise<bool>& promise) noexcept -> void
{
    if (false == running_) {
        promise.set_value(false);

        return;
    }

    if (address.Chain() != chain_) {
        promise.set_value(false);

        return;
    }

    auto endpoint = Endpoint{factory::BlockchainAddress(
        api_,
        address.Style(),
        address.Type(),
        address.Bytes(),
        address.Port(),
        address.Chain(),
        address.LastConnected(),
        address.Services())};

    OT_ASSERT(endpoint);

    add_peer(std::move(endpoint));
    promise.set_value(true);
}

auto PeerManager::Peers::Disconnect(const int id) noexcept -> void
{
    auto it = peers_.find(id);

    if (peers_.end() == it) { return; }

    auto& peer = *it->second;
    const auto address = peer.AddressID();
    --active_.at(address);
    peer.Shutdown().get();
    it->second.reset();
    peers_.erase(it);
    --count_;
    connected_.erase(address);
}

auto PeerManager::Peers::get_default_peer() const noexcept -> Endpoint
{
    if (localhost_peer_.get() == default_peer_) { return {}; }

    const auto& data = params::Data::chains_.at(chain_);

    return Endpoint{factory::BlockchainAddress(
        api_,
        data.p2p_protocol_,
        p2p::Network::ipv4,
        default_peer_,
        data.default_port_,
        chain_,
        Time{},
        {})};
}

auto PeerManager::Peers::get_dns_peer() const noexcept -> Endpoint
{
    try {
        const auto& data = params::Data::chains_.at(chain_);
        const auto& dns = data.dns_seeds_;
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
        const auto port = data.default_port_;
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
                output = factory::BlockchainAddress(
                    api_,
                    data.p2p_protocol_,
                    p2p::Network::ipv4,
                    api_.Factory().Data(ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
                        bytes.size()}),
                    port,
                    chain_,
                    Time{},
                    {});
            } else if (address.is_v6()) {
                const auto bytes = address.to_v6().to_bytes();
                output = factory::BlockchainAddress(
                    api_,
                    data.p2p_protocol_,
                    p2p::Network::ipv6,
                    api_.Factory().Data(ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
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

auto PeerManager::Peers::get_fallback_peer(
    const p2p::Protocol protocol) const noexcept -> Endpoint
{
    return database_.Get(
        protocol, {p2p::Network::ipv4, p2p::Network::ipv6}, {});
}

auto PeerManager::Peers::get_peer() const noexcept -> Endpoint
{
    const auto protocol = params::Data::chains_.at(chain_).p2p_protocol_;
    auto pAddress = get_default_peer();

    if (pAddress && is_not_connected(*pAddress)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_preferred_peer(protocol);

    if (pAddress && is_not_connected(*pAddress)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(
            ": Attempting to connect to peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_dns_peer();

    if (pAddress && is_not_connected(*pAddress)) {
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

auto PeerManager::Peers::get_preferred_peer(
    const p2p::Protocol protocol) const noexcept -> Endpoint
{
    return database_.Get(
        protocol,
        {p2p::Network::ipv4, p2p::Network::ipv6},
        {p2p::Service::CompactFilters});
}

auto PeerManager::Peers::is_not_connected(
    const p2p::Address& endpoint) const noexcept -> bool
{
    return 0 == connected_.count(endpoint.ID());
}

auto PeerManager::Peers::peer_factory(Endpoint endpoint, const int id) noexcept
    -> p2p::internal::Peer*
{
    switch (chain_) {
        case Type::Bitcoin:
        case Type::Bitcoin_testnet3:
        case Type::BitcoinCash:
        case Type::BitcoinCash_testnet3:
        case Type::Litecoin:
        case Type::Litecoin_testnet4: {
            return factory::BitcoinP2PPeerLegacy(
                api_,
                network_,
                parent_,
                context_,
                id,
                std::move(endpoint),
                shutdown_endpoint_);
        }
        case Type::Unknown:
        case Type::Ethereum_frontier:
        case Type::Ethereum_ropsten:
        case Type::UnitTest:
        default: {
            OT_FAIL;
        }
    }
}

auto PeerManager::Peers::set_default_peer(
    const std::string node,
    const Data& localhost,
    bool& invalidPeer) noexcept -> OTData
{
    if (false == node.empty()) {
        try {
            const auto bytes = ip::make_address_v4(node).to_bytes();

            return Data::Factory(bytes.data(), bytes.size());
        } catch (...) {
            invalidPeer = true;
        }
    }

    return localhost;
}

auto PeerManager::Peers::Run() noexcept -> bool
{
    if ((false == running_) || invalid_peer_) { return false; }

    const auto target = minimum_peers_.load();

    if (target > peers_.size()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Fewer peers (")(peers_.size())(
            ") than desired (")(target)(")")
            .Flush();
        add_peer(get_peer());
    }

    return target > peers_.size();
}

auto PeerManager::Peers::Shutdown() noexcept -> void
{
    OT_ASSERT(false == running_);

    for (auto& [id, peer] : peers_) {
        peer->Shutdown().wait_for(std::chrono::seconds(1));
        peer.reset();
    }

    peers_.clear();
    count_.store(0);
    active_.clear();
}

auto PeerManager::AddPeer(const p2p::Address& address) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    auto promise = std::promise<bool>{};
    auto future = promise.get_future();
    auto work = MakeWork(Work::AddPeer);
    work->AddFrame(reinterpret_cast<std::uintptr_t>(&address));
    work->AddFrame(reinterpret_cast<std::uintptr_t>(&promise));
    pipeline_->Push(work);

    while (running_.get()) {
        if (std::future_status::ready ==
            future.wait_for(std::chrono::seconds(5))) {
            return future.get();
        }
    }

    return false;
}

auto PeerManager::BroadcastTransaction(
    const block::bitcoin::Transaction& tx) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    if (0 == peers_.Count()) { return false; }

    auto bytes = Space{};

    if (false == tx.Serialize(writer(bytes))) { return false; }

    const auto view = reader(bytes);
    auto work = jobs_.Work(Task::BroadcastTransaction);
    work->AddFrame(view.data(), view.size());
    jobs_.Dispatch(work);

    return true;
}

auto PeerManager::Connect() noexcept -> bool
{
    if (false == running_.get()) { return false; }

    trigger();

    return true;
}

auto PeerManager::Disconnect(const int id) const noexcept -> void
{
    auto work = MakeWork(Work::Disconnect);
    work->AddFrame(id);
    pipeline_->Push(work);
}

auto PeerManager::init() noexcept -> void
{
    init_promise_.set_value();
    trigger();
}

auto PeerManager::pipeline(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = message.Body();

    OT_ASSERT(0 < body.size());

    switch (body.at(0).as<Work>()) {
        case Work::Disconnect: {
            OT_ASSERT(1 < body.size());

            peers_.Disconnect(body.at(1).as<int>());
            do_work();
        } break;
        case Work::AddPeer: {
            OT_ASSERT(2 < body.size());

            auto* address_p = reinterpret_cast<const p2p::Address*>(
                body.at(1).as<std::uintptr_t>());
            auto* promise_p = reinterpret_cast<std::promise<bool>*>(
                body.at(2).as<std::uintptr_t>());

            OT_ASSERT(nullptr != address_p);
            OT_ASSERT(nullptr != promise_p);

            const auto& address = *address_p;
            auto& promise = *promise_p;

            peers_.AddPeer(address, promise);
            do_work();
        } break;
        case Work::StateMachine: {
            do_work();
        } break;
        case Work::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto PeerManager::RequestBlock(const block::Hash& block) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    if (0 == peers_.Count()) { return false; }

    auto work = jobs_.Work(Task::Getblock);
    work->AddFrame(block);
    jobs_.Dispatch(work);

    return true;
}

auto PeerManager::RequestFilterHeaders(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    if (0 == peers_.Count()) { return false; }

    auto work = jobs_.Work(Task::Getcfheaders);
    work->AddFrame(type);
    work->AddFrame(start);
    work->AddFrame(stop);
    jobs_.Dispatch(work);

    return true;
}

auto PeerManager::RequestFilters(
    const filter::Type type,
    const block::Height start,
    const block::Hash& stop) const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    if (0 == peers_.Count()) { return false; }

    auto work = jobs_.Work(Task::Getcfilters);
    work->AddFrame(type);
    work->AddFrame(start);
    work->AddFrame(stop);
    jobs_.Dispatch(work);

    return true;
}

auto PeerManager::RequestHeaders() const noexcept -> bool
{
    if (false == running_.get()) { return false; }

    if (0 == peers_.Count()) { return false; }

    jobs_.Dispatch(Task::Getheaders);

    return true;
}

auto PeerManager::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        jobs_.Shutdown();
        peers_.Shutdown();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto PeerManager::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__).Flush();

    if (false == running_.get()) { return false; }

    return peers_.Run();
}

PeerManager::~PeerManager() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
