// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::blockchain::implementation
{
class Database final : virtual public internal::Database
{
public:
    bool AddOrUpdate(Address address) const noexcept final
    {
        return peers_.Insert(std::move(address));
    }
    bool ApplyUpdate(
        std::unique_ptr<block::Header> header,
        std::unique_ptr<client::internal::UpdateTransaction> update) const
        noexcept final
    {
        return headers_.ApplyUpdate(std::move(header), std::move(update));
    }
    bool ApplyUpdate(
        std::unique_ptr<client::internal::UpdateTransaction> update) const
        noexcept final
    {
        return headers_.ApplyUpdate({}, std::move(update));
    }
    // Throws std::out_of_range if no block at that position
    block::pHash BestBlock(const block::Height position) const
        noexcept(false) final
    {
        return headers_.BestBlock(position);
    }
    std::unique_ptr<block::Header> CurrentBest() const noexcept final
    {
        return headers_.CurrentBest();
    }
    block::Position CurrentCheckpoint() const noexcept final
    {
        return headers_.CurrentCheckpoint();
    }
    block::Position CurrentTip(const filter::Type type) const noexcept final
    {
        return filters_.CurrentTip(type);
    }
    client::DisconnectedList DisconnectedHashes() const noexcept final
    {
        return headers_.DisconnectedHashes();
    }
    Address Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept final
    {
        return peers_.Find(protocol, onNetworks, withServices);
    }
    bool HasDisconnectedChildren(const block::Hash& hash) const noexcept final
    {
        return headers_.HasDisconnectedChildren(hash);
    }
    bool HaveCheckpoint() const noexcept final
    {
        return headers_.HaveCheckpoint();
    }
    bool HaveFilter(const filter::Type type, const block::Hash& block) const
        noexcept final
    {
        return filters_.HaveFilter(type, block);
    }
    bool HeaderExists(const block::Hash& hash) const noexcept final
    {
        return headers_.HeaderExists(hash);
    }
    bool IsSibling(const block::Hash& hash) const noexcept final
    {
        return headers_.IsSibling(hash);
    }
    // Throws std::out_of_range if the header does not exist
    std::unique_ptr<block::Header> LoadHeader(const block::Hash& hash) const
        noexcept(false) final
    {
        return headers_.LoadHeader(hash);
    }
    std::vector<block::pHash> RecentHashes() const noexcept final
    {
        return headers_.RecentHashes();
    }
    bool SetTip(const filter::Type type, const block::Position position) const
        noexcept final
    {
        return filters_.SetTip(type, position);
    }
    client::Hashes SiblingHashes() const noexcept final
    {
        return headers_.SiblingHashes();
    }
    bool StoreFilter(
        const filter::Type type,
        const block::Hash& block,
        std::unique_ptr<const blockchain::internal::GCS> filter) const
        noexcept final
    {
        return filters_.StoreFilter(type, block, std::move(filter));
    }
    // Returns null pointer if the header does not exist
    std::unique_ptr<block::Header> TryLoadHeader(const block::Hash& hash) const
        noexcept final
    {
        return headers_.TryLoadHeader(hash);
    }

    ~Database() = default;

private:
    friend opentxs::Factory;

    struct Filters {
        block::Position CurrentTip(const filter::Type type) const noexcept;
        bool HaveFilter(const filter::Type type, const block::Hash& block) const
            noexcept;
        bool SetTip(const filter::Type type, const block::Position position)
            const noexcept;
        bool StoreFilter(
            const filter::Type type,
            const block::Hash& block,
            std::unique_ptr<const blockchain::internal::GCS> filter) const
            noexcept;

        Filters(const api::Core& api) noexcept;

    private:
        const api::Core& api_;
        mutable std::mutex lock_;
        mutable std::map<filter::Type, block::Position> tip_;
        mutable std::map<
            filter::Type,
            std::map<
                block::pHash,
                std::shared_ptr<const blockchain::internal::GCS>>>
            filters_;
    };

    struct Headers {
        using HeaderMap =
            std::map<block::pHash, std::unique_ptr<block::Header>>;

        const api::internal::Core& api_;
        const client::internal::Network& network_;
        mutable std::mutex lock_;
        HeaderMap block_headers_;
        std::map<block::Height, block::pHash> best_chain_;
        // parent block hash, disconnected block hash
        std::multimap<block::pHash, block::pHash> disconnected_;
        block::Position checkpoint_;
        std::set<block::pHash> sibling_chains_;

        static HeaderMap init_genesis(
            const api::internal::Core& api,
            const blockchain::Type type) noexcept;

        block::pHash BestBlock(const block::Height position) const
            noexcept(false);
        std::unique_ptr<block::Header> CurrentBest() const noexcept;
        block::Position CurrentCheckpoint() const noexcept;
        client::DisconnectedList DisconnectedHashes() const noexcept;
        bool HasDisconnectedChildren(const block::Hash& hash) const noexcept;
        bool HaveCheckpoint() const noexcept;
        bool HeaderExists(const block::Hash& hash) const noexcept;
        bool IsSibling(const block::Hash& hash) const noexcept;
        // Throws std::out_of_range if the header does not exist
        std::unique_ptr<block::Header> LoadHeader(
            const block::Hash& hash) const;
        std::vector<block::pHash> RecentHashes() const noexcept;
        client::Hashes SiblingHashes() const noexcept;
        // Returns null pointer if the header does not exist
        std::unique_ptr<block::Header> TryLoadHeader(
            const block::Hash& hash) const noexcept;

        bool ApplyUpdate(
            std::unique_ptr<block::Header> header,
            std::unique_ptr<client::internal::UpdateTransaction>
                update) noexcept;

        Headers(
            const api::internal::Core& api,
            const client::internal::Network& network,
            const blockchain::Type type) noexcept;

    private:
        // Throws std::out_of_range if the header does not exist
        std::unique_ptr<block::Header> load_header(
            const Lock& lock,
            const block::Hash& hash) const noexcept(false);
        std::vector<block::pHash> recent_hashes(const Lock& lock) const
            noexcept;
    };

    struct Peers {
        using AddressMap = std::map<OTIdentifier, Address>;
        using ProtocolIndexMap = std::map<Protocol, std::set<OTIdentifier>>;
        using ServiceIndexMap = std::map<Service, std::set<OTIdentifier>>;
        using TypeIndexMap = std::map<Type, std::set<OTIdentifier>>;

        mutable std::mutex lock_;
        AddressMap addresses_;
        ProtocolIndexMap protocols_;
        ServiceIndexMap services_;
        TypeIndexMap networks_;

        Address Find(
            const Protocol protocol,
            const std::set<Type> onNetworks,
            const std::set<Service> withServices) const noexcept;

        bool Insert(Address) noexcept;

        Peers() noexcept;
    };

    mutable Filters filters_;
    mutable Headers headers_;
    mutable Peers peers_;

    Database(
        const api::internal::Core& api,
        const client::internal::Network& network,
        const blockchain::Type type) noexcept;
    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&) = delete;
};
}  // namespace opentxs::blockchain::implementation
