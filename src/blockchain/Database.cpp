// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Proto.tpp"

#include "api/client/blockchain/database/Database.hpp"
#include "blockchain/client/UpdateTransaction.hpp"
#include "internal/api/Api.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "util/LMDB.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>

#include "Database.hpp"

#define OT_METHOD "opentxs::blockchain::implementation::Database::"

namespace opentxs
{
blockchain::internal::Database* Factory::BlockchainDatabase(
    const api::internal::Core& api,
    const blockchain::client::internal::Network& network,
    const api::client::blockchain::database::implementation::Database& common,
    const blockchain::Type type)
{
    using ReturnType = blockchain::implementation::Database;

    return new ReturnType(api, network, common, type);
}
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
template <typename Input>
ReadView tsv(const Input& in) noexcept
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

const std::size_t Database::db_version_{1};
const opentxs::storage::lmdb::TableNames Database::table_names_{
    {Config, "config"},
    {BlockHeaderMetadata, "block_header_metadata"},
    {BlockHeaderBest, "best_header_chain"},
    {ChainData, "block_header_data"},
    {BlockHeaderSiblings, "block_siblings"},
    {BlockHeaderDisconnected, "disconnected_block_headers"},
};

const std::map<
    blockchain::Type,
    std::map<filter::Type, std::pair<std::string, std::string>>>
    Database::Filters::genesis_filters_{
        {blockchain::Type::Bitcoin,
         {
             {filter::Type::Basic,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
         }},
        {blockchain::Type::BitcoinCash,
         {
             {filter::Type::Basic,
              {"9f3c30f0c37fb977cf3e1a3173c631e8ff119ad3088b6f5b2bced0802139c20"
               "2",
               "017fa880"}},
         }},
        {blockchain::Type::Bitcoin_testnet3,
         {
             {filter::Type::Basic,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
         }},
        {blockchain::Type::BitcoinCash_testnet3,
         {
             {filter::Type::Basic,
              {"50b781aed7b7129012a6d20e2d040027937f3affaee573779908ebb77945582"
               "1",
               "019dfca8"}},
         }},
    };

Database::Database(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const Common& common,
    const blockchain::Type type) noexcept
    : chain_(type)
    , common_(common)
    , lmdb_(
          table_names_,
          common.AllocateStorageFolder(
              std::to_string(static_cast<std::uint32_t>(type))),
          {{Config, MDB_INTEGERKEY},
           {BlockHeaderMetadata, 0},
           {BlockHeaderBest, MDB_INTEGERKEY},
           {ChainData, MDB_INTEGERKEY},
           {BlockHeaderSiblings, 0},
           {BlockHeaderDisconnected, MDB_DUPSORT}},
          0)
    , filters_(api, type)
    , headers_(api, network, common_, lmdb_, type)
{
    init_db();
}

Database::Filters::Filters(
    const api::internal::Core& api,
    const blockchain::Type chain) noexcept
    : api_(api)
    , lock_()
    , tip_()
    , header_tip_()
    , filters_()
    , headers_()
{
    const auto pBlock = opentxs::Factory::GenesisBlockHeader(api_, chain);

    OT_ASSERT(pBlock);

    const auto& block = *pBlock;
    const auto& hash = block.Hash();
    const auto& genesis = genesis_filters_.at(chain).at(filter::Type::Basic);
    auto header = api_.Factory().Data(genesis.first, StringStyle::Hex);
    const auto bytes = api_.Factory().Data(genesis.second, StringStyle::Hex);
    auto gcs = std::unique_ptr<blockchain::internal::GCS>{Factory::GCS(
        api_,
        19,
        784931,
        blockchain::internal::BlockHashToFilterKey(hash.Bytes()),
        1,
        bytes)};

    OT_ASSERT(gcs);

    filters_[filter::Type::Basic].emplace(hash, std::move(gcs));
    headers_[filter::Type::Basic].emplace(
        hash, HeaderData{std::move(header), api_.Factory().Data()});
    tip_.emplace(filter::Type::Basic, block.Position());
    header_tip_.emplace(filter::Type::Basic, block.Position());
}

Database::Headers::Headers(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const Common& common,
    const opentxs::storage::lmdb::LMDB& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , network_(network)
    , common_(common)
    , lmdb_(lmdb)
    , lock_()
{
    import_genesis(type);

    OT_ASSERT(HeaderExists(best().second));
}

auto Database::Filters::CurrentHeaderTip(const filter::Type type) const noexcept
    -> block::Position
{
    Lock lock(lock_);

    try {

        return header_tip_.at(type);
    } catch (...) {

        return make_blank<block::Position>::value(api_);
    }
}

auto Database::Filters::CurrentTip(const filter::Type type) const noexcept
    -> block::Position
{
    Lock lock(lock_);

    try {

        return tip_.at(type);
    } catch (...) {

        return make_blank<block::Position>::value(api_);
    }
}

auto Database::Filters::HaveFilter(
    const filter::Type type,
    const block::Hash& block) const noexcept -> bool
{
    Lock lock(lock_);

    try {

        return 0 < filters_.at(type).count(block);
    } catch (...) {

        return false;
    }
}

auto Database::Filters::HaveFilterHeader(
    const filter::Type type,
    const block::Hash& block) const noexcept -> bool
{
    Lock lock(lock_);

    try {

        return 0 < headers_.at(type).count(block);
    } catch (...) {

        return false;
    }
}

auto Database::Filters::LoadFilterHash(
    const filter::Type type,
    const ReadView block) const noexcept -> Hash
{
    const auto hash = api_.Factory().Data(block);
    Lock lock(lock_);

    try {

        return headers_.at(type).at(hash).second;
    } catch (...) {

        return api_.Factory().Data();
    }
}

auto Database::Filters::LoadFilterHeader(
    const filter::Type type,
    const ReadView block) const noexcept -> Hash
{
    const auto hash = api_.Factory().Data(block);
    Lock lock(lock_);

    try {

        return headers_.at(type).at(hash).first;
    } catch (...) {

        return api_.Factory().Data();
    }
}

auto Database::Filters::SetHeaderTip(
    const filter::Type type,
    const block::Position position) const noexcept -> bool
{
    Lock lock(lock_);
    auto it = header_tip_.find(type);

    if (header_tip_.end() == it) {
        header_tip_.emplace(type, position);
    } else {
        it->second = position;
    }

    return true;
}

auto Database::Filters::SetTip(
    const filter::Type type,
    const block::Position position) const noexcept -> bool
{
    Lock lock(lock_);
    auto it = tip_.find(type);

    if (tip_.end() == it) {
        tip_.emplace(type, position);
    } else {
        it->second = position;
    }

    return true;
}

auto Database::Filters::StoreHeaders(
    const filter::Type type,
    const ReadView previous,
    const std::vector<Header> headers) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = headers_[type];

    for (const auto& [block, header, hash] : headers) {
        map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::move(block)),
            std::forward_as_tuple(
                std::move(header), api_.Factory().Data(hash)));
    }

    return true;
}

auto Database::Filters::StoreFilters(
    const filter::Type type,
    std::vector<Filter> filters) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = filters_[type];

