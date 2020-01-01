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

#include "internal/blockchain/block/Block.hpp"

#include <optional>
#include <random>

#include "Database.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::database::implementation::Database::"

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
          })
    , peers_(api, lmdb_)
{
}

Database::Peers::Peers(
    const api::internal::Core& api,
    opentxs::storage::lmdb::LMDB& lmdb) noexcept(false)
    : api_(api)
    , lmdb_(lmdb)
    , lock_()
    , chains_()
    , protocols_()
    , services_()
    , networks_()
    , connected_()
{
    using Dir = opentxs::storage::lmdb::LMDB::Dir;

    auto chain = [this](const auto key, const auto value) {
        return read_index<Chain>(key, value, chains_);
    };
    auto protocol = [this](const auto key, const auto value) {
        return read_index<Protocol>(key, value, protocols_);
    };
    auto service = [this](const auto key, const auto value) {
        return read_index<Service>(key, value, services_);
    };
    auto type = [this](const auto key, const auto value) {
        return read_index<Type>(key, value, networks_);
    };
    auto last = [this](const auto key, const auto value) {
        auto input = std::size_t{};

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        connected_.emplace(value, Clock::from_time_t(input));

        return true;
    };

    lmdb_.Read(PeerChainIndex, chain, Dir::Forward);
    lmdb_.Read(PeerProtocolIndex, protocol, Dir::Forward);
    lmdb_.Read(PeerServiceIndex, service, Dir::Forward);
    lmdb_.Read(PeerNetworkIndex, type, Dir::Forward);
    lmdb_.Read(PeerConnectedIndex, last, Dir::Forward);
}

auto Database::Peers::Find(
    const Chain chain,
    const Protocol protocol,
    const std::set<Type> onNetworks,
    const std::set<Service> withServices) const noexcept -> Address_p
{
    Lock lock(lock_);

    try {
        auto candidates = std::set<std::string>{};
        const auto& protocolSet = protocols_.at(protocol);
        const auto& chainSet = chains_.at(chain);

        if (protocolSet.empty()) { return {}; }
        if (chainSet.empty()) { return {}; }

        for (const auto& network : onNetworks) {
            try {
                for (const auto& id : networks_.at(network)) {
                    if ((1 == chainSet.count(id)) &&
                        (1 == protocolSet.count(id))) {
                        candidates.emplace(id);
                    }
                }
            } catch (...) {
            }
        }

        if (candidates.empty()) {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": No peers available for specified chain/protocol")
                .Flush();

            return {};
        }

        auto haveServices = std::set<std::string>{};

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

        if (haveServices.empty()) {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": No peers available with specified services")
                .Flush();

            return {};
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(": Choosing from ")(
                haveServices.size())(" candidates")
                .Flush();
        }

        auto weighted = std::vector<std::string>{};
        const auto now = Clock::now();

        for (const auto& id : haveServices) {
            auto weight = std::size_t{1};

            try {
                const auto& last = connected_.at(id);
                const auto since =
                    std::chrono::duration_cast<std::chrono::hours>(now - last);

                if (since.count() <= 1) {
                    weight = 10;
                } else if (since.count() <= 24) {
                    weight = 5;
                }
            } catch (...) {
            }

            weighted.insert(weighted.end(), weight, id);
        }

        std::vector<std::string> output;
        const std::size_t count{1};
        std::sample(
            weighted.begin(),
            weighted.end(),
            std::back_inserter(output),
            count,
            std::mt19937{std::random_device{}()});

        OT_ASSERT(count == output.size());

        LogTrace(OT_METHOD)(__FUNCTION__)(": Loading peer ")(output.front())
            .Flush();

        return load_address(output.front());
    } catch (...) {

        return {};
    }
}

bool Database::Peers::Import(std::vector<Address_p> peers) noexcept
{
    auto newPeers = std::vector<Address_p>{};

    for (auto& peer : peers) {
        if (false == lmdb_.Exists(Table::PeerDetails, peer->ID().str())) {
            newPeers.emplace_back(std::move(peer));
        }
    }

    Lock lock(lock_);

    return insert(lock, std::move(newPeers));
}

auto Database::Peers::Insert(Address_p pAddress) noexcept -> bool
{
    auto peers = std::vector<Address_p>{};
    peers.emplace_back(std::move(pAddress));
    Lock lock(lock_);

    return insert(lock, std::move(peers));
}

auto Database::Peers::insert(
    const Lock& lock,
    std::vector<Address_p> peers) noexcept -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (auto& pAddress : peers) {
        if (false == bool(pAddress)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid peer").Flush();

            return false;
        }

        auto& address = *pAddress;
        const auto id = address.ID().str();
        auto deleteServices = address.PreviousServices();

        for (const auto& service : address.Services()) {
            deleteServices.erase(service);
        }

        // write to database
        {
            auto result = lmdb_.Store(
                Table::PeerDetails,
                id,
                proto::ToString(address.Serialize()),
                parentTxn);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to save peer address")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerChainIndex,
                static_cast<std::size_t>(address.Chain()),
                id,
                parentTxn);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to save peer chain index")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerProtocolIndex,
                static_cast<std::size_t>(address.Style()),
                id,
                parentTxn);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to save peer protocol index")
                    .Flush();

                return false;
            }

            for (const auto& service : address.Services()) {
                result = lmdb_.Store(
                    Table::PeerServiceIndex,
                    static_cast<std::size_t>(service),
                    id,
                    parentTxn);

                if (false == result.first) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failed to save peer service index")
                        .Flush();

                    return false;
                }
            }

            for (const auto& service : deleteServices) {
                result.first = lmdb_.Delete(
                    Table::PeerServiceIndex,
                    static_cast<std::size_t>(service),
                    id,
                    parentTxn);
            }

            result = lmdb_.Store(
                Table::PeerNetworkIndex,
                static_cast<std::size_t>(address.Type()),
                id,
                parentTxn);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to save peer network index")
                    .Flush();

                return false;
            }

            result = lmdb_.Store(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(
                    Clock::to_time_t(address.LastConnected())),
                id,
                parentTxn);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to save peer network index")
                    .Flush();

                return false;
            }

            lmdb_.Delete(
                Table::PeerConnectedIndex,
                static_cast<std::size_t>(
                    Clock::to_time_t(address.PreviousLastConnected())),
                id,
                parentTxn);
        }

        // Update in-memory indices to match database
        {
            chains_[address.Chain()].emplace(id);
            protocols_[address.Style()].emplace(id);
            networks_[address.Type()].emplace(id);

            for (const auto& service : address.Services()) {
                services_[service].emplace(id);
            }

            for (const auto& service : deleteServices) {
                services_[service].erase(id);
            }

            connected_[id] = address.LastConnected();
        }
    }

    parentTxn.Finalize(true);

    return true;
}

auto Database::Peers::load_address(const std::string& id) const noexcept(false)
    -> Address_p
{
    auto output = std::optional<Address::SerializedType>{};
    lmdb_.Load(Table::PeerDetails, id, [&](const auto data) -> void {
        output =
            proto::Factory<Address::SerializedType>(data.data(), data.size());
    });

    if (false == output.has_value()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Peer ")(id)(" not found").Flush();

        throw std::out_of_range("Address not found");
    }

    const auto& serialized = output.value();

    if (false == proto::Validate(serialized, SILENT)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Peer ")(id)(" invalid").Flush();

        throw std::out_of_range("Invalid address");
    }

    return opentxs::Factory::BlockchainAddress(api_, serialized);
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
