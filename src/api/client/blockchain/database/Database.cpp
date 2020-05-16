// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/client/blockchain/database/Database.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "api/client/blockchain/database/Blocks.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Legacy.hpp"

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
};

Database::Database(
    const api::internal::Core& api,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args) noexcept(false)
    : api_(api)
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
          })
    , block_policy_(block_storage_level(args, lmdb_))
    , headers_(api, lmdb_)
    , peers_(api, lmdb_)
    , filters_(api, lmdb_)
#if OPENTXS_BLOCK_STORAGE_ENABLED
    , blocks_(lmdb_, blocks_path_->Get())
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
{
}

auto Database::AllocateStorageFolder(const std::string& dir) const noexcept
    -> std::string
{
    return init_folder(api_.Legacy(), blockchain_path_, String::Factory(dir))
        ->Get();
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
    if (false == db.Exists(Config, tsv(Key::BlockStoragePolicy))) { return {}; }

    auto output{BlockStorage::None};
    auto cb = [&output](const auto in) {
        if (sizeof(output) != in.size()) { return; }

        std::memcpy(&output, in.data(), in.size());
    };

    if (false == db.Load(Config, tsv(Key::BlockStoragePolicy), cb)) {
        return {};
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
    return blocks_.Exists(block);
#else
    return false;
#endif
}

auto Database::BlockLoad(const BlockHash& block) const noexcept -> BlockReader
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return blocks_.Load(block);
#else
    return BlockReader{};
#endif
}

auto Database::BlockStore(const BlockHash& block, const std::size_t bytes) const
    noexcept -> BlockWriter
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return blocks_.Store(block, bytes);
#else
    return {};
#endif
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
}  // namespace opentxs::api::client::blockchain::database::implementation
