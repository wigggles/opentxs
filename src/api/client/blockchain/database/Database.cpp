// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/Proto.tpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/block/Block.hpp"

#include <optional>

#include "Database.hpp"

namespace opentxs::api::client::blockchain::database::implementation
{
const opentxs::storage::lmdb::TableNames Database::table_names_{
    {BlockHeaders, "block_headers"},
};

Database::Database(
    const api::internal::Core& api,
    const api::Legacy& legacy,
    const std::string& dataFolder) noexcept(false)
    : api_(api)
    , blockchain_path_(init_storage_path(legacy, dataFolder))
    , common_path_(
          init_folder(legacy, blockchain_path_, String::Factory("common")))
    , lmdb_(table_names_, common_path_->Get(), {{BlockHeaders, 0}})
{
}

auto Database::AllocateStorageFolder(const std::string& dir) const noexcept
    -> std::string
{
    return init_folder(api_.Legacy(), blockchain_path_, String::Factory(dir))
        ->Get();
}

auto Database::BlockHeaderExists(
    const opentxs::blockchain::block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Exists(
        Table::BlockHeaders, api_.Crypto().Encode().IdentifierEncode(hash));
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

auto Database::LoadBlockHeader(const opentxs::blockchain::block::Hash& hash)
    const noexcept(false) -> proto::BlockchainBlockHeader
{
    auto output = std::optional<proto::BlockchainBlockHeader>{};
    lmdb_.Load(
        Table::BlockHeaders,
        api_.Crypto().Encode().IdentifierEncode(hash),
        [&](const auto data) -> void {
            output = proto::Factory<proto::BlockchainBlockHeader>(
                data.data(), data.size());
        });

    if (false == output.has_value()) {
        throw std::out_of_range("Block header not found");
    }

    return output.value();
}

auto Database::StoreBlockHeader(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    auto serialized = header.Serialize();
    serialized.clear_local();
    const auto result = lmdb_.Store(
        Table::BlockHeaders,
        api_.Crypto().Encode().IdentifierEncode(header.Hash()),
        proto::ToString(header.Serialize()),
        nullptr,
        MDB_NOOVERWRITE);

    if (false == result.first) {
        if (MDB_KEYEXIST != result.second) { return false; }
    }

    return true;
}

auto Database::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (const auto& [hash, pair] : headers) {
        const auto& [header, newBlock] = pair;

        if (newBlock) {
            auto serialized = header->Serialize();
            serialized.clear_local();
            const auto stored = lmdb_.Store(
                Table::BlockHeaders,
                api_.Crypto().Encode().IdentifierEncode(header->Hash()),
                proto::ToString(serialized),
                parentTxn,
                MDB_NOOVERWRITE);

            if (false == stored.first) {
                if (MDB_KEYEXIST != stored.second) { return false; }
            }
        }
    }

    return parentTxn.Finalize(true);
}
}  // namespace opentxs::api::client::blockchain::database::implementation
