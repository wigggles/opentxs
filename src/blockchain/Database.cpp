// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/Blockchain.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>

#include "Database.hpp"

// #define OT_METHOD "opentxs::blockchain::implementation::Database::"

namespace opentxs
{
blockchain::internal::Database* Factory::BlockchainDatabase(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const blockchain::Type type)
{
    using ReturnType = blockchain::implementation::Database;

    return new ReturnType(api, network, type);
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
Database::Database(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const blockchain::Type type) noexcept
    : internal::Database()
    , headers_(api, network, type)
    , peers_()
{
}

Database::Headers::Headers(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const blockchain::Type type) noexcept
    : api_(api)
    , network_(network)
    , lock_()
    , block_headers_(init_genesis(api, type))
    , best_chain_({{0, client::HeaderOracle::GenesisBlockHash(type)}})
    , disconnected_()
    , checkpoint_(make_blank<block::Position>::value())
    , sibling_chains_()
{
    OT_ASSERT(1 == block_headers_.size());
    OT_ASSERT(1 == best_chain_.size());
    OT_ASSERT(1 == block_headers_.count(best_chain_.crbegin()->second));
}

Database::Peers::Peers() noexcept
    : lock_()
    , addresses_()
    , protocols_()
    , services_()
    , networks_()
{
}

bool Database::Headers::ApplyUpdate(
    std::unique_ptr<block::Header> newHeader,
    std::unique_ptr<client::internal::UpdateTransaction> pUpdate) noexcept
{
    Lock lock(lock_);

    if (false == bool(pUpdate)) { return false; }

    auto& update = *pUpdate;

    // TODO start atomic database transaction

    if (newHeader) {
        block_headers_.emplace(newHeader->Hash(), std::move(newHeader));
    }

    if (update.HaveCheckpoint()) { checkpoint_ = update.Checkpoint(); }

    for (const auto& it : update.Disconnected()) { disconnected_.emplace(it); }

    for (const auto& [parent, child] : update.Connected()) {
        const auto [first, last] = disconnected_.equal_range(parent);

        for (auto i = first; i != last;) {
            if (child == i->second) {
                i = disconnected_.erase(i);
            } else {
                ++i;
            }
        }
    }

    for (const auto& hash : update.SiblingsToAdd()) {
        sibling_chains_.emplace(hash);
    }

    for (const auto& hash : update.SiblingsToDelete()) {
        sibling_chains_.erase(hash);
    }

    for (auto& [hash, header] : update.UpdatedHeaders()) {
        block_headers_.at(hash).reset(header.release());
    }

    if (update.HaveReorg()) {
        while (static_cast<std::size_t>(update.ReorgParent().first) + 1 <
               best_chain_.size()) {
            best_chain_.erase(best_chain_.rbegin()->first);
        }
    }

    for (const auto& [height, hash] : update.BestChain()) {
        OT_ASSERT(1 == block_headers_.count(hash));

        auto it = best_chain_.find(height);

        if (best_chain_.end() == it) {
            best_chain_.emplace(height, hash);
        } else {
            it->second = hash;
        }
    }

    // TODO commit database transaction

    if (update.HaveReorg()) {
        LogVerbose("Blockchain reorg detected. Last common ancestor is ")(
            update.ReorgParent().second->asHex())(" at height ")(
            update.ReorgParent().first)
            .Flush();

        // TODO broadcast reorg signal
    }

    network_.UpdateLocalHeight(*best_chain_.crbegin());

    return true;
}

block::pHash Database::Headers::BestBlock(const block::Height position) const
    noexcept(false)
{
    Lock lock(lock_);

    return best_chain_.at(position);
}

Database::Headers::HeaderMap Database::Headers::init_genesis(
    const api::internal::Core& api,
    const blockchain::Type type) noexcept
{
    std::map<block::pHash, std::unique_ptr<block::Header>> output{};
    output.emplace(
        client::HeaderOracle::GenesisBlockHash(type),
        opentxs::Factory::GenesisBlockHeader(api, type));

    return output;
}

std::unique_ptr<block::Header> Database::Headers::CurrentBest() const noexcept
{
    Lock lock(lock_);

    return load_header(lock, best_chain_.crbegin()->second);
}

block::Position Database::Headers::CurrentCheckpoint() const noexcept
{
    Lock lock(lock_);

    return checkpoint_;
}

client::DisconnectedList Database::Headers::DisconnectedHashes() const noexcept
{
    Lock lock(lock_);

    return disconnected_;
}

bool Database::Headers::HasDisconnectedChildren(const block::Hash& hash) const
    noexcept
{
    Lock lock(lock_);

    return 0 < disconnected_.count(hash);
}

bool Database::Headers::HaveCheckpoint() const noexcept
{
    Lock lock(lock_);

    return make_blank<block::Position>::value() != checkpoint_;
}

bool Database::Headers::HeaderExists(const block::Hash& hash) const noexcept
{
    Lock lock(lock_);

    return 0 < block_headers_.count(hash);
}

bool Database::Headers::IsSibling(const block::Hash& hash) const noexcept
{
    Lock lock(lock_);

    return 0 < sibling_chains_.count(hash);
}

std::unique_ptr<block::Header> Database::Headers::LoadHeader(
    const block::Hash& hash) const
{
    Lock lock(lock_);

    return load_header(lock, hash);
}

std::unique_ptr<block::Header> Database::Headers::load_header(
    const Lock& lock,
    const block::Hash& hash) const
{
    const auto& output = block_headers_.at(hash);

    OT_ASSERT(output);

    return output->clone();
}

std::vector<block::pHash> Database::Headers::RecentHashes() const noexcept
{
    Lock lock(lock_);

    return recent_hashes(lock);
}

std::vector<block::pHash> Database::Headers::recent_hashes(
    const Lock& lock) const noexcept
{
    std::vector<block::pHash> output{};

    for (auto i = best_chain_.crbegin(); i != best_chain_.crend(); ++i) {
        output.emplace_back(i->second);

        if (100 <= output.size()) { break; }
    }

    return output;
}

client::Hashes Database::Headers::SiblingHashes() const noexcept
{
    Lock lock(lock_);

    return sibling_chains_;
}

std::unique_ptr<block::Header> Database::Headers::TryLoadHeader(
    const block::Hash& hash) const noexcept
{
    try {
        return LoadHeader(hash);
    } catch (...) {
        return {};
    }
}

Database::Address Database::Peers::Find(
    const Protocol protocol,
    const std::set<Type> onNetworks,
    const std::set<Service> withServices) const noexcept
{
    Lock lock(lock_);

    try {
        std::set<OTIdentifier> candidates{};
        const auto& protocolSet = protocols_.at(protocol);

        if (protocolSet.empty()) { return {}; }

        for (const auto& network : onNetworks) {
            try {
                for (const auto& id : networks_.at(network)) {
                    if (1 == protocolSet.count(id)) { candidates.emplace(id); }
                }
            } catch (...) {
            }
        }

        if (candidates.empty()) { return {}; }

        std::set<OTIdentifier> haveServices{};

        if (withServices.empty()) {
            haveServices = candidates;
        } else {
            for (const auto& id : candidates) {
                bool haveAllServices{true};

                for (const auto& service : withServices) {
                    try {
                        if (0 == services_.at(service).count(id)) {
                            haveAllServices = false;
                            break;
                        }
                    } catch (...) {
                        haveAllServices = false;
                        break;
                    }
                }

                if (haveAllServices) { haveServices.emplace(id); }
            }
        }

        if (haveServices.empty()) { return {}; }

        std::vector<OTIdentifier> output;
        const std::size_t count{1};
        std::sample(
            haveServices.begin(),
            haveServices.end(),
            std::back_inserter(output),
            count,
            std::mt19937{std::random_device{}()});

        OT_ASSERT(count == output.size());

        return addresses_.at(output.front())->clone_internal();
    } catch (...) {
        return {};
    }
}

bool Database::Peers::Insert(Address pAddress) noexcept
{
    if (false == bool(pAddress)) { return false; }

    Lock lock(lock_);
    auto it = addresses_.find(pAddress->ID());
    std::set<Service> oldServices{};
    bool newAddress{false};

    if (addresses_.end() == it) {
        newAddress = true;
        auto [it2, added] =
            addresses_.emplace(pAddress->ID(), std::move(pAddress));
        it = it2;
    } else {
        oldServices = pAddress->Services();
        it->second.reset(pAddress.release());
    }

    OT_ASSERT(it->second);

    const auto& address = *it->second;
    const auto& id = address.ID();
    const auto newServices{address.Services()};

    if (newAddress) {
        protocols_[address.Style()].emplace(id);
        networks_[address.Type()].emplace(id);

        for (const auto& service : newServices) {
            services_[service].emplace(id);
        }
    } else {
        for (const auto& service : oldServices) {
            if (0 == newServices.count(service)) {
                services_[service].erase(id);
            }
        }
    }

    return true;
}
}  // namespace opentxs::blockchain::implementation
