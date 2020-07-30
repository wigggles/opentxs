// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/client/blockchain/database/Database.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "api/client/blockchain/database/BlockFilter.hpp"
#include "api/client/blockchain/database/BlockHeaders.hpp"
#include "api/client/blockchain/database/Blocks.hpp"
#include "api/client/blockchain/database/Peers.hpp"
#include "api/client/blockchain/database/Wallet.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"

// #define OT_METHOD
// "opentxs::api::client::blockchain::database::implementation::Database::"

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

const opentxs::storage::lmdb::TableNames Database::table_names_{
    {BlockHeaders, "block_headers"},
    {PeerDetails, "peers"},
    {PeerChainIndex, "peer_chain_index"},
    {PeerProtocolIndex, "peer_protocol_index"},
    {PeerServiceIndex, "peer_service_index"},
    {PeerNetworkIndex, "peer_network_index"},
    {PeerConnectedIndex, "peer_connected_index"},
    {FiltersBasic, "block_filters_basic"},
    {FiltersBCH, "block_filters_bch"},
    {FiltersOpentxs, "block_filters_opentxs"},
    {FilterHeadersBasic, "block_filter_headers_basic"},
    {FilterHeadersBCH, "block_filter_headers_bch"},
    {FilterHeadersOpentxs, "block_filter_headers_opentxs"},
    {Config, "config"},
    {BlockIndex, "blocks"},
    {Enabled, "enabled_chains"},
};

Database::Database(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args) noexcept(false)
    : api_(api)
    , legacy_(legacy)
    , blockchain_path_(init_storage_path(legacy, dataFolder))
    , common_path_(
          init_folder(legacy, blockchain_path_, String::Factory("common")))
#if OPENTXS_BLOCK_STORAGE_ENABLED
    , blocks_path_(init_folder(legacy, common_path_, String::Factory("blocks")))
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    , lmdb_(
          table_names_,
          common_path_->Get(),
          {
              {BlockHeaders, 0},
              {PeerDetails, 0},
              {PeerChainIndex, MDB_DUPSORT | MDB_INTEGERKEY},
              {PeerProtocolIndex, MDB_DUPSORT | MDB_INTEGERKEY},
              {PeerServiceIndex, MDB_DUPSORT | MDB_INTEGERKEY},
              {PeerNetworkIndex, MDB_DUPSORT | MDB_INTEGERKEY},
              {PeerConnectedIndex, MDB_DUPSORT | MDB_INTEGERKEY},
              {FiltersBasic, 0},
              {FiltersBCH, 0},
              {FiltersOpentxs, 0},
              {FilterHeadersBasic, 0},
              {FilterHeadersBCH, 0},
              {FilterHeadersOpentxs, 0},
              {Config, MDB_INTEGERKEY},
              {BlockIndex, 0},
              {Enabled, MDB_INTEGERKEY},
          })
    , block_policy_(block_storage_level(args, lmdb_))
    , siphash_key_(siphash_key(lmdb_))
    , headers_(std::make_unique<BlockHeader>(api_, lmdb_))
    , peers_(std::make_unique<Peers>(api_, lmdb_))
    , filters_(std::make_unique<BlockFilter>(api_, lmdb_))
#if OPENTXS_BLOCK_STORAGE_ENABLED
    , blocks_(std::make_unique<Blocks>(lmdb_, blocks_path_->Get()))
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    , wallet_(std::make_unique<Wallet>(blockchain, lmdb_))
{
    OT_ASSERT(crypto_shorthash_KEYBYTES == siphash_key_.size());

    static_assert(
        sizeof(opentxs::blockchain::PatternID) == crypto_shorthash_BYTES);

    OT_ASSERT(headers_)
    OT_ASSERT(peers_)
    OT_ASSERT(filters_)
#if OPENTXS_BLOCK_STORAGE_ENABLED
    OT_ASSERT(blocks_)
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    OT_ASSERT(wallet_)
}

auto Database::AddOrUpdate(Address_p address) const noexcept -> bool
{
    return peers_->Insert(std::move(address));
}

auto Database::AllocateStorageFolder(const std::string& dir) const noexcept
    -> std::string
{
    return init_folder(legacy_, blockchain_path_, String::Factory(dir))->Get();
}

auto Database::AssociateTransaction(
    const Txid& txid,
    const std::vector<PatternID>& patterns) const noexcept -> bool
{
    return wallet_->AssociateTransaction(txid, patterns);
}

auto Database::BlockHeaderExists(const BlockHash& hash) const noexcept -> bool
{
    return headers_->BlockHeaderExists(hash);
}

