// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Legacy.hpp"

#include "internal/api/Api.hpp"

#include "Database.hpp"

// #define OT_METHOD
// "opentxs::api::client::blockchain::database::implementation::Database::"

namespace opentxs::api::client::blockchain::database::implementation
{
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
};

Database::Database(
    const api::internal::Core& api,
    const api::Legacy& legacy,
    const std::string& dataFolder) noexcept(false)
    : api_(api)
    , blockchain_path_(init_storage_path(legacy, dataFolder))
    , common_path_(
          init_folder(legacy, blockchain_path_, String::Factory("common")))
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
          })
    , headers_(api, lmdb_)
    , peers_(api, lmdb_)
    , filters_(api, lmdb_)
{
}

auto Database::AllocateStorageFolder(const std::string& dir) const noexcept
    -> std::string
{
    return init_folder(api_.Legacy(), blockchain_path_, String::Factory(dir))
        ->Get();
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
