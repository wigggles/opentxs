// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class BlockFilter;
class BlockHeader;
class Blocks;
class Peers;
class Wallet;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain

class Blockchain;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin

class Header;
}  // namespace block

namespace client
{
struct GCS;
}  // namespace client
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

class Contact;
class Data;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::database::implementation
{
class Database final
{
public:
    enum class Key : std::size_t {
        BlockStoragePolicy = 0,
        NextBlockAddress = 1,
        SiphashKey = 2,
    };

    using BlockHash = opentxs::blockchain::block::Hash;
    using PatternID = opentxs::blockchain::PatternID;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using Chain = opentxs::blockchain::Type;

    auto AddOrUpdate(Address_p address) const noexcept -> bool;
    auto AllocateStorageFolder(const std::string& dir) const noexcept
        -> std::string;
    auto AssociateTransaction(
        const Txid& txid,
        const std::vector<PatternID>& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool;
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> BlockReader;
    auto BlockPolicy() const noexcept -> BlockStorage { return block_policy_; }
    auto BlockStore(const BlockHash& block, const std::size_t bytes)
        const noexcept -> BlockWriter;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type) const noexcept -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address_p;
    auto HashKey() const noexcept { return reader(siphash_key_); }
    auto HaveFilter(const FilterType type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const FilterType type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(std::vector<Address_p> peers) const noexcept -> bool;
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> std::vector<Chain>;
    auto LoadFilter(const FilterType type, const ReadView blockHash) const
        noexcept -> std::unique_ptr<const opentxs::blockchain::client::GCS>;
    auto LoadFilterHash(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::optional<proto::BlockchainTransaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> std::vector<pTxid>;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const FilterType type,
        const std::vector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(const FilterType type, std::vector<FilterData>& filters)
        const noexcept -> bool;
    auto StoreFilters(
        const FilterType type,
        const std::vector<FilterHeader>& headers,
        const std::vector<FilterData>& filters) const noexcept -> bool;
    auto StoreTransaction(const proto::BlockchainTransaction& tx) const noexcept
        -> bool;
    auto UpdateContact(const Contact& contact) const noexcept
        -> std::vector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> std::vector<pTxid>;

    Database(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args) noexcept(false);

    ~Database();

private:
    using SiphashKey = Space;

    static const opentxs::storage::lmdb::TableNames table_names_;

    const api::Core& api_;
    const api::Legacy& legacy_;
    const OTString blockchain_path_;
    const OTString common_path_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    const OTString blocks_path_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    opentxs::storage::lmdb::LMDB lmdb_;
    const BlockStorage block_policy_;
    const SiphashKey siphash_key_;
    const std::unique_ptr<BlockHeader> headers_;
    const std::unique_ptr<Peers> peers_;
    const std::unique_ptr<BlockFilter> filters_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    const std::unique_ptr<Blocks> blocks_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    const std::unique_ptr<Wallet> wallet_;

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
    static auto siphash_key(opentxs::storage::lmdb::LMDB& db) noexcept
        -> SiphashKey;
    static auto siphash_key_configured(
        opentxs::storage::lmdb::LMDB& db) noexcept -> std::optional<SiphashKey>;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database &&) -> Database& = delete;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
