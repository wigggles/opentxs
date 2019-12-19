// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

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
std::string_view tsv(const Input& in) noexcept
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

Database::Database(
    const api::internal::Core& api,
    const client::internal::Network& network,
    const Common& common,
    const blockchain::Type type) noexcept
    : common_(common)
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
    , filters_(api)
    , headers_(api, network, common_, lmdb_, type)
    , peers_()
{
    init_db();
}

Database::Filters::Filters(const api::Core& api) noexcept
    : api_(api)
    , lock_()
    , tip_()
    , filters_()
{
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

Database::Peers::Peers() noexcept
    : lock_()
    , addresses_()
    , protocols_()
    , services_()
    , networks_()
    , last_connected_()
{
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

auto Database::Filters::StoreFilter(
    const filter::Type type,
    const block::Hash& block,
    std::unique_ptr<const blockchain::internal::GCS> filter) const noexcept
    -> bool
{
    Lock lock(lock_);
    auto& map = filters_[type];
    map.emplace(block, std::move(filter));

    return 0 < map.count(block);
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
                             update.Checkpoint().second.get(),
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
                             parent.get(),
                             child.get(),
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
                         parent.get(),
                         child.get(),
                         parentTxn)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to delete disconnected hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToAdd()) {
        if (false ==
            lmdb_.Store(BlockHeaderSiblings, hash.get(), hash.get(), parentTxn)
                .first) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to save sibling hash")
                .Flush();

            return false;
        }
    }

    for (const auto& hash : update.SiblingsToDelete()) {
        lmdb_.Delete(BlockHeaderSiblings, hash.get(), parentTxn);
    }

    for (const auto& [hash, pair] : update.UpdatedHeaders()) {
        const auto& [header, newBlock] = pair;
        const auto result = lmdb_.Store(
            BlockHeaderMetadata,
            hash.get(),
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

    return lmdb_.Exists(BlockHeaderDisconnected, hash);
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
           lmdb_.Exists(BlockHeaderMetadata, hash);
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

        if (false == lmdb_.Exists(BlockHeaderMetadata, hash)) {
            auto genesis = api_.Factory().BlockHeader(serialized);

            OT_ASSERT(genesis);

            const auto result = lmdb_.Store(
                BlockHeaderMetadata,
                hash,
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
                          hash,
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

    return lmdb_.Exists(BlockHeaderSiblings, hash);
}

auto Database::Headers::load_header(const block::Hash& hash) const
    -> std::unique_ptr<block::Header>
{
    auto proto = common_.LoadBlockHeader(hash);
    const auto haveMeta =
        lmdb_.Load(BlockHeaderMetadata, hash, [&](const auto data) {
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
        next.second.get(),
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

auto Database::Peers::Find(
    const Protocol protocol,
    const std::set<Type> onNetworks,
    const std::set<Service> withServices) const noexcept -> Database::Address
{
    Lock lock(lock_);

    try {
        std::set<OTIdentifier> candidates{};
        const auto& protocolSet = protocols_.at(protocol);

        if (protocolSet.empty()) { return {}; }

        for (const auto& network : onNetworks) {
            try {
                for (const auto& id : networks_.at(network)) {
                    if (1 == protocolSet.count(id)) { candidates.emplace(id); }
                }
            } catch (...) {
            }
        }

        if (candidates.empty()) { return {}; }

        std::set<OTIdentifier> haveServices{};

        if (withServices.empty()) {
            haveServices = candidates;
        } else {
            for (const auto& id : candidates) {
                bool haveAllServices{true};

                for (const auto& service : withServices) {
                    try {
                        if (0 == services_.at(service).count(id)) {
                            haveAllServices = false;
                            break;
                        }
                    } catch (...) {
                        haveAllServices = false;
                        break;
                    }
                }

                if (haveAllServices) { haveServices.emplace(id); }
            }
        }

        if (haveServices.empty()) { return {}; }

        std::vector<OTIdentifier> output;
        const std::size_t count{1};
        std::sample(
            haveServices.begin(),
            haveServices.end(),
            std::back_inserter(output),
            count,
            std::mt19937{std::random_device{}()});

        OT_ASSERT(count == output.size());

        return addresses_.at(output.front())->clone_internal();
    } catch (...) {
        return {};
    }
}

auto Database::Peers::Insert(Address pAddress) noexcept -> bool
{
    if (false == bool(pAddress)) { return false; }

    Lock lock(lock_);
    auto it = addresses_.find(pAddress->ID());
    std::set<Service> oldServices{};
    bool newAddress{false};

    if (addresses_.end() == it) {
        newAddress = true;
        auto [it2, added] =
            addresses_.emplace(pAddress->ID(), std::move(pAddress));
        it = it2;
    } else {
        oldServices = pAddress->Services();
        it->second.reset(pAddress.release());
    }

    OT_ASSERT(it->second);

    const auto& address = *it->second;
    const auto& id = address.ID();
    const auto newServices{address.Services()};

    if (newAddress) {
        protocols_[address.Style()].emplace(id);
        networks_[address.Type()].emplace(id);
        last_connected_[address.LastConnected()].emplace(id);

        for (const auto& service : newServices) {
            services_[service].emplace(id);
        }
    } else {
        for (const auto& service : oldServices) {
            if (0 == newServices.count(service)) {
                services_[service].erase(id);
            }
        }

        if (auto it = last_connected_.find(address.PreviousLastConnected());
            last_connected_.end() != it) {
            it->second.erase(address.ID());
            last_connected_[address.LastConnected()].emplace(id);
        }
    }

    return true;
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
