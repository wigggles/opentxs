// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/String.hpp"
#include "opentxs/Bytes.hpp"

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "util/LMDB.hpp"
#include "BlockFilter.hpp"
#include "BlockHeaders.hpp"
#include "Blocks.hpp"
#include "Peers.hpp"

namespace opentxs::api::client::blockchain::database::implementation
{
class Database final
{
public:
    enum class Key : std::size_t {
        BlockStoragePolicy = 0,
        NextBlockAddress = 1,
    };

    using BlockHash = opentxs::blockchain::block::Hash;

    auto AddOrUpdate(Address_p address) const noexcept -> bool
    {
        return peers_.Insert(std::move(address));
    }
    auto AllocateStorageFolder(const std::string& dir) const noexcept
        -> std::string;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool
    {
        return headers_.BlockHeaderExists(hash);
    }
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> ReadView;
    auto BlockPolicy() const noexcept -> BlockStorage { return block_policy_; }
    auto BlockStore(const BlockHash& block, const ReadView bytes) const noexcept
        -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address_p
    {
        return peers_.Find(chain, protocol, onNetworks, withServices);
    }
    auto HaveFilter(const FilterType type, const ReadView blockHash) const
        noexcept -> bool
    {
        return filters_.HaveFilter(type, blockHash);
    }
    auto HaveFilterHeader(const FilterType type, const ReadView blockHash) const
        noexcept -> bool
    {
        return filters_.HaveFilterHeader(type, blockHash);
    }
    auto Import(std::vector<Address_p> peers) const noexcept -> bool
    {
        return peers_.Import(std::move(peers));
    }
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader
    {
        return headers_.LoadBlockHeader(hash);
    }
    auto LoadFilter(const FilterType type, const ReadView blockHash) const
        noexcept -> std::unique_ptr<const opentxs::blockchain::internal::GCS>
    {
        return filters_.LoadFilter(type, blockHash);
    }
    auto LoadFilterHash(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool
    {
        return filters_.LoadFilterHash(type, blockHash, filterHash);
    }
    auto LoadFilterHeader(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool
    {
        return filters_.LoadFilterHeader(type, blockHash, header);
    }
    auto StoreBlockHeader(
        const opentxs::blockchain::block::Header& header) const noexcept -> bool
    {
        return headers_.StoreBlockHeader(header);
    }
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool
    {
        return headers_.StoreBlockHeaders(headers);
    }
    auto StoreFilterHeaders(
        const FilterType type,
        const std::vector<FilterHeader>& headers) const noexcept -> bool
    {
        return filters_.StoreFilterHeaders(type, headers);
    }
    auto StoreFilters(const FilterType type, std::vector<FilterData>& filters)
        const noexcept -> bool
    {
        return filters_.StoreFilters(type, filters);
    }

    Database(
        const api::internal::Core& api,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args) noexcept(false);

private:
    static const opentxs::storage::lmdb::TableNames table_names_;

    const api::internal::Core& api_;
    const OTString blockchain_path_;
    const OTString common_path_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    const OTString blocks_path_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    opentxs::storage::lmdb::LMDB lmdb_;
    const BlockStorage block_policy_;
    mutable BlockHeader headers_;
    mutable Peers peers_;
    mutable BlockFilter filters_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    mutable Blocks blocks_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED

    static auto block_storage_enabled() noexcept -> bool;
    static auto block_storage_level(
        const ArgList& args,
        opentxs::storage::lmdb::LMDB& db) noexcept -> BlockStorage;
    static auto block_storage_level_arg(const ArgList& args) noexcept
        -> std::optional<BlockStorage>;
    static auto block_storage_level_configured(
        opentxs::storage::lmdb::LMDB& db) noexcept
        -> std::optional<BlockStorage>;
    static auto block_storage_level_default() noexcept -> BlockStorage;
    static auto init_folder(
        const api::Legacy& legacy,
        const String& parent,
        const String& child) noexcept(false) -> OTString;
    static auto init_storage_path(
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false) -> OTString;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
