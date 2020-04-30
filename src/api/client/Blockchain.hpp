// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/Blockchain.cpp"

#pragma once

#include <atomic>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#if OT_BLOCKCHAIN
#include "api/client/blockchain/database/Database.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceTree;
class HD;
}  // namespace blockchain

class Activity;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransaction;
class HDPath;
}  // namespace proto

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class Blockchain final : virtual public internal::Blockchain
{
public:
#if OT_BLOCKCHAIN
    using ThreadPoolType = opentxs::blockchain::client::internal::ThreadPool;
#endif  // OT_BLOCKCHAIN

    const blockchain::BalanceTree& Account(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false) final
    {
        return BalanceTree(nymID, chain);
    }
    std::set<OTIdentifier> AccountList(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept final;
    const api::internal::Core& API() const noexcept final { return api_; }
    bool AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept final;
    bool AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept final;
    const blockchain::internal::BalanceTree& BalanceTree(
        const identifier::Nym& nymID,
        const Chain chain) const noexcept(false) final;
#if OT_BLOCKCHAIN
    const blockchain::database::implementation::Database& BlockchainDB() const
        noexcept final
    {
        return db_;
    }
#endif  // OT_BLOCKCHAIN
    std::string CalculateAddress(
        const Chain chain,
        const blockchain::AddressStyle format,
        const Data& pubkey) const noexcept final;
    const api::client::Contacts& Contacts() const noexcept { return contacts_; }
    const TxoDB& DB() const noexcept { return txo_db_; }
    std::tuple<OTData, Style, Chain> DecodeAddress(
        const std::string& encoded) const noexcept final;
    std::string EncodeAddress(
        const Style style,
        const Chain chain,
        const Data& data) const noexcept final;
#if OT_BLOCKCHAIN
    const opentxs::blockchain::Network& GetChain(const Chain type) const
        noexcept(false) final;
#endif  // OT_BLOCKCHAIN
    const blockchain::HD& HDSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false) final;
#if OT_BLOCKCHAIN
    const opentxs::blockchain::client::internal::IO& IO() const noexcept final
    {
        return io_;
    }
#endif  // OT_BLOCKCHAIN
    OTIdentifier NewHDSubaccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept final;
    OTData PubkeyHash(const Chain chain, const Data& pubkey) const
        noexcept(false) final;
#if OT_BLOCKCHAIN
    const opentxs::network::zeromq::socket::Publish& Reorg() const
        noexcept final
    {
        return reorg_;
    }
    bool Start(const Chain type, const std::string& seednode) const
        noexcept final;
    bool Stop(const Chain type) const noexcept final;
#endif  // OT_BLOCKCHAIN
    bool StoreTransaction(
        const identifier::Nym& nymID,
        const Chain chain,
        const proto::BlockchainTransaction& transaction,
        const PasswordPrompt& reason) const noexcept final;
#if OT_BLOCKCHAIN
    auto ThreadPool() const noexcept -> const ThreadPoolType& final
    {
        return thread_pool_;
    }
#endif  // OT_BLOCKCHAIN
    std::shared_ptr<proto::BlockchainTransaction> Transaction(
        const std::string& id) const noexcept final;
    bool UpdateTransactions(const std::map<OTData, OTIdentifier>& changed) const
        noexcept final;

    Blockchain(
        const api::client::internal::Manager& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args) noexcept;

    ~Blockchain() final;

private:
    enum class Prefix {
        Unknown = 0,
        BitcoinP2PKH,
        BitcoinP2SH,
        BitcoinTestnetP2PKH,
        BitcoinTestnetP2SH,
        LitecoinP2PKH,
    };

    using IDLock = std::map<OTIdentifier, std::mutex>;
    using TXOs = std::vector<blockchain::Activity>;
    /// Unspent, spent
    using ParsedTransaction = std::pair<TXOs, TXOs>;
    using AddressMap = std::map<Prefix, std::string>;
    using AddressReverseMap = std::map<std::string, Prefix>;
    using StylePair = std::pair<Style, Chain>;
    using StyleMap = std::map<StylePair, Prefix>;
    using StyleReverseMap = std::map<Prefix, StylePair>;
    using TransactionMap = std::
        map<std::string, std::shared_ptr<const proto::BlockchainTransaction>>;
    using TransactionContactMap = std::map<std::string, std::set<OTIdentifier>>;
    using NymTransactionMap = std::map<OTNymID, TransactionContactMap>;

    friend opentxs::Factory;

    struct BalanceLists {
        client::blockchain::internal::BalanceList& Get(
            const Chain chain) noexcept;

        BalanceLists(api::client::internal::Blockchain& parent) noexcept;

    private:
        api::client::internal::Blockchain& parent_;
        std::mutex lock_;
        std::map<
            Chain,
            std::unique_ptr<client::blockchain::internal::BalanceList>>
            lists_;
    };
    struct Txo final : virtual public internal::Blockchain::TxoDB {
        bool AddSpent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::string txid) const noexcept final;
        bool AddUnspent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::vector<OTData>& elements) const noexcept final;
        bool Claim(const identifier::Nym& nym, const blockchain::Coin txo) const
            noexcept final;
        std::vector<Status> Lookup(
            const identifier::Nym& nym,
            const Data& element) const noexcept final;

        Txo(api::client::internal::Blockchain& parent);

    private:
        api::client::internal::Blockchain& parent_;
        mutable std::mutex lock_;
    };
