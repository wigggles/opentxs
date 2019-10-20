// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/core/Log.hpp"

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

namespace opentxs
{
blockchain::client::internal::PeerManager* Factory::BlockchainPeerManager(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::Type type,
    const std::string& seednode)
{
    using ReturnType = blockchain::client::implementation::PeerManager;

    return new ReturnType{api, network, type, seednode};
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
    const Type chain,
    const std::string& seednode) noexcept
    : internal::PeerManager()
    , StateMachine(std::bind(&PeerManager::state_machine, this))
    , api_(api)
    , io_context_()
    , peers_(api, network, *this, chain, seednode, io_context_)
    , heartbeat_task_()
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

PeerManager::Peers::Peers(
    const api::internal::Core& api,
    const internal::Network& network,
    const internal::PeerManager& parent,
    const Type chain,
    const std::string& seednode,
    boost::asio::io_context& context) noexcept
    : api_(api)
    , network_(network)
    , parent_(parent)
    , chain_(chain)
    , default_peer_(set_default_peer(seednode))
    , context_(context)
    , lock_()
    , queue_lock_()
    , next_id_(0)
    , minimum_peers_(3)
    , disconnect_peers_()
    , peers_()
    , active_()
{
}

void PeerManager::IO::Shutdown() noexcept
{
    work_.reset();
    thread_pool_.join_all();
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

        if (added) { ++count; }
    }
}

bool PeerManager::Peers::AddPeer(const p2p::Address& address) noexcept
{
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

    auto pAddress = network_.Database().Get(
        protocol,
        {p2p::Network::ipv4},  // TODO
        {});                   // TODO

    if (pAddress) {
        return pAddress;
    } else {
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
    Lock lock(lock_);
    std::for_each(
        std::begin(peers_), std::end(peers_), [](const auto& it) -> void {
            const auto& peer = *it.second;
            peer.Heartbeat();
        });
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
                api_, network_, parent_, id, std::move(endpoint), context_);
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
    Lock lock(queue_lock_);
    disconnect_peers_.push(id);
}

bool PeerManager::Peers::Run() noexcept
{
    Lock queueLock(queue_lock_);
    auto disconnect{disconnect_peers_};
    queueLock.unlock();
    Lock peerLock(lock_);

    while (false == disconnect.empty()) {
        const auto id = disconnect.front();
        disconnect.pop();
        auto it = peers_.find(id);

        if (peers_.end() == it) { continue; }

        const auto address = it->second->AddressID();
        peers_.erase(it);
        --active_.at(address);
    }

    const auto target = minimum_peers_.load();

    if (target > peers_.size()) { add_peer(peerLock, get_peer()); }

    return target > peers_.size();
}

void PeerManager::Peers::Shutdown() noexcept
{
    Lock lock(lock_);
    peers_.clear();
}

bool PeerManager::AddPeer(const p2p::Address& address) const noexcept
{
    const auto output = peers_.AddPeer(address);
    Trigger();

    return output;
}

void PeerManager::Disconnect(const int id) const noexcept
{
    peers_.QueueDisconnect(id);
    Trigger();
}

void PeerManager::init() noexcept
{
    Trigger();
    heartbeat_task_ = api_.Schedule(
        std::chrono::seconds(1), [this]() -> void { this->Heartbeat(); });
}

bool PeerManager::state_machine() noexcept { return peers_.Run(); }

PeerManager::~PeerManager()
{
    api_.Cancel(heartbeat_task_);
    Stop().get();
    peers_.Shutdown();
    io_context_.Shutdown();
}
}  // namespace opentxs::blockchain::client::implementation