auto Database::block_storage_enabled() noexcept -> bool
{
    return 1 == OPENTXS_BLOCK_STORAGE_ENABLED;
}

auto Database::block_storage_level(
    const ArgList& args,
    opentxs::storage::lmdb::LMDB& lmdb) noexcept -> BlockStorage
{
    if (false == block_storage_enabled()) { return BlockStorage::None; }

    auto output = block_storage_level_default();
    const auto arg = block_storage_level_arg(args);

    if (arg.has_value()) { output = arg.value(); }

    const auto db = block_storage_level_configured(lmdb);

    if (db.has_value()) { output = std::max(output, db.value()); }

    if ((false == db.has_value()) || (output != db.value())) {
        lmdb.Store(Config, tsv(Key::BlockStoragePolicy), tsv(output));
    }

    return output;
}

auto Database::block_storage_level_arg(const ArgList& args) noexcept
    -> std::optional<BlockStorage>
{
    try {
        const auto& arg = args.at(OPENTXS_ARG_BLOCK_STORAGE_LEVEL);

        if (0 == arg.size()) { return BlockStorage::None; }

        switch (std::stoi(*arg.cbegin())) {
            case 2: {
                return BlockStorage::All;
            }
            case 1: {
                return BlockStorage::Cache;
            }
            default: {
                return BlockStorage::None;
            }
        }
    } catch (...) {
        return {};
    }
}

auto Database::block_storage_level_configured(
    opentxs::storage::lmdb::LMDB& db) noexcept -> std::optional<BlockStorage>
{
    if (false == db.Exists(Config, tsv(Key::BlockStoragePolicy))) {
        return std::nullopt;
    }

    auto output{BlockStorage::None};
    auto cb = [&output](const auto in) {
        if (sizeof(output) != in.size()) { return; }

        std::memcpy(&output, in.data(), in.size());
    };

    if (false == db.Load(Config, tsv(Key::BlockStoragePolicy), cb)) {
        return std::nullopt;
    }

    return output;
}

auto Database::block_storage_level_default() noexcept -> BlockStorage
{
    if (2 == OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY) {

        return BlockStorage::All;
    } else if (1 == OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY) {

        return BlockStorage::Cache;
    } else {

        return BlockStorage::None;
    }
}

auto Database::BlockExists(const BlockHash& block) const noexcept -> bool
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return blocks_->Exists(block);
#else
    return false;
#endif
}

auto Database::BlockLoad(const BlockHash& block) const noexcept -> BlockReader
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return blocks_->Load(block);
#else
    return BlockReader{};
#endif
}

auto Database::BlockStore(const BlockHash& block, const std::size_t bytes)
    const noexcept -> BlockWriter
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return blocks_->Store(block, bytes);
#else
    return {};
#endif
}

auto Database::Disable(const Chain type) const noexcept -> bool
{
    static const auto data{false};
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};

    return lmdb_.Store(Enabled, key, tsv(data)).first;
}

auto Database::Enable(const Chain type) const noexcept -> bool
{
    static const auto data{true};
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};

    return lmdb_.Store(Enabled, key, tsv(data)).first;
}

auto Database::Find(
    const Chain chain,
    const Protocol protocol,
    const std::set<Type> onNetworks,
    const std::set<Service> withServices) const noexcept -> Address_p
{
    return peers_->Find(chain, protocol, onNetworks, withServices);
}

auto Database::HaveFilter(const FilterType type, const ReadView blockHash)
    const noexcept -> bool
{
    return filters_->HaveFilter(type, blockHash);
}

auto Database::HaveFilterHeader(const FilterType type, const ReadView blockHash)
    const noexcept -> bool
{
    return filters_->HaveFilterHeader(type, blockHash);
}

auto Database::Import(std::vector<Address_p> peers) const noexcept -> bool
{
    return peers_->Import(std::move(peers));
}

auto Database::init_folder(
    const api::Legacy& legacy,
    const String& parent,
    const String& child) noexcept(false) -> OTString
{
    auto output = String::Factory();

    if (false == legacy.AppendFolder(output, parent, child)) {
        throw std::runtime_error("Failed to calculate path");
    }

    if (false == legacy.BuildFolderPath(output)) {
        throw std::runtime_error("Failed to construct path");
    }

    return output;
}

auto Database::init_storage_path(
    const api::Legacy& legacy,
    const std::string& dataFolder) noexcept(false) -> OTString
{
    return init_folder(
        legacy, String::Factory(dataFolder), String::Factory("blockchain"));
}

