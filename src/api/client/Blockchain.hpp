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
#include "opentxs/api/Factory.hpp"
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
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/socket/Router.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/protobuf/ContactEnums.pb.h"

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

namespace internal
{
struct Manager;
}  // namespace internal

class Activity;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
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

    auto Account(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const blockchain::BalanceTree& final
    {
        return BalanceTree(nymID, chain);
    }
    auto AccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> std::set<OTIdentifier> final
    {
        return accounts_.AccountList(nymID, chain);
    }
    auto API() const noexcept -> const api::internal::Core& final
    {
        return api_;
    }
    auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept -> bool final;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const blockchain::Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept -> bool final;
    auto BalanceTree(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const blockchain::internal::BalanceTree& final;
#if OT_BLOCKCHAIN
    auto BlockchainDB() const noexcept
        -> const blockchain::database::implementation::Database& final
    {
        return db_;
    }
#endif  // OT_BLOCKCHAIN
    auto CalculateAddress(
        const Chain chain,
        const blockchain::AddressStyle format,
        const Data& pubkey) const noexcept -> std::string final;
    auto Contacts() const noexcept -> const api::client::Contacts&
    {
        return contacts_;
    }
    auto DB() const noexcept -> const TxoDB& { return txo_db_; }
    auto DecodeAddress(const std::string& encoded) const noexcept
        -> std::tuple<OTData, Style, Chain> final;
    auto EncodeAddress(const Style style, const Chain chain, const Data& data)
        const noexcept -> std::string final;
#if OT_BLOCKCHAIN
    auto GetChain(const Chain type) const noexcept(false)
        -> const opentxs::blockchain::Network& final;
#endif  // OT_BLOCKCHAIN
    auto HDSubaccount(const identifier::Nym& nymID, const Identifier& accountID)
        const noexcept(false) -> const blockchain::HD& final;
#if OT_BLOCKCHAIN
    auto IO() const noexcept
        -> const opentxs::blockchain::client::internal::IO& final
    {
        return io_;
    }
#endif  // OT_BLOCKCHAIN
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const BlockchainAccountType standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier final;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym& final
    {
        return accounts_.AccountOwner(accountID);
    }
    auto Owner(const blockchain::Key& key) const noexcept
        -> const identifier::Nym& final
    {
        return Owner(api_.Factory().Identifier(std::get<0>(key)));
    }
    auto PubkeyHash(const Chain chain, const Data& pubkey) const noexcept(false)
        -> OTData final;
#if OT_BLOCKCHAIN
    auto Reorg() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& final
    {
        return reorg_;
    }
    auto Start(const Chain type, const std::string& seednode) const noexcept
        -> bool final;
    auto Stop(const Chain type) const noexcept -> bool final;
#endif  // OT_BLOCKCHAIN
    auto StoreTransaction(
        const identifier::Nym& nymID,
        const Chain chain,
        const proto::BlockchainTransaction& transaction,
        const PasswordPrompt& reason) const noexcept -> bool final;
#if OT_BLOCKCHAIN
    auto ThreadPool() const noexcept -> const ThreadPoolType& final
    {
        return thread_pool_;
    }
#endif  // OT_BLOCKCHAIN
    auto Transaction(const std::string& id) const noexcept
        -> std::shared_ptr<proto::BlockchainTransaction> final;
#if OT_BLOCKCHAIN
    auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void
    {
        balances_.UpdateBalance(chain, balance);
    }
#endif  // OT_BLOCKCHAIN
    auto UpdateTransactions(const std::map<OTData, OTIdentifier>& changed)
        const noexcept -> bool final;

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

    struct AccountCache {
        auto AccountList(const identifier::Nym& nymID, const Chain chain)
            const noexcept -> std::set<OTIdentifier>;
        auto AccountOwner(const Identifier& accountID) const noexcept
            -> const identifier::Nym&;
        auto NewAccount(
            const Chain chain,
            const Identifier& account,
            const identifier::Nym& owner) const noexcept -> void;

        AccountCache(const api::Core& api) noexcept;

    private:
        using NymAccountMap = std::map<OTNymID, std::set<OTIdentifier>>;
        using ChainAccountMap = std::map<Chain, std::optional<NymAccountMap>>;
        using AccountNymIndex = std::map<OTIdentifier, OTNymID>;

        const api::Core& api_;
        mutable std::mutex lock_;
        mutable ChainAccountMap account_map_;
        mutable AccountNymIndex account_index_;

        auto build_account_map(
            const Lock&,
            const Chain chain,
            std::optional<NymAccountMap>& map) const noexcept -> void;
        auto get_account_map(const Lock&, const Chain chain) const noexcept
            -> NymAccountMap&;
    };

    struct BalanceLists {
        auto Get(const Chain chain) noexcept
            -> client::blockchain::internal::BalanceList&;

        BalanceLists(api::client::internal::Blockchain& parent) noexcept;

    private:
        api::client::internal::Blockchain& parent_;
        std::mutex lock_;
        std::map<
            Chain,
            std::unique_ptr<client::blockchain::internal::BalanceList>>
            lists_;
    };
#if OT_BLOCKCHAIN
    struct BalanceOracle {
        using Balance = opentxs::blockchain::Balance;
        using Chain = opentxs::blockchain::Type;

        auto UpdateBalance(const Chain chain, const Balance balance)
            const noexcept -> void;

        BalanceOracle(
            const api::client::internal::Blockchain& parent,
            const api::Core& api) noexcept;

    private:
        const api::client::internal::Blockchain& parent_;
        const api::Core& api_;
        const opentxs::network::zeromq::Context& zmq_;
        OTZMQListenCallback cb_;
        OTZMQRouterSocket socket_;
        mutable std::mutex lock_;
        mutable std::map<Chain, std::set<OTData>> subscribers_;

        auto cb(opentxs::network::zeromq::Message& message) noexcept -> void;

        BalanceOracle() = delete;
    };
#endif  // OT_BLOCKCHAIN
    struct Txo final : virtual public internal::Blockchain::TxoDB {
        auto AddSpent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::string txid) const noexcept -> bool final;
        auto AddUnspent(
            const identifier::Nym& nym,
            const blockchain::Coin txo,
            const std::vector<OTData>& elements) const noexcept -> bool final;
        auto Claim(const identifier::Nym& nym, const blockchain::Coin txo)
            const noexcept -> bool final;
        auto Lookup(const identifier::Nym& nym, const Data& element)
            const noexcept -> std::vector<Status> final;

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

    const api::client::internal::Manager& api_;
    const api::client::Activity& activity_;
    const api::client::Contacts& contacts_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable AccountCache accounts_;
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
    BalanceOracle balances_;
#endif  // OT_BLOCKCHAIN

    auto address_prefix(const Style style, const Chain chain) const
        noexcept(false) -> OTData;
    auto assign_transactions(
        const blockchain::internal::BalanceElement& element) const noexcept
        -> bool;
    auto assign_transactions(
        const identifier::Nym& nymID,
        const std::set<OTIdentifier> contacts,
        const TransactionMap& transactions) const noexcept -> bool;
    auto assign_transactions(
        const identifier::Nym& nymID,
        const Identifier& contactID,
        const TransactionMap& transactions) const noexcept -> bool;
    auto bip44_type(const proto::ContactItemType type) const noexcept
        -> Bip44Type;
    void init_path(
        const std::string& root,
        const proto::ContactItemType chain,
        const Bip32Index account,
        const BlockchainAccountType standard,
        proto::HDPath& path) const noexcept;
    auto move_transactions(
        const blockchain::internal::BalanceElement& element,
        const Identifier& fromContact) const noexcept -> bool;
    auto parse_transaction(
        const identifier::Nym& nym,
        const proto::BlockchainTransaction& transaction,
        const blockchain::internal::BalanceTree& tree,
        std::set<OTIdentifier>& contacts) const noexcept -> ParsedTransaction;
    auto p2pkh(const Chain chain, const Data& pubkeyHash) const noexcept
        -> std::string;
    auto p2sh(const Chain chain, const Data& scriptHash) const noexcept
        -> std::string;
    auto update_transactions(
        const Lock& lock,
        const identifier::Nym& nym,
        const TransactionContactMap& transactions) const noexcept -> bool;
    auto validate_nym(const identifier::Nym& nymID) const noexcept -> bool;

    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain &&) -> Blockchain& = delete;
};
}  // namespace opentxs::api::client::implementation