    for (auto& [block, filter] : filters) {
        if (false == bool(filter)) { return false; }

        map.emplace(api_.Factory().Data(block), std::move(filter));
    }

    return true;
}

auto Database::Headers::ApplyUpdate(
    const client::UpdateTransaction& update) noexcept -> bool
{
    if (false == common_.StoreBlockHeaders(update.UpdatedHeaders())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save block headers")
            .Flush();

        return false;
    }

    Lock lock(lock_);
    const auto initialHeight = best(lock).first;
    auto parentTxn = lmdb_.TransactionRW();

    if (update.HaveCheckpoint()) {
        if (false ==
            lmdb_
                .Store(
                    ChainData,
                    tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
                    tsv(static_cast<std::size_t>(update.Checkpoint().first)),
                    parentTxn)
                .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to save checkpoint height")
                .Flush();

            return false;
        }

        if (false == lmdb_
                         .Store(
                             ChainData,
                             tsv(static_cast<std::size_t>(Key::CheckpointHash)),
                             update.Checkpoint().second->Bytes(),
                             parentTxn)
                         .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to save checkpoint hash")
                .Flush();

            return false;
        }
    }

    for (const auto& [parent, child] : update.Disconnected()) {
        if (false == lmdb_
                         .Store(
                             BlockHeaderDisconnected,
                             parent->Bytes(),
                             child->Bytes(),
                             parentTxn)
                         .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to save disconnected hash")
                .Flush();

            return false;
        }
    }

    for (const auto& [parent, child] : update.Connected()) {
        if (false == lmdb_.Delete(
                         BlockHeaderDisconnected,
                         parent->Bytes(),
                         child->Bytes(),
                         parentTxn)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to delete disconnected hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToAdd()) {
        if (false == lmdb_
                         .Store(
                             BlockHeaderSiblings,
                             hash->Bytes(),
                             hash->Bytes(),
                             parentTxn)
                         .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save sibling hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToDelete()) {
        lmdb_.Delete(BlockHeaderSiblings, hash->Bytes(), parentTxn);
    }

    for (const auto& [hash, pair] : update.UpdatedHeaders()) {
        const auto& [header, newBlock] = pair;
        const auto result = lmdb_.Store(
            BlockHeaderMetadata,
            hash->Bytes(),
            proto::ToString(header->Serialize().local()),
            parentTxn);

        if (false == result.first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to save block metadata")
                .Flush();

            return false;
        }
    }

    if (update.HaveReorg()) {
        for (auto i = initialHeight; i > update.ReorgParent().first; --i) {
            if (false == pop_best(i, parentTxn)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to delete best hash")
                    .Flush();

                return false;
            }
        }
    }

    for (const auto& position : update.BestChain()) {
        push_best(position, false, parentTxn);
    }

    if (0 < update.BestChain().size()) {
        const auto& tip = *update.BestChain().crbegin();

        if (false == lmdb_
                         .Store(
                             ChainData,
                             tsv(static_cast<std::size_t>(Key::TipHeight)),
                             tsv(static_cast<std::size_t>(tip.first)),
                             parentTxn)
                         .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to store best hash")
                .Flush();

            return false;
        }
    }

    parentTxn.Finalize(true);

    if (update.HaveReorg()) {
        LogVerbose("Blockchain reorg detected. Last common ancestor is ")(
            update.ReorgParent().second->asHex())(" at height ")(
            update.ReorgParent().first)
            .Flush();

        // TODO broadcast reorg signal
    }

    network_.UpdateLocalHeight(best(lock));

    return true;
}

auto Database::Headers::BestBlock(const block::Height position) const
    noexcept(false) -> block::pHash
{
    auto output = Data::Factory();

    if (0 > position) { return output; }

    lmdb_.Load(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(position)),
        [&](const auto in) -> void { output->Assign(in.data(), in.size()); });

    return output;
}

auto Database::Headers::best() const noexcept -> block::Position
{
    Lock lock(lock_);

    return best(lock);
}

auto Database::Headers::best(const Lock& lock) const noexcept -> block::Position
{
    auto output = make_blank<block::Position>::value(api_);
    auto height = std::size_t{0};

    if (false ==
        lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {

        return make_blank<block::Position>::value(api_);
    }

    if (false ==
        lmdb_.Load(BlockHeaderBest, tsv(height), [&](const auto in) -> void {
            output.second->Assign(in.data(), in.size());
        })) {

        return make_blank<block::Position>::value(api_);
    }

    output.first = height;

    return output;
}

auto Database::Headers::checkpoint(const Lock& lock) const noexcept
    -> block::Position
{
    auto output = make_blank<block::Position>::value(api_);
    auto height = std::size_t{0};

    if (false ==
        lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {
        return make_blank<block::Position>::value(api_);
    }

    if (false == lmdb_.Load(
                     ChainData,
                     tsv(static_cast<std::size_t>(Key::CheckpointHash)),
                     [&](const auto in) -> void {
                         output.second->Assign(in.data(), in.size());
                     })) {

        return make_blank<block::Position>::value(api_);
    }

    output.first = height;

    return output;
}

auto Database::Headers::CurrentCheckpoint() const noexcept -> block::Position
{
    Lock lock(lock_);

    return checkpoint(lock);
}

auto Database::Headers::DisconnectedHashes() const noexcept
    -> client::DisconnectedList
{
    Lock lock(lock_);
    auto output = client::DisconnectedList{};
    lmdb_.Read(
        BlockHeaderDisconnected,
        [&](const auto key, const auto value) -> bool {
            output.emplace(
                Data::Factory(key.data(), key.size()),
                Data::Factory(value.data(), value.size()));

            return true;
        },
        opentxs::storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Database::Headers::HasDisconnectedChildren(const block::Hash& hash) const
    noexcept -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderDisconnected, hash.Bytes());
}

auto Database::Headers::HaveCheckpoint() const noexcept -> bool
{
    Lock lock(lock_);

    return 0 < checkpoint(lock).first;
}

auto Database::Headers::header_exists(const Lock& lock, const block::Hash& hash)
    const noexcept -> bool
{
    return common_.BlockHeaderExists(hash) &&
           lmdb_.Exists(BlockHeaderMetadata, hash.Bytes());
}

auto Database::Headers::HeaderExists(const block::Hash& hash) const noexcept
    -> bool
{
    Lock lock(lock_);

    return header_exists(lock, hash);
}

auto Database::Headers::import_genesis(const blockchain::Type type) const
    noexcept -> void
{
    auto success{false};
    const auto& hash = client::HeaderOracle::GenesisBlockHash(type);

    try {
        const auto serialized = common_.LoadBlockHeader(hash);

        if (false == lmdb_.Exists(BlockHeaderMetadata, hash.Bytes())) {
            auto genesis = api_.Factory().BlockHeader(serialized);

            OT_ASSERT(genesis);

            const auto result = lmdb_.Store(
                BlockHeaderMetadata,
                hash.Bytes(),
                proto::ToString(genesis->Serialize().local()));

            OT_ASSERT(result.first);
        }
    } catch (...) {
        auto genesis = std::unique_ptr<blockchain::block::Header>{
            opentxs::Factory::GenesisBlockHeader(api_, type)};

        OT_ASSERT(genesis);

        success = common_.StoreBlockHeader(*genesis);

        OT_ASSERT(success);

        success = lmdb_
                      .Store(
                          BlockHeaderMetadata,
                          hash.Bytes(),
                          proto::ToString(genesis->Serialize().local()))
                      .first;

        OT_ASSERT(success);
    }

    if (0 > best().first) {
        auto transaction = lmdb_.TransactionRW();
        success = push_best({0, hash}, true, transaction);

        OT_ASSERT(success);

        success = transaction.Finalize(true);

        OT_ASSERT(success);
    }
}

auto Database::Headers::IsSibling(const block::Hash& hash) const noexcept
    -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderSiblings, hash.Bytes());
}

auto Database::Headers::load_header(const block::Hash& hash) const
    -> std::unique_ptr<block::Header>
{
    auto proto = common_.LoadBlockHeader(hash);
    const auto haveMeta =
        lmdb_.Load(BlockHeaderMetadata, hash.Bytes(), [&](const auto data) {
            proto.mutable_local()->ParseFromArray(data.data(), data.size());
        });

    if (false == haveMeta) {
        throw std::out_of_range("Block header metadata not found");
    }

    auto output = api_.Factory().BlockHeader(proto);

    OT_ASSERT(output);

    return output;
}

auto Database::Headers::pop_best(const std::size_t i, MDB_txn* parent) const
    noexcept -> bool
{
    return lmdb_.Delete(BlockHeaderBest, tsv(i), parent);
}

auto Database::Headers::push_best(
    const block::Position next,
    const bool setTip,
    MDB_txn* parent) const noexcept -> bool
{
    OT_ASSERT(nullptr != parent);

    auto output = lmdb_.Store(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(next.first)),
        next.second->Bytes(),
        parent);

    if (output.first && setTip) {
        output = lmdb_.Store(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            tsv(static_cast<std::size_t>(next.first)),
            parent);
    }

    return output.first;
}