auto Database::LoadBlockHeader(const BlockHash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    return headers_->LoadBlockHeader(hash);
}

auto Database::LoadFilter(const FilterType type, const ReadView blockHash)
    const noexcept -> std::unique_ptr<const opentxs::blockchain::internal::GCS>
{
    return filters_->LoadFilter(type, blockHash);
}

auto Database::LoadFilterHash(
    const FilterType type,
    const ReadView blockHash,
    const AllocateOutput filterHash) const noexcept -> bool
{
    return filters_->LoadFilterHash(type, blockHash, filterHash);
}

auto Database::LoadFilterHeader(
    const FilterType type,
    const ReadView blockHash,
    const AllocateOutput header) const noexcept -> bool
{
    return filters_->LoadFilterHeader(type, blockHash, header);
}

auto Database::LoadTransaction(const ReadView txid) const noexcept
    -> std::optional<proto::BlockchainTransaction>
{
    return wallet_->LoadTransaction(txid);
}

auto Database::LookupContact(const Data& pubkeyHash) const noexcept
    -> std::set<OTIdentifier>
{
    return wallet_->LookupContact(pubkeyHash);
}

auto Database::LookupTransactions(const PatternID pattern) const noexcept
    -> std::vector<pTxid>
{
    return wallet_->LookupTransactions(pattern);
}

auto Database::LoadEnabledChains() const noexcept -> std::vector<Chain>
{
    auto output = std::vector<Chain>{};
    const auto cb = [&](const auto key, const auto value) -> bool {
        auto chain = Chain{};
        auto enabled{false};
        std::memcpy(
            static_cast<void*>(&chain),
            key.data(),
            std::min(key.size(), sizeof(chain)));
        std::memcpy(
            static_cast<void*>(&enabled),
            value.data(),
            std::min(value.size(), sizeof(enabled)));

        if (enabled) { output.emplace_back(chain); }

        return true;
    };
    lmdb_.Read(Enabled, cb, opentxs::storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Database::siphash_key(opentxs::storage::lmdb::LMDB& db) noexcept
    -> SiphashKey
{
    auto configured = siphash_key_configured(db);

    if (configured.has_value()) { return configured.value(); }

    auto output = space(crypto_shorthash_KEYBYTES);
    ::crypto_shorthash_keygen(reinterpret_cast<unsigned char*>(output.data()));
    const auto saved = db.Store(Config, tsv(Key::SiphashKey), reader(output));

    OT_ASSERT(saved.first);

    return output;
}

auto Database::siphash_key_configured(opentxs::storage::lmdb::LMDB& db) noexcept
    -> std::optional<SiphashKey>
{
    if (false == db.Exists(Config, tsv(Key::SiphashKey))) {
        return std::nullopt;
    }

    auto output = space(crypto_shorthash_KEYBYTES);
    auto cb = [&output](const auto in) {
        if (output.size() != in.size()) { return; }

        std::memcpy(output.data(), in.data(), in.size());
    };

    if (false == db.Load(Config, tsv(Key::SiphashKey), cb)) {
        return std::nullopt;
    }

    return std::move(output);
}

auto Database::StoreBlockHeader(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    return headers_->StoreBlockHeader(header);
}

auto Database::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    return headers_->StoreBlockHeaders(headers);
}

auto Database::StoreFilterHeaders(
    const FilterType type,
    const std::vector<FilterHeader>& headers) const noexcept -> bool
{
    return filters_->StoreFilterHeaders(type, headers);
}

auto Database::StoreFilters(
    const FilterType type,
    std::vector<FilterData>& filters) const noexcept -> bool
{
    return filters_->StoreFilters(type, filters);
}

auto Database::StoreFilters(
    const FilterType type,
    const std::vector<FilterHeader>& headers,
    const std::vector<FilterData>& filters) const noexcept -> bool
{
    return filters_->StoreFilters(type, headers, filters);
}

auto Database::StoreTransaction(
    const proto::BlockchainTransaction& tx) const noexcept -> bool
{
    return wallet_->StoreTransaction(tx);
}

auto Database::UpdateContact(const Contact& contact) const noexcept
    -> std::vector<pTxid>
{
    return wallet_->UpdateContact(contact);
}

auto Database::UpdateMergedContact(const Contact& parent, const Contact& child)
    const noexcept -> std::vector<pTxid>
{
    return wallet_->UpdateMergedContact(parent, child);
}

Database::~Database() = default;
}  // namespace opentxs::api::client::blockchain::database::implementation