#if OT_BLOCKCHAIN
    struct ThreadPoolManager final : virtual public ThreadPoolType {
        auto Endpoint() const noexcept -> std::string final;
        auto Reset(const Chain chain) const noexcept -> void final;
        auto Stop(const Chain chain) const noexcept -> Future final;

        auto Shutdown() noexcept -> void;

        ThreadPoolManager(const api::Core& api) noexcept;
        ~ThreadPoolManager();

    private:
        using NetworkData = std::tuple<
            bool,
            int,
            std::promise<void>,
            std::shared_future<void>,
            std::mutex>;
        using NetworkMap = std::map<Chain, NetworkData>;

        struct Worker {
            Worker(NetworkData& data) noexcept
                : data_(data)
                , not_moved_from_(true)
            {
                auto& [active, running, promise, future, mutex] = data_;
                ++running;
            }
            Worker(Worker&& rhs) noexcept
                : data_(rhs.data_)
                , not_moved_from_(true)
            {
                rhs.not_moved_from_ = false;
            }

            ~Worker()
            {
                if (not_moved_from_) {
                    auto& [active, running, promise, future, mutex] = data_;
                    Lock lock(mutex);
                    const auto tasks = --running;

                    if ((false == active) && (0 == tasks)) {
                        promise.set_value();
                    }
                }
            }

        private:
            NetworkData& data_;
            bool not_moved_from_;

            Worker(const Worker&) = delete;
        };

        const api::Core& api_;
        mutable NetworkMap map_;
        std::atomic<bool> running_;
        OTZMQPushSocket int_;
        mutable std::vector<OTZMQPullSocket> workers_;
        OTZMQListenCallback cbi_;
        OTZMQListenCallback cbe_;
        OTZMQPullSocket ext_;
        mutable bool init_;
        mutable std::mutex lock_;

        static auto init() noexcept -> NetworkMap;

        auto startup() const noexcept -> void;

        auto callback(zmq::Message& in) noexcept -> void;
        auto run(const Chain chain) noexcept -> std::optional<Worker>;
    };
#endif  // OT_BLOCKCHAIN

    static const AddressMap address_prefix_map_;
    static const AddressReverseMap address_prefix_reverse_map_;
    static const StyleMap address_style_map_;
    static const StyleReverseMap address_style_reverse_map_;

    const api::internal::Core& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable BalanceLists balance_lists_;
    mutable Txo txo_db_;
#if OT_BLOCKCHAIN
    ThreadPoolManager thread_pool_;
    opentxs::blockchain::client::internal::IO io_;
    blockchain::database::implementation::Database db_;
    OTZMQPublishSocket reorg_;
    mutable std::map<
        Chain,
        std::unique_ptr<opentxs::blockchain::client::internal::Network>>
        networks_;
#endif  // OT_BLOCKCHAIN

    OTData address_prefix(const Style style, const Chain chain) const
        noexcept(false);
    bool assign_transactions(
        const blockchain::internal::BalanceElement& element) const noexcept;
    bool assign_transactions(
        const identifier::Nym& nymID,
        const std::set<OTIdentifier> contacts,
        const TransactionMap& transactions) const noexcept;
    bool assign_transactions(
        const identifier::Nym& nymID,
        const Identifier& contactID,
        const TransactionMap& transactions) const noexcept;
    Bip44Type bip44_type(const proto::ContactItemType type) const noexcept;
    void init_path(
        const std::string& root,
        const proto::ContactItemType chain,
        const Bip32Index account,
        const BlockchainAccountType standard,
        proto::HDPath& path) const noexcept;
    bool move_transactions(
        const blockchain::internal::BalanceElement& element,
        const Identifier& fromContact) const noexcept;
    ParsedTransaction parse_transaction(
        const identifier::Nym& nym,
        const proto::BlockchainTransaction& transaction,
        const blockchain::internal::BalanceTree& tree,
        std::set<OTIdentifier>& contacts) const noexcept;
    std::string p2pkh(const Chain chain, const Data& pubkeyHash) const noexcept;
    std::string p2sh(const Chain chain, const Data& scriptHash) const noexcept;
    bool update_transactions(
        const Lock& lock,
        const identifier::Nym& nym,
        const TransactionContactMap& transactions) const noexcept;
    bool validate_nym(const identifier::Nym& nymID) const noexcept;

    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace opentxs::api::client::implementation
