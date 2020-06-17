// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/Database.cpp"

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "api/client/blockchain/database/Database.hpp"
#include "blockchain/database/Blocks.hpp"
#include "blockchain/database/Filters.hpp"
#include "blockchain/database/Headers.hpp"
#include "blockchain/database/Wallet.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal

namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class Header;

namespace bitcoin
{
class Block;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace client
{
class UpdateTransaction;
}  // namespace client
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
class Database final : virtual public internal::Database
{
public:
    auto AddConfirmedTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction,
        const VersionNumber version) const noexcept -> bool final
    {
        return wallet_.AddConfirmedTransaction(
            balanceNode,
            subchain,
            type,
            version,
            block,
            outputIndices,
            transaction);
    }
    auto AddOrUpdate(Address address) const noexcept -> bool final
    {
        return common_.AddOrUpdate(std::move(address));
    }
    auto ApplyUpdate(const client::UpdateTransaction& update) const noexcept
        -> bool final
    {
        return headers_.ApplyUpdate(update);
    }
    // Throws std::out_of_range if no block at that position
    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash final
    {
        return headers_.BestBlock(position);
    }
    auto BlockExists(const block::Hash& block) const noexcept -> bool final
    {
        return common_.BlockExists(block);
    }
    auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> final
    {
        return blocks_.LoadBitcoin(block);
    }
    auto BlockPolicy() const noexcept
        -> api::client::blockchain::BlockStorage final
    {
        return common_.BlockPolicy();
    }
    auto BlockStore(const block::Block& block) const noexcept -> bool final
    {
        return blocks_.Store(block);
    }
    auto CurrentBest() const noexcept -> std::unique_ptr<block::Header> final
    {
        return headers_.CurrentBest();
    }
    auto CurrentCheckpoint() const noexcept -> block::Position final
    {
        return headers_.CurrentCheckpoint();
    }
    auto FilterHeaderTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentHeaderTip(type);
    }
    auto FilterTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentTip(type);
    }
    auto DisconnectedHashes() const noexcept -> client::DisconnectedList final
    {
        return headers_.DisconnectedHashes();
    }
    auto Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address final
    {
        return common_.Find(chain_, protocol, onNetworks, withServices);
    }
    auto GetBalance() const noexcept -> Balance final
    {
        return wallet_.GetBalance();
    }
    auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> Patterns final
    {
        return wallet_.GetPatterns(balanceNode, subchain, type, version);
    }
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs();
    }
    auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> Patterns
    {
        return wallet_.GetUntestedPatterns(
            balanceNode, subchain, type, blockID, version);
    }
    auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool final
    {
        return headers_.HasDisconnectedChildren(hash);
    }
    auto HaveCheckpoint() const noexcept -> bool final
    {
        return headers_.HaveCheckpoint();
    }
    auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilter(type, block);
    }
    auto HaveFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilterHeader(type, block);
    }
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.HeaderExists(hash);
    }
    auto Import(std::vector<Address> peers) const noexcept -> bool final
    {
        return common_.Import(std::move(peers));
    }
    auto IsSibling(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.IsSibling(hash);
    }
    auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const blockchain::internal::GCS> final
    {
        return filters_.LoadFilter(type, block);
    }
    auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash final
    {
        return filters_.LoadFilterHash(type, block);
    }
    auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash final
    {
        return filters_.LoadFilterHeader(type, block);
    }
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> final
    {
        return headers_.LoadHeader(hash);
    }
    auto RecentHashes() const noexcept -> std::vector<block::pHash> final
    {
        return headers_.RecentHashes();
    }
    auto ReorgTo(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const std::vector<block::Position>& reorg) const noexcept -> bool final
    {
        return wallet_.ReorgTo(balanceNode, subchain, type, reorg);
    }
    auto SetFilterHeaderTip(
        const filter::Type type,
        const block::Position position) const noexcept -> bool final
    {
        return filters_.SetHeaderTip(type, position);
    }
    auto SetFilterTip(const filter::Type type, const block::Position position)
        const noexcept -> bool final
    {
        return filters_.SetTip(type, position);
    }
    auto SiblingHashes() const noexcept -> client::Hashes final
    {
        return headers_.SiblingHashes();
    }
    auto StoreFilters(const filter::Type type, std::vector<Filter> filters)
        const noexcept -> bool final
    {
        return filters_.StoreFilters(type, std::move(filters));
    }
    auto StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept -> bool final
    {
        return filters_.StoreHeaders(type, previous, std::move(headers));
    }
    auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version) const noexcept -> bool final
    {
        return wallet_.SubchainAddElements(
            balanceNode, subchain, type, elements, version);
    }
    auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool final
    {
        return wallet_.SubchainDropIndex(balanceNode, subchain, type, version);
    }
    auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber final
    {
        return wallet_.SubchainIndexVersion(balanceNode, subchain, type);
    }
    auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept
        -> std::optional<Bip32Index> final
    {
        return wallet_.SubchainLastIndexed(
            balanceNode, subchain, type, version);
    }
    auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position final
    {
        return wallet_.SubchainLastProcessed(balanceNode, subchain, type);
    }
    auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position final
    {
        return wallet_.SubchainLastScanned(balanceNode, subchain, type);
    }
    auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> bool final
    {
        return wallet_.SubchainMatchBlock(
            balanceNode, subchain, type, indices, blockID, version);
    }
    auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool
    {
        return wallet_.SubchainSetLastProcessed(
            balanceNode, subchain, type, position);
    }
    auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool
    {
        return wallet_.SubchainSetLastScanned(
            balanceNode, subchain, type, position);
    }
    // Returns null pointer if the header does not exist
    auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> final
    {
        return headers_.TryLoadHeader(hash);
    }

    Database(
        const api::internal::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const client::internal::Network& network,
        const database::Common& common,
        const blockchain::Type type) noexcept;

    ~Database() = default;

private:
    friend opentxs::Factory;

    static const std::size_t db_version_;
    static const opentxs::storage::lmdb::TableNames table_names_;

    const blockchain::Type chain_;
    const database::Common& common_;
    opentxs::storage::lmdb::LMDB lmdb_;
    mutable database::Blocks blocks_;
    mutable database::Filters filters_;
    mutable database::Headers headers_;
    mutable database::Wallet wallet_;

    void init_db() noexcept;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database &&) -> Database& = delete;
};
}  // namespace opentxs::blockchain::implementation
