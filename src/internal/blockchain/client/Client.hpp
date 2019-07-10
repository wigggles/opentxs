// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Network.hpp"

#include "internal/core/Core.hpp"

#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static blockchain::block::Height value() { return -1; }
};
template <>
struct make_blank<blockchain::block::Position> {
    static blockchain::block::Position value()
    {
        return {make_blank<blockchain::block::Height>::value(),
                make_blank<blockchain::block::pHash>::value()};
    }
};
}  // namespace opentxs

namespace opentxs::blockchain::client
{
// parent hash, child hash
using ChainSegment = std::pair<block::pHash, block::pHash>;
using UpdatedHeader = std::map<block::pHash, std::unique_ptr<block::Header>>;
using BestHashes = std::map<block::Height, block::pHash>;
using Hashes = std::set<block::pHash>;
using Segments = std::set<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = std::multimap<block::pHash, block::pHash>;
}  // namespace opentxs::blockchain::client

namespace opentxs::blockchain::client::internal
{
struct FilterDatabase {
    virtual block::Position CurrentTip(const filter::Type type) const
        noexcept = 0;
    virtual bool HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept = 0;
    virtual bool SetTip(const filter::Type type, const block::Position position)
        const noexcept = 0;
    virtual bool StoreFilter(
        const filter::Type type,
        const block::Hash& block,
        std::unique_ptr<const blockchain::internal::GCS> filter) const
        noexcept = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle {
    virtual void AddFilter(
        const filter::Type type,
        const block::Hash& block,
        const Data& filter) const noexcept = 0;
    virtual void CheckBlocks() const noexcept = 0;

    virtual void Start() noexcept = 0;
    virtual void Shutdown() noexcept = 0;

    virtual ~FilterOracle() = default;
};

struct HeaderOracle : virtual public opentxs::blockchain::client::HeaderOracle {
    virtual ~HeaderOracle() = default;
};

struct HeaderDatabase {
    virtual bool ApplyUpdate(
        std::unique_ptr<block::Header> header,
        std::unique_ptr<UpdateTransaction> update) const noexcept = 0;
    virtual bool ApplyUpdate(std::unique_ptr<UpdateTransaction> update) const
        noexcept = 0;
    // Throws std::out_of_range if no block at that position
    virtual block::pHash BestBlock(const block::Height position) const
        noexcept(false) = 0;
    virtual block::Position CurrentCheckpoint() const noexcept = 0;
    virtual std::unique_ptr<block::Header> CurrentBest() const noexcept = 0;
    virtual DisconnectedList DisconnectedHashes() const noexcept = 0;
    virtual bool HasDisconnectedChildren(const block::Hash& hash) const
        noexcept = 0;
    virtual bool HaveCheckpoint() const noexcept = 0;
    virtual bool HeaderExists(const block::Hash& hash) const noexcept = 0;
    virtual bool IsSibling(const block::Hash& hash) const noexcept = 0;
    // Throws std::out_of_range if the header does not exist
    virtual std::unique_ptr<block::Header> LoadHeader(
        const block::Hash& hash) const noexcept(false) = 0;
    virtual std::vector<block::pHash> RecentHashes() const noexcept = 0;
    virtual Hashes SiblingHashes() const noexcept = 0;
    // Returns null pointer if the header does not exist
    virtual std::unique_ptr<block::Header> TryLoadHeader(
        const block::Hash& hash) const noexcept = 0;

    virtual ~HeaderDatabase() = default;
};

struct Network : virtual public opentxs::blockchain::Network {
    virtual Type Chain() const noexcept = 0;
    virtual const network::zeromq::Pipeline& FilterPipeline() const
        noexcept = 0;
    virtual const client::HeaderOracle& HeaderOracle() const noexcept = 0;
    virtual const network::zeromq::Pipeline& HeaderPipeline() const
        noexcept = 0;
    virtual bool IsSynchronized() const noexcept = 0;
    virtual void RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;
    virtual void UpdateHeight(const block::Height height) const noexcept = 0;
    virtual void UpdateLocalHeight(const block::Position position) const
        noexcept = 0;

    virtual client::HeaderOracle& HeaderOracle() noexcept = 0;

    virtual ~Network() = default;
};

struct PeerDatabase {
    using Address = std::unique_ptr<p2p::internal::Address>;
    using Protocol = p2p::Protocol;
    using Service = p2p::Service;
    using Type = p2p::Network;

    virtual bool AddOrUpdate(Address address) const noexcept = 0;
    virtual Address Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task {
        Getcfilters,
    };

    virtual bool AddPeer(const p2p::Address& address) const noexcept = 0;
    virtual bool Connect() noexcept = 0;
    virtual const PeerDatabase& Database() const noexcept = 0;
    virtual void Disconnect(const int id) const noexcept = 0;
    virtual std::string Endpoint(const Task type) const noexcept = 0;
    virtual std::size_t GetPeerCount() const noexcept = 0;
    virtual void RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;

    virtual void init() noexcept = 0;
    virtual void Shutdown() noexcept = 0;

    virtual ~PeerManager() = default;
};

struct UpdateTransaction {
    virtual const BestHashes& BestChain() const = 0;
    virtual const block::Position& Checkpoint() const = 0;
    virtual const Segments& Connected() const = 0;
    virtual const Segments& Disconnected() const = 0;
    virtual bool HaveCheckpoint() const = 0;
    virtual bool HaveReorg() const = 0;
    // throws std::out_of_range if header does not exist
    virtual const block::Header& Header(const block::Hash& hash) const = 0;
    virtual bool IsInBestChain(const block::Position& position) const = 0;
    virtual bool IsInBestChain(
        const block::Height height,
        const block::Hash& hash) const = 0;
    virtual const block::Position& ReorgParent() const = 0;
    virtual const Hashes& SiblingsToAdd() const = 0;
    virtual const Hashes& SiblingsToDelete() const = 0;
    virtual const block::Position& Tip() const = 0;
    virtual bool TipIsBest() const = 0;

    virtual void AddSibling(const block::Position& position) = 0;
    virtual void AddToBestChain(const block::Position& position) = 0;
    virtual void AddToBestChain(
        const block::Height height,
        const block::Hash& hash) = 0;
    virtual void ClearCheckpoint() = 0;
    virtual void ConnectBlock(ChainSegment&& segment) = 0;
    virtual void DisconnectBlock(const block::Header& header) = 0;
    virtual block::Header& ModifyExistingBlock(
        std::unique_ptr<block::Header> header) = 0;
    virtual void RemoveSibling(const block::Hash& hash) = 0;
    virtual void SetCheckpoint(block::Position&& checkpoint) = 0;
    virtual void SetReorg(const bool value) = 0;
    virtual void SetReorgParent(const block::Position& pos) = 0;
    virtual void SetTip(const block::Position& position) = 0;
    virtual void SetTipBest(const bool isBest) = 0;
    virtual UpdatedHeader& UpdatedHeaders() = 0;

    virtual ~UpdateTransaction() = default;
};

struct Wallet {
    virtual ~Wallet() = default;
};
}  // namespace opentxs::blockchain::client::internal
