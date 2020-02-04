// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#if OT_BLOCKCHAIN
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/Bytes.hpp"
#endif  // OT_BLOCKCHAIN

#include "internal/core/Core.hpp"

#include <future>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

#if OT_BLOCKCHAIN
namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static blockchain::block::Height value(const api::Core&) { return -1; }
};
template <>
struct make_blank<blockchain::block::Position> {
    static blockchain::block::Position value(const api::Core& api)
    {
        return {make_blank<blockchain::block::Height>::value(api),
                make_blank<blockchain::block::pHash>::value(api)};
    }
};
}  // namespace opentxs

namespace opentxs::blockchain::client
{
// parent hash, child hash
using ChainSegment = std::pair<block::pHash, block::pHash>;
using UpdatedHeader =
    std::map<block::pHash, std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = std::map<block::Height, block::pHash>;
using Hashes = std::set<block::pHash>;
using Segments = std::set<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = std::multimap<block::pHash, block::pHash>;
}  // namespace opentxs::blockchain::client
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::client::internal
{
#if OT_BLOCKCHAIN
struct FilterDatabase {
    using Hash = block::pHash;
    /// block hash, filter header, filter hash
    using Header = std::tuple<block::pHash, block::pHash, ReadView>;
    using Filter =
        std::pair<ReadView, std::unique_ptr<const blockchain::internal::GCS>>;

    virtual block::Position FilterHeaderTip(const filter::Type type) const
        noexcept = 0;
    virtual block::Position FilterTip(const filter::Type type) const
        noexcept = 0;
    virtual bool HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept = 0;
    virtual bool HaveFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept = 0;
    virtual Hash LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept = 0;
    virtual Hash LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept = 0;
    virtual bool SetFilterHeaderTip(
        const filter::Type type,
        const block::Position position) const noexcept = 0;
    virtual bool SetFilterTip(
        const filter::Type type,
        const block::Position position) const noexcept = 0;
    virtual bool StoreFilters(
        const filter::Type type,
        std::vector<Filter> filters) const noexcept = 0;
    virtual bool StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle {
    virtual void AddFilter(
        const filter::Type type,
        const block::Hash& block,
        const Data& filter) const noexcept = 0;
    virtual void AddHeaders(
        const filter::Type type,
        const ReadView stopBlock,
        const ReadView previousHeader,
        const std::vector<ReadView> hashes) const noexcept = 0;
    virtual void CheckBlocks() const noexcept = 0;

    virtual void Start() noexcept = 0;
    virtual std::shared_future<void> Shutdown() noexcept = 0;

    virtual ~FilterOracle() = default;
};

struct HeaderOracle : virtual public opentxs::blockchain::client::HeaderOracle {
    virtual ~HeaderOracle() = default;
};

struct HeaderDatabase {
    virtual bool ApplyUpdate(const client::UpdateTransaction& update) const
        noexcept = 0;
    // Throws std::out_of_range if no block at that position
    virtual block::pHash BestBlock(const block::Height position) const
        noexcept(false) = 0;
    virtual std::unique_ptr<block::Header> CurrentBest() const noexcept = 0;
    virtual block::Position CurrentCheckpoint() const noexcept = 0;
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
    virtual const network::zeromq::Pipeline& FilterHeaderPipeline() const
        noexcept = 0;
    virtual const network::zeromq::Pipeline& FilterPipeline() const
        noexcept = 0;
    virtual const client::HeaderOracle& HeaderOracle() const noexcept = 0;
    virtual const network::zeromq::Pipeline& HeaderPipeline() const
        noexcept = 0;
    virtual bool IsSynchronized() const noexcept = 0;
    virtual void RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;
    virtual void RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;
    virtual void UpdateHeight(const block::Height height) const noexcept = 0;
    virtual void UpdateLocalHeight(const block::Position position) const
        noexcept = 0;

    virtual client::HeaderOracle& HeaderOracle() noexcept = 0;
    virtual std::shared_future<void> Shutdown() noexcept = 0;

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
    virtual bool Import(std::vector<Address> peers) const noexcept = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task {
        Getheaders,
        Getcfheaders,
        Getcfilters,
        Heartbeat,
    };

    virtual bool AddPeer(const p2p::Address& address) const noexcept = 0;
    virtual bool Connect() noexcept = 0;
    virtual const PeerDatabase& Database() const noexcept = 0;
    virtual void Disconnect(const int id) const noexcept = 0;
    virtual std::string Endpoint(const Task type) const noexcept = 0;
    virtual std::size_t GetPeerCount() const noexcept = 0;
    virtual void RequestFilterHeaders(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;
    virtual void RequestFilters(
        const filter::Type type,
        const block::Height start,
        const block::Hash& stop) const noexcept = 0;
    virtual void RequestHeaders() const noexcept = 0;

    virtual void init() noexcept = 0;
    virtual void Run() noexcept = 0;
    virtual std::shared_future<void> Shutdown() noexcept = 0;

    virtual ~PeerManager() = default;
};
#endif  // OT_BLOCKCHAIN

struct Wallet {
    virtual ~Wallet() = default;
};
}  // namespace opentxs::blockchain::client::internal
