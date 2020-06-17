// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
#include <tuple>
#include <utility>
#include <vector>

#include "api/client/blockchain/database/Database.hpp"
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
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Core;

namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Output;
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Wallet
{
public:
    using Common = api::client::blockchain::database::implementation::Database;
    using Parent = client::internal::WalletDatabase;
    using FilterType = Parent::FilterType;
    using NodeID = Parent::NodeID;
    using pNodeID = Parent::pNodeID;
    using Subchain = Parent::Subchain;
    using ElementID = Parent::ElementID;
    using ElementMap = Parent::ElementMap;
    using Pattern = Parent::Pattern;
    using Patterns = Parent::Patterns;
    using MatchingIndices = Parent::MatchingIndices;
    using UTXO = Parent::UTXO;

    auto AddConfirmedTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> Patterns;
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO>;
    auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> Patterns;
    auto ReorgTo(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const std::vector<block::Position>& reorg) const noexcept -> bool;
    auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber;
    auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;
    auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;

    Wallet(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const blockchain::Type chain) noexcept;

private:
    enum class State : std::uint8_t {
        UnconfirmedNew,
        UnconfirmedSpend,
        ConfirmedNew,
        ConfirmedSpend,
        OrphanedNew,
        OrphanedSpend,
    };

    using SubchainID = Identifier;
    using pSubchainID = OTIdentifier;
    using PatternID = Identifier;
    using pPatternID = OTIdentifier;
    using IDSet = boost::container::flat_set<pPatternID>;
    using PatternMap =
        std::map<pPatternID, std::vector<std::pair<Bip32Index, Space>>>;
    using SubchainPatternIndex = std::map<pSubchainID, IDSet>;
    using SubchainIndexMap = std::map<pSubchainID, VersionNumber>;
    using VersionIndex = std::map<OTIdentifier, VersionNumber>;
    using PositionMap = std::map<OTIdentifier, block::Position>;
    using MatchIndex = std::map<block::pHash, IDSet>;
    using OutputMap = std::map<
        block::bitcoin::Outpoint,
        std::tuple<State, block::Position, proto::BlockchainTransactionOutput>>;
    using OutputPositionIndex =
        std::map<block::Position, std::vector<block::bitcoin::Outpoint>>;
    using OutputStateIndex =
        std::map<State, std::vector<block::bitcoin::Outpoint>>;
    using OutputSubchainIndex =
        std::map<pSubchainID, std::vector<block::bitcoin::Outpoint>>;
    using TransactionMap = std::map<block::pTxid, proto::BlockchainTransaction>;
    using TransactionBlockMap =
        std::map<block::pTxid, std::vector<block::pHash>>;
    using BlockTransactionMap =
        std::map<block::pHash, std::vector<block::pTxid>>;

    const api::Core& api_;
    const api::client::internal::Blockchain& blockchain_;
    const blockchain::Type chain_;
    mutable std::mutex lock_;
    mutable PatternMap patterns_;
    mutable SubchainPatternIndex subchain_pattern_index_;
    mutable SubchainIndexMap subchain_last_indexed_;
    mutable VersionIndex subchain_version_;
    mutable PositionMap subchain_last_scanned_;
    mutable PositionMap subchain_last_processed_;
    mutable MatchIndex match_index_;
    mutable OutputMap outputs_;
    mutable OutputPositionIndex output_positions_;
    mutable OutputStateIndex output_states_;
    mutable OutputSubchainIndex output_subchain_;
    mutable TransactionMap transactions_;
    mutable TransactionBlockMap tx_to_block_;
    mutable BlockTransactionMap block_to_tx_;

    auto belongs_to(
        const Lock& lock,
        const block::bitcoin::Outpoint& id,
        const SubchainID& subchain) const noexcept -> bool;
    auto effective_position(const State state, const block::Position& position)
        const noexcept -> const block::Position&;
    auto get_balance(const Lock& lock) const noexcept -> Balance;
    auto get_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept(false) -> const IDSet&;
    auto get_unspent_outputs(const Lock& lock) const noexcept
        -> std::vector<UTXO>;
    template <typename PatternList>
    auto load_patterns(
        const Lock& lock,
        const NodeID& balanceNode,
        const Subchain subchain,
        const PatternList& patterns) const noexcept -> Patterns
    {
        auto output = Patterns{};

        for (const auto& patternID : patterns) {
            try {
                for (const auto& [index, pattern] : patterns_.at(patternID)) {
                    output.emplace_back(
                        Pattern{{index, {subchain, balanceNode}}, pattern});
                }
            } catch (...) {
            }
        }

        return output;
    }

    auto add_transaction(
        const Lock& lock,
        const block::Hash& block,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto change_state(
        const Lock& lock,
        const block::bitcoin::Outpoint& id,
        const State newState,
        const block::Position newPosition) const noexcept -> bool;
    auto create_state(
        const Lock& lock,
        const block::bitcoin::Outpoint& id,
        const State state,
        const block::Position position,
        const block::bitcoin::Output& output) const noexcept -> bool;
    auto find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
        const noexcept -> std::optional<OutputMap::iterator>;
    auto pattern_id(const SubchainID& subchain, const Bip32Index index)
        const noexcept -> pPatternID;
    auto rollback(
        const Lock& lock,
        const SubchainID& subchain,
        const block::Position& position) const noexcept -> bool;
    auto subchain_version_index(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainID;
    auto subchain_id(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainID;
};
}  // namespace opentxs::blockchain::database