auto Database::Headers::RecentHashes() const noexcept
    -> std::vector<block::pHash>
{
    Lock lock(lock_);

    return recent_hashes(lock);
}

auto Database::Headers::recent_hashes(const Lock& lock) const noexcept
    -> std::vector<block::pHash>
{
    auto output = std::vector<block::pHash>{};
    lmdb_.Read(
        BlockHeaderBest,
        [&](const auto, const auto value) -> bool {
            output.emplace_back(Data::Factory(value.data(), value.size()));

            return 100 > output.size();
        },
        opentxs::storage::lmdb::LMDB::Dir::Backward);

    return output;
}

auto Database::Headers::SiblingHashes() const noexcept -> client::Hashes
{
    Lock lock(lock_);
    auto output = client::Hashes{};
    lmdb_.Read(
        BlockHeaderSiblings,
        [&](const auto, const auto value) -> bool {
            output.emplace(Data::Factory(value.data(), value.size()));

            return true;
        },
        opentxs::storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Database::Headers::TryLoadHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<block::Header>
{
    try {
        return LoadHeader(hash);
    } catch (...) {
        return {};
    }
}

auto Database::init_db() noexcept -> void
{
    if (false == lmdb_.Exists(Config, tsv(Key::Version))) {
        const auto stored =
            lmdb_.Store(Config, tsv(Key::Version), tsv(db_version_));

        OT_ASSERT(stored.first);
    }
}
}  // namespace opentxs::blockchain::implementation
