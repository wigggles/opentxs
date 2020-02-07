// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/String.hpp"

#include "internal/api/Api.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"

#include "util/LMDB.hpp"

namespace opentxs::api::client::blockchain::database::implementation
{
class Database final
{
public:
    using Address = opentxs::blockchain::p2p::internal::Address;
    using Address_p = std::unique_ptr<Address>;
    using Chain = opentxs::blockchain::Type;
    using Protocol = opentxs::blockchain::p2p::Protocol;
    using Service = opentxs::blockchain::p2p::Service;
    using Type = opentxs::blockchain::p2p::Network;
    using UpdatedHeader = opentxs::blockchain::client::UpdatedHeader;

    bool AddOrUpdate(Address_p address) const noexcept
    {
        return peers_.Insert(std::move(address));
    }
    std::string AllocateStorageFolder(const std::string& dir) const noexcept;
    bool BlockHeaderExists(const opentxs::blockchain::block::Hash& hash) const
        noexcept;
    Address_p Find(
        const Chain chain,
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept
    {
        return peers_.Find(chain, protocol, onNetworks, withServices);
    }
    bool Import(std::vector<Address_p> peers) const noexcept
    {
        return peers_.Import(std::move(peers));
    }
    proto::BlockchainBlockHeader LoadBlockHeader(
        const opentxs::blockchain::block::Hash& hash) const noexcept(false);
    bool StoreBlockHeader(
        const opentxs::blockchain::block::Header& header) const noexcept;
    bool StoreBlockHeaders(const UpdatedHeader& headers) const noexcept;

    Database(
        const api::internal::Core& api,
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false);

private:
    struct Peers {
        auto Find(
            const Chain chain,
            const Protocol protocol,
            const std::set<Type> onNetworks,
            const std::set<Service> withServices) const noexcept -> Address_p;

        auto Import(std::vector<Address_p> peers) noexcept -> bool;
        auto Insert(Address_p address) noexcept -> bool;

        Peers(
            const api::internal::Core& api,
            opentxs::storage::lmdb::LMDB& lmdb) noexcept(false);

    private:
        using ChainIndexMap = std::map<Chain, std::set<std::string>>;
        using ProtocolIndexMap = std::map<Protocol, std::set<std::string>>;
        using ServiceIndexMap = std::map<Service, std::set<std::string>>;
        using TypeIndexMap = std::map<Type, std::set<std::string>>;
        using ConnectedIndexMap = std::map<std::string, Time>;

        const api::internal::Core& api_;
        opentxs::storage::lmdb::LMDB& lmdb_;
        mutable std::mutex lock_;
        ChainIndexMap chains_;
        ProtocolIndexMap protocols_;
        ServiceIndexMap services_;
        TypeIndexMap networks_;
        ConnectedIndexMap connected_;

        auto insert(const Lock& lock, std::vector<Address_p> peers) noexcept
            -> bool;
        auto load_address(const std::string& id) const noexcept(false)
            -> Address_p;
        template <typename Index, typename Map>
        auto read_index(
            const ReadView key,
            const ReadView value,
            Map& map) noexcept(false) -> bool
        {
            auto input = std::size_t{};

            if (sizeof(input) != key.size()) {
                throw std::runtime_error("Invalid key");
            }

            std::memcpy(&input, key.data(), key.size());
            map[static_cast<Index>(input)].emplace(value);

            return true;
        }
    };

    enum Table {
        BlockHeaders = 0,
        PeerDetails = 1,
        PeerChainIndex = 2,
        PeerProtocolIndex = 3,
        PeerServiceIndex = 4,
        PeerNetworkIndex = 5,
        PeerConnectedIndex = 6,
    };

    static const opentxs::storage::lmdb::TableNames table_names_;

    const api::internal::Core& api_;
    const OTString blockchain_path_;
    const OTString common_path_;
    opentxs::storage::lmdb::LMDB lmdb_;
    mutable Peers peers_;

    static OTString init_folder(
        const api::Legacy& legacy,
        const String& parent,
        const String& child) noexcept(false);
    static OTString init_storage_path(
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false);

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
};
}  // namespace opentxs::api::client::blockchain::database::implementation
