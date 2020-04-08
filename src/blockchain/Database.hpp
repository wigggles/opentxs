// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::implementation
{
class Database final : virtual public internal::Database
{
public:
    using Common = api::client::blockchain::database::implementation::Database;

    auto AddConfirmedTransaction(
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool final
    {
        return wallet_.AddConfirmedTransaction(
            block, outputIndices, transaction);
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
    auto GetBalance() const noexcept -> BalanceData final
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
    auto HaveFilter(const filter::Type type, const block::Hash& block) const
        noexcept -> bool final
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
    auto LoadFilter(const filter::Type type, const ReadView block) const
        noexcept -> std::unique_ptr<const blockchain::internal::GCS> final
    {
        return filters_.LoadFilter(type, block);
    }
    auto LoadFilterHash(const filter::Type type, const ReadView block) const
        noexcept -> Hash final
    {
        return filters_.LoadFilterHash(type, block);
    }
    auto LoadFilterHeader(const filter::Type type, const ReadView block) const
        noexcept -> Hash final
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
        const client::internal::Network& network,
        const Common& common,
        const blockchain::Type type) noexcept;

    ~Database() = default;

private:
    friend opentxs::Factory;

    struct Filters {
        auto CurrentHeaderTip(const filter::Type type) const noexcept
            -> block::Position;
        auto CurrentTip(const filter::Type type) const noexcept
            -> block::Position;
        auto HaveFilter(const filter::Type type, const block::Hash& block) const
            noexcept -> bool
        {
            return common_.HaveFilter(type, block.Bytes());
        }
        auto HaveFilterHeader(const filter::Type type, const block::Hash& block)
            const noexcept -> bool
        {
            return common_.HaveFilterHeader(type, block.Bytes());
        }
        auto LoadFilter(const filter::Type type, const ReadView block) const
            noexcept -> std::unique_ptr<const blockchain::internal::GCS>
        {
            return common_.LoadFilter(type, block);
        }
        auto LoadFilterHash(const filter::Type type, const ReadView block) const
            noexcept -> Hash;
        auto LoadFilterHeader(const filter::Type type, const ReadView block)
            const noexcept -> Hash;
        auto SetHeaderTip(
            const filter::Type type,
            const block::Position position) const noexcept -> bool;
        auto SetTip(const filter::Type type, const block::Position position)
            const noexcept -> bool;
        auto StoreHeaders(
            const filter::Type type,
            const ReadView previous,
            const std::vector<Header> headers) const noexcept -> bool
        {
            return common_.StoreFilterHeaders(type, headers);
        }
        auto StoreFilters(const filter::Type type, std::vector<Filter> filters)
            const noexcept -> bool
        {
            return common_.StoreFilters(type, filters);
        }

        Filters(
            const api::internal::Core& api,
            const Common& common,
            const opentxs::storage::lmdb::LMDB& lmdb,
            const blockchain::Type chain) noexcept;

    private:
        static const std::map<
            blockchain::Type,
            std::map<filter::Type, std::pair<std::string, std::string>>>
            genesis_filters_;

        const api::internal::Core& api_;
        const Common& common_;
        const opentxs::storage::lmdb::LMDB& lmdb_;
        const block::Position blank_position_;
        mutable std::mutex lock_;

        auto import_genesis(const blockchain::Type type) const noexcept -> void;
    };

    struct Headers {
        block::pHash BestBlock(const block::Height position) const
            noexcept(false);
        std::unique_ptr<block::Header> CurrentBest() const noexcept
        {
            return load_header(best().second);
        }
        block::Position CurrentCheckpoint() const noexcept;
        client::DisconnectedList DisconnectedHashes() const noexcept;
        bool HasDisconnectedChildren(const block::Hash& hash) const noexcept;
        bool HaveCheckpoint() const noexcept;
        bool HeaderExists(const block::Hash& hash) const noexcept;
        void import_genesis(const blockchain::Type type) const noexcept;
        bool IsSibling(const block::Hash& hash) const noexcept;
        // Throws std::out_of_range if the header does not exist
        std::unique_ptr<block::Header> LoadHeader(const block::Hash& hash) const
        {
            return load_header(hash);
        }
        std::vector<block::pHash> RecentHashes() const noexcept;
        client::Hashes SiblingHashes() const noexcept;
        // Returns null pointer if the header does not exist
        std::unique_ptr<block::Header> TryLoadHeader(
            const block::Hash& hash) const noexcept;

        bool ApplyUpdate(const client::UpdateTransaction& update) noexcept;

        Headers(
            const api::internal::Core& api,
            const client::internal::Network& network,
            const Common& common,
            const opentxs::storage::lmdb::LMDB& lmdb,
            const blockchain::Type type) noexcept;

    private:
        const api::internal::Core& api_;
        const client::internal::Network& network_;
        const Common& common_;
        const opentxs::storage::lmdb::LMDB& lmdb_;
        mutable std::mutex lock_;

        block::Position best() const noexcept;
        block::Position best(const Lock& lock) const noexcept;
        block::Position checkpoint(const Lock& lock) const noexcept;
        bool header_exists(const Lock& lock, const block::Hash& hash) const
            noexcept;
        // Throws std::out_of_range if the header does not exist
        std::unique_ptr<block::Header> load_header(
            const block::Hash& hash) const noexcept(false);
        bool pop_best(const std::size_t i, MDB_txn* parent) const noexcept;
        bool push_best(
            const block::Position next,
            const bool setTip,
            MDB_txn* parent) const noexcept;
        std::vector<block::pHash> recent_hashes(const Lock& lock) const
            noexcept;
    };

    struct Wallet {
        auto AddConfirmedTransaction(
            const block::Position& block,
            const std::vector<std::uint32_t> outputIndices,
            const block::bitcoin::Transaction& transaction) const noexcept
            -> bool;
        auto GetBalance() const noexcept -> BalanceData;
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

        Wallet(const api::Core& api) noexcept;

    private:
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
            std::pair<block::Height, proto::BlockchainTransactionOutput>>;
        using OutputStateMap =
            std::map<block::Height, std::vector<block::bitcoin::Outpoint>>;
        using TransactionMap =
            std::map<block::pTxid, proto::BlockchainTransaction>;
        using TransactionBlockMap =
            std::map<block::pTxid, std::vector<block::pHash>>;
        using BlockTransactionMap =
            std::map<block::pHash, std::vector<block::pTxid>>;

        const api::Core& api_;
        mutable std::mutex lock_;
        mutable PatternMap patterns_;
        mutable SubchainPatternIndex subchain_pattern_index_;
        mutable SubchainIndexMap subchain_last_indexed_;
        mutable VersionIndex subchain_version_;
        mutable PositionMap subchain_last_scanned_;
        mutable PositionMap subchain_last_processed_;
        mutable MatchIndex match_index_;
        mutable OutputMap outputs_;
        mutable OutputStateMap unconfirmed_new_;
        mutable OutputStateMap confirmed_new_;
        mutable OutputStateMap unconfirmed_spend_;
        mutable OutputStateMap confirmed_spend_;
        mutable OutputStateMap orphaned_new_;
        mutable OutputStateMap orphaned_spend_;
        mutable TransactionMap transactions_;
        mutable TransactionBlockMap tx_to_block_;
        mutable BlockTransactionMap block_to_tx_;

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
                    for (const auto& [index, pattern] :
                         patterns_.at(patternID)) {
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
            const block::bitcoin::Transaction& transaction) const noexcept
            -> bool;
        auto change_state(
            const Lock& lock,
            const block::bitcoin::Outpoint& id,
            const block::Height originalHeight,
            const block::Height newHeight,
            OutputStateMap& to) const noexcept -> bool;
        auto find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
            const noexcept -> std::optional<OutputMap::iterator>;
        auto pattern_id(const SubchainID& subchain, const Bip32Index index)
            const noexcept -> pPatternID;
        auto remove_state(
            const Lock& lock,
            const block::bitcoin::Outpoint& id,
            const block::Height height,
            OutputStateMap& from) const noexcept -> bool;
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

    enum Table {
        Config = 0,
        BlockHeaderMetadata = 1,
        BlockHeaderBest = 2,
        ChainData = 3,
        BlockHeaderSiblings = 4,
        BlockHeaderDisconnected = 5,
        BlockFilterBest = 6,
        BlockFilterHeaderBest = 7,
    };

    enum class Key : std::size_t {
        Version = 0,
        TipHeight = 1,
        CheckpointHeight = 2,
        CheckpointHash = 3,
    };

    static const std::size_t db_version_;
    static const opentxs::storage::lmdb::TableNames table_names_;

    const blockchain::Type chain_;
    const Common& common_;
    opentxs::storage::lmdb::LMDB lmdb_;
    mutable Filters filters_;
    mutable Headers headers_;
    mutable Wallet wallet_;

    void init_db() noexcept;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
};
}  // namespace opentxs::blockchain::implementation
