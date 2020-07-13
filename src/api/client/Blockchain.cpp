// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <type_traits>

#include "2_Factory.hpp"
#if OT_BLOCKCHAIN
#include "core/Worker.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/api/client/Client.hpp"
#include "internal/api/client/Factory.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#if OT_BLOCKCHAIN
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/api/Endpoints.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/storage/Storage.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/identity/Nym.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"
#include "opentxs/protobuf/StorageThread.pb.h"
#include "opentxs/protobuf/StorageThreadItem.pb.h"
#include "util/Container.hpp"
#if OT_BLOCKCHAIN
#include "util/ScopeGuard.hpp"
#endif  // OT_BLOCKCHAIN
#include "util/HDIndex.hpp"
#if OT_BLOCKCHAIN
#include "util/Work.hpp"
#endif  // OT_BLOCKCHAIN

#define LOCK_NYM()                                                             \
    Lock mapLock(lock_);                                                       \
    auto& nymMutex = nym_lock_[nymID];                                         \
    mapLock.unlock();                                                          \
    Lock nymLock(nymMutex);

#define PATH_VERSION 1
#define COMPRESSED_PUBKEY_SIZE 33

#define OT_METHOD "opentxs::api::client::implementation::Blockchain::"

namespace zmq = opentxs::network::zeromq;

using ReturnType = opentxs::api::client::implementation::Blockchain;

namespace opentxs::factory
{
auto BlockchainAPI(
    const api::client::internal::Manager& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args) noexcept
    -> std::shared_ptr<api::client::internal::Blockchain>
{
    return std::make_shared<ReturnType>(
        api, activity, contacts, legacy, dataFolder, args);
}
}  // namespace opentxs::factory

namespace opentxs::api::client::implementation
{
const Blockchain::AddressReverseMap Blockchain::address_prefix_reverse_map_{
    {"00", Prefix::BitcoinP2PKH},
    {"05", Prefix::BitcoinP2SH},
    {"30", Prefix::LitecoinP2PKH},
    {"32", Prefix::LitecoinP2SH},
    {"3a", Prefix::LitecoinTestnetP2SH},
    {"6f", Prefix::BitcoinTestnetP2PKH},
    {"c4", Prefix::BitcoinTestnetP2SH},
};
const Blockchain::AddressMap Blockchain::address_prefix_map_{
    reverse_map(address_prefix_reverse_map_)};
const Blockchain::StyleMap Blockchain::address_style_map_{
    {{Style::P2PKH, Chain::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, Chain::BitcoinCash}, {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, Chain::Bitcoin_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, Chain::Bitcoin}, {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, Chain::Litecoin_testnet4},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, Chain::Litecoin}, {Prefix::LitecoinP2PKH, {}}},
    {{Style::P2SH, Chain::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, Chain::BitcoinCash}, {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, Chain::Bitcoin_testnet3}, {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, Chain::Bitcoin}, {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, Chain::Litecoin_testnet4},
     {Prefix::LitecoinTestnetP2SH, {Prefix::BitcoinTestnetP2SH}}},
    {{Style::P2SH, Chain::Litecoin},
     {Prefix::LitecoinP2SH, {Prefix::BitcoinP2SH}}},
};
const Blockchain::StyleReverseMap Blockchain::address_style_reverse_map_{
    ReturnType::reverse(address_style_map_)};

Blockchain::Blockchain(
    const api::client::internal::Manager& api,
    [[maybe_unused]] const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    [[maybe_unused]] const api::Legacy& legacy,
    [[maybe_unused]] const std::string& dataFolder,
    [[maybe_unused]] const ArgList& args) noexcept
    : api_(api)
#if OT_BLOCKCHAIN
    , activity_(activity)
#endif  // OT_BLOCKCHAIN
    , contacts_(contacts)
    , lock_()
    , nym_lock_()
    , accounts_(api_)
    , balance_lists_(*this)
#if OT_BLOCKCHAIN
    , key_generated_endpoint_(
          std::string{"inproc://"} + Identifier::Random()->str())
    , thread_pool_(api)
    , io_(api)
    , db_(api, legacy, dataFolder, args)
    , reorg_(api_.ZeroMQ().PublishSocket())
    , transaction_updates_(api_.ZeroMQ().PublishSocket())
    , peer_updates_(api_.ZeroMQ().PublishSocket())
    , key_updates_(api_.ZeroMQ().PublishSocket())
    , networks_()
    , balances_(*this, api)
    , running_(true)
    , heartbeat_(&Blockchain::heartbeat, this)
#endif  // OT_BLOCKCHAIN
{
    // WARNING: do not access api_.Wallet() during construction
#if OT_BLOCKCHAIN
    auto listen = reorg_->Start(api_.Endpoints().BlockchainReorg());

    OT_ASSERT(listen);

    listen =
        transaction_updates_->Start(api_.Endpoints().BlockchainTransactions());

    OT_ASSERT(listen);

    listen = peer_updates_->Start(api_.Endpoints().BlockchainPeer());

    OT_ASSERT(listen);

    listen = key_updates_->Start(key_generated_endpoint_);

    OT_ASSERT(listen);
#endif  // OT_BLOCKCHAIN
}

Blockchain::AccountCache::AccountCache(const api::Core& api) noexcept
    : api_(api)
    , lock_()
    , account_map_()
    , account_index_()
    , account_type_()
{
}

#if OT_BLOCKCHAIN
Blockchain::BalanceOracle::BalanceOracle(
    const api::client::internal::Blockchain& parent,
    const api::Core& api) noexcept
    : parent_(parent)
    , api_(api)
    , zmq_(api_.ZeroMQ())
    , cb_(zmq::ListenCallback::Factory([this](auto& in) { cb(in); }))
    , socket_(zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind))
    , lock_()
    , subscribers_()
{
    const auto started = socket_->Start(api_.Endpoints().BlockchainBalance());

    OT_ASSERT(started);
}
#endif  // OT_BLOCKCHAIN

Blockchain::BalanceLists::BalanceLists(
    api::client::internal::Blockchain& parent) noexcept
    : parent_(parent)
    , lock_()
    , lists_()
{
}

#if OT_BLOCKCHAIN
Blockchain::ThreadPoolManager::ThreadPoolManager(const api::Core& api) noexcept
    : api_(api)
    , map_(init())
    , running_(true)
    , int_(api_.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , workers_()
    , cbi_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
    , cbe_(zmq::ListenCallback::Factory([this](auto& in) { int_->Send(in); }))
    , ext_(api_.ZeroMQ().PullSocket(cbe_, zmq::socket::Socket::Direction::Bind))
    , init_(false)
    , lock_()
{
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::AccountCache::build_account_map(
    const Lock&,
    const Chain chain,
    std::optional<NymAccountMap>& map) const noexcept -> void
{
    const auto nyms = api_.Wallet().LocalNyms();
    map = NymAccountMap{};

    OT_ASSERT(map.has_value());

    auto& output = map.value();
    std::for_each(std::begin(nyms), std::end(nyms), [&](const auto& nym) {
        const auto& accounts =
            api_.Storage().BlockchainAccountList(nym->str(), Translate(chain));
        std::for_each(
            std::begin(accounts), std::end(accounts), [&](const auto& account) {
                auto& set = output[nym];
                auto accountID = api_.Factory().Identifier(account);
                account_index_.emplace(accountID, nym);
                account_type_.emplace(accountID, AccountType::HD);
                set.emplace(std::move(accountID));
            });
    });
}

auto Blockchain::AccountCache::get_account_map(
    const Lock& lock,
    const Chain chain) const noexcept -> NymAccountMap&
{
    auto& map = account_map_[chain];

    if (false == map.has_value()) { build_account_map(lock, chain, map); }

    OT_ASSERT(map.has_value());

    return map.value();
}

auto Blockchain::AccountCache::List(
    const identifier::Nym& nymID,
    const Chain chain) const noexcept -> std::set<OTIdentifier>
{
    Lock lock(lock_);
    const auto& map = get_account_map(lock, chain);
    auto it = map.find(nymID);

    if (map.end() == it) { return {}; }

    return it->second;
}

auto Blockchain::AccountCache::New(
    const Chain chain,
    const Identifier& account,
    const identifier::Nym& owner) const noexcept -> void
{
    Lock lock(lock_);
    get_account_map(lock, chain)[owner].emplace(account);
    account_index_.emplace(account, owner);
    account_type_.emplace(account, AccountType::HD);
}

auto Blockchain::AccountCache::Owner(const Identifier& accountID) const noexcept
    -> const identifier::Nym&
{
    static const auto blank = api_.Factory().NymID();

    try {

        return account_index_.at(accountID);
    } catch (...) {

        return blank;
    }
}

auto Blockchain::AccountCache::Type(const Identifier& accountID) const noexcept
    -> AccountType
{
    static const auto blank = api_.Factory().NymID();

    try {

        return account_type_.at(accountID);
    } catch (...) {

        return AccountType::Error;
    }
}

auto Blockchain::BalanceLists::Get(const Chain chain) noexcept
    -> client::blockchain::internal::BalanceList&
{
    Lock lock(lock_);
    auto it = lists_.find(chain);

    if (lists_.end() != it) { return *it->second; }

    auto [it2, added] = lists_.emplace(
        chain, opentxs::Factory::BlockchainBalanceList(parent_, chain));

    OT_ASSERT(added);
    OT_ASSERT(it2->second);

    return *it2->second;
}

#if OT_BLOCKCHAIN
auto Blockchain::BalanceOracle::cb(
    opentxs::network::zeromq::Message& in) noexcept -> void
{
    const auto& header = in.Header();

    if (0 == header.size()) { return; }

    const auto& connectionID = header.at(0);
    const auto& body = in.Body();

    if (0 == body.size()) { return; }

    auto output = opentxs::blockchain::Balance{};
    const auto& chainFrame = body.at(0);
    auto postcondition = ScopeGuard{[&]() {
        auto message = zmq_.ReplyMessage(in);
        message->AddFrame(chainFrame);
        message->AddFrame(output.first);
        message->AddFrame(output.second);
        socket_->Send(message);
    }};

    const auto chain = chainFrame.as<Chain>();

    if (0 == opentxs::blockchain::SupportedChains().count(chain)) { return; }

    try {
        output = parent_.GetChain(chain).GetBalance();
    } catch (...) {

        return;
    }

    Lock lock(lock_);
    subscribers_[chain].emplace(api_.Factory().Data(connectionID.Bytes()));
}

auto Blockchain::BalanceOracle::UpdateBalance(
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    auto cb = [this, &chain, &balance](const auto& in) {
        auto out = zmq_.Message(in);
        out->AddFrame();
        out->AddFrame(chain);
        out->AddFrame(balance.first);
        out->AddFrame(balance.second);
        socket_->Send(out);
    };
    Lock lock(lock_);
    const auto& subscribers = subscribers_[chain];
    std::for_each(std::begin(subscribers), std::end(subscribers), cb);
}

auto Blockchain::ThreadPoolManager::callback(zmq::Message& in) noexcept -> void
{
    const auto header = in.Header();

    if (2 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    auto worker = run(header.at(0).as<Chain>());

    if (false == worker.has_value()) { return; }

    switch (header.at(1).as<Work>()) {
        case Work::Wallet: {
            opentxs::blockchain::client::internal::Wallet::ProcessTask(in);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::ThreadPoolManager::Endpoint() const noexcept -> std::string
{
    return api_.Endpoints().InternalBlockchainThreadPool();
}

auto Blockchain::ThreadPoolManager::init() noexcept -> NetworkMap
{
    auto output = NetworkMap{};

    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        auto& [active, running, promise, future, mutex] = output[chain];
        active = true;
        future = promise.get_future();
    }

    return output;
}

auto Blockchain::ThreadPoolManager::Reset(const Chain chain) const noexcept
    -> void
{
    if (false == running_.load()) { return; }

    startup();
    auto& [active, running, promise, future, mutex] = map_.at(chain);
    Lock lock(mutex);

    if (false == active) {
        try {
            promise.set_value();
        } catch (...) {
        }

        promise = {};
        future = promise.get_future();
        active = true;
    }
}

auto Blockchain::ThreadPoolManager::run(const Chain chain) noexcept
    -> std::optional<Worker>
{
    auto& data = map_.at(chain);
    auto& [active, running, promise, future, mutex] = data;
    Lock lock(mutex);
    auto output = std::optional<Worker>{};

    if (active) { output.emplace(data); }

    return output;
}

auto Blockchain::ThreadPoolManager::Shutdown() noexcept -> void
{
    if (running_.exchange(false)) {
        ext_->Close();
        int_->Close();
        auto futures = std::vector<std::shared_future<void>>{};

        for (auto& [chain, data] : map_) { futures.emplace_back(Stop(chain)); }

        for (auto& future : futures) { future.get(); }
    }
}

auto Blockchain::ThreadPoolManager::startup() const noexcept -> void
{
    Lock lock(lock_);

    if (init_) { return; }

    const auto target = std::thread::hardware_concurrency();
    workers_.reserve(target);
    const auto random = Identifier::Random();
    const auto endpoint = std::string{"inproc://"} + random->str();
    auto zmq = int_->Start(endpoint);

    OT_ASSERT(zmq);

    for (unsigned int i{0}; i < target; ++i) {
        auto& worker = workers_.emplace_back(api_.ZeroMQ().PullSocket(
            cbi_, zmq::socket::Socket::Direction::Connect));
        auto zmq = worker->Start(endpoint);

        OT_ASSERT(zmq);

        LogTrace("Started blockchain worker thread ")(i).Flush();
    }

    zmq = ext_->Start(Endpoint());

    OT_ASSERT(zmq);

    init_ = true;
}

auto Blockchain::ThreadPoolManager::Stop(const Chain chain) const noexcept
    -> Future
{
    try {
        auto& [active, running, promise, future, mutex] = map_.at(chain);
        Lock lock(mutex);
        active = false;

        if (0 == running) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }

        return future;
    } catch (...) {
        auto promise = std::promise<void>{};
        promise.set_value();

        return promise.get_future();
    }
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::ActivityDescription(
    const identifier::Nym& nym,
    const Identifier& thread,
    const std::string& threadItemID) const noexcept -> std::string
{
#if OT_BLOCKCHAIN
    auto pThread = std::shared_ptr<proto::StorageThread>{};
    api_.Storage().Load(nym.str(), thread.str(), pThread);

    if (false == bool(pThread)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": thread ")(thread.str())(
            " does not exist for nym ")(nym.str())
            .Flush();

        return {};
    }

    const auto& data = *pThread;

    for (const auto& item : data.item()) {
        if (item.id() != threadItemID) { continue; }

        const auto txid = api_.Factory().Data(item.txid(), StringStyle::Raw);
        const auto chain = static_cast<opentxs::blockchain::Type>(item.chain());
        const auto pTx = LoadTransactionBitcoin(txid);

        if (false == bool(pTx)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": failed to load transaction ")(
                txid->asHex())
                .Flush();

            return {};
        }

        const auto& tx = *pTx;

        return this->ActivityDescription(nym, chain, tx);
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": item ")(threadItemID)(" not found ")
        .Flush();
#endif  // OT_BLOCKCHAIN

    return {};
}

#if OT_BLOCKCHAIN
auto Blockchain::ActivityDescription(
    const identifier::Nym& nym,
    const Chain chain,
    const Tx& transaction) const noexcept -> std::string
{
    auto output = std::stringstream{};
    const auto amount = transaction.NetBalanceChange(*this, nym);
    const auto memo = transaction.Memo(*this);

    if (0 < amount) {
        output << "Incoming ";
    } else if (0 > amount) {
        output << "Outgoing ";
    }

    output << opentxs::blockchain::internal::DisplayString(chain);
    output << " transaction";

    if (false == memo.empty()) { output << ": " << memo; }

    return output.str();
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::address_prefix(const Style style, const Chain chain) const
    noexcept(false) -> OTData
{
    return api_.Factory().Data(
        address_prefix_map_.at(address_style_map_.at({style, chain}).first),
        StringStyle::Hex);
}

auto Blockchain::AssignContact(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const blockchain::Subchain subchain,
    const Bip32Index index,
    const Identifier& contactID) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    LOCK_NYM()

    const auto chain = Translate(
        api_.Storage().BlockchainAccountType(nymID.str(), accountID.str()));

    OT_ASSERT(Chain::Unknown != chain);

    try {
        auto& node = balance_lists_.Get(chain).Nym(nymID).Node(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);
            const auto existing = element.Contact();

            if (contactID == existing) { return true; }

            return node.SetContact(subchain, index, contactID);
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account").Flush();

        return false;
    }
}

auto Blockchain::AssignLabel(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const blockchain::Subchain subchain,
    const Bip32Index index,
    const std::string& label) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    LOCK_NYM()

    const auto chain = Translate(
        api_.Storage().BlockchainAccountType(nymID.str(), accountID.str()));

    OT_ASSERT(Chain::Unknown != chain);

    try {
        auto& node = balance_lists_.Get(chain).Nym(nymID).Node(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);

            if (label == element.Label()) { return true; }

            return node.SetLabel(subchain, index, label);
        } catch (...) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account").Flush();

        return false;
    }
}

#if OT_BLOCKCHAIN
auto Blockchain::AssignTransactionMemo(
    const TxidHex& id,
    const std::string& label) const noexcept -> bool
{
    Lock lock(lock_);
    auto pTransaction = load_transaction(lock, id);

    if (false == bool(pTransaction)) { return false; }

    auto& transaction = *pTransaction;
    transaction.SetMemo(label);
    const auto serialized = transaction.Serialize(*this);

    OT_ASSERT(serialized.has_value());

    if (false == db_.StoreTransaction(serialized.value())) { return false; }

    broadcast_update_signal(transaction);

    return true;
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::BalanceTree(const identifier::Nym& nymID, const Chain chain)
    const noexcept(false) -> const blockchain::internal::BalanceTree&
{
    if (false == validate_nym(nymID)) {
        throw std::runtime_error("Invalid nym");
    }

    if (Chain::Unknown == chain) { throw std::runtime_error("Invalid chain"); }

    auto& balanceList = balance_lists_.Get(chain);

    return balanceList.Nym(nymID);
}

auto Blockchain::bip44_type(const proto::ContactItemType type) const noexcept
    -> Bip44Type
{
    switch (type) {
        case proto::CITEMTYPE_BTC: {

            return Bip44Type::BITCOIN;
        }
        case proto::CITEMTYPE_LTC: {

            return Bip44Type::LITECOIN;
        }
        case proto::CITEMTYPE_DOGE: {

            return Bip44Type::DOGECOIN;
        }
        case proto::CITEMTYPE_DASH: {

            return Bip44Type::DASH;
        }
        case proto::CITEMTYPE_BCH: {

            return Bip44Type::BITCOINCASH;
        }
        case proto::CITEMTYPE_TNBCH:
        case proto::CITEMTYPE_TNBTC:
        case proto::CITEMTYPE_TNXRP:
        case proto::CITEMTYPE_TNLTC:
        case proto::CITEMTYPE_TNXEM:
        case proto::CITEMTYPE_TNDASH:
        case proto::CITEMTYPE_TNMAID:
        case proto::CITEMTYPE_TNLSK:
        case proto::CITEMTYPE_TNDOGE:
        case proto::CITEMTYPE_TNXMR:
        case proto::CITEMTYPE_TNWAVES:
        case proto::CITEMTYPE_TNNXT:
        case proto::CITEMTYPE_TNSC:
        case proto::CITEMTYPE_TNSTEEM: {
            return Bip44Type::TESTNET;
        }
        default: {
            OT_FAIL;
        }
    }
}

#if OT_BLOCKCHAIN
auto Blockchain::broadcast_update_signal(
    const std::vector<pTxid>& transactions) const noexcept -> void
{
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [this](const auto& txid) {
            const auto data = db_.LoadTransaction(txid->Bytes());

            OT_ASSERT(data.has_value());

            const auto tx =
                factory::BitcoinTransaction(api_, *this, data.value());

            OT_ASSERT(tx);

            broadcast_update_signal(*tx);
        });
}

auto Blockchain::broadcast_update_signal(
    const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
    const noexcept -> void
{
    const auto chains = tx.Chains();
    std::for_each(std::begin(chains), std::end(chains), [&](const auto& chain) {
        auto out = api_.ZeroMQ().Message();
        out->AddFrame();
        out->AddFrame(tx.ID());
        out->AddFrame(chain);
        transaction_updates_->Send(out);
    });
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::CalculateAddress(
    const Chain chain,
    const Style format,
    const Data& pubkey) const noexcept -> std::string
{
    auto data = api_.Factory().Data();

    switch (format) {
        case Style::P2PKH: {
            try {
                data = PubkeyHash(chain, pubkey);
            } catch (...) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key.")
                    .Flush();

                return {};
            }
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported address style (")(
                static_cast<std::uint16_t>(format))(")")
                .Flush();

            return {};
        }
    }

    return EncodeAddress(format, chain, data);
}

auto Blockchain::DecodeAddress(const std::string& encoded) const noexcept
    -> DecodedAddress
{
    auto output = DecodedAddress{api_.Factory().Data(), Style::Unknown, {}};
    auto& [data, style, chains] = output;
    const auto bytes = api_.Factory().Data(
        api_.Crypto().Encode().IdentifierDecode(encoded), StringStyle::Raw);
    auto type = api_.Factory().Data();

    switch (bytes->size()) {
        case 21: {
            bytes->Extract(1, type, 0);
            auto prefix{Prefix::Unknown};

            try {
                prefix = address_prefix_reverse_map_.at(type->asHex());
            } catch (...) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to decode version byte (")(type->asHex())
                    .Flush();

                return output;
            }

            const auto& map = address_style_reverse_map_.at(prefix);

            for (const auto& [decodeStyle, decodeChain] : map) {
                style = decodeStyle;
                chains.emplace(decodeChain);
            }

            bytes->Extract(20, data, 1);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to decode address (")(
                bytes->asHex())
                .Flush();
        }
    }

    return output;
}

auto Blockchain::EncodeAddress(
    const Style style,
    const Chain chain,
    const Data& data) const noexcept -> std::string
{
    switch (style) {
        case Style::P2PKH: {

            return p2pkh(chain, data);
        }
        case Style::P2SH: {

            return p2sh(chain, data);
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported address style (")(
                static_cast<std::uint16_t>(style))(")")
                .Flush();

            return {};
        }
    }
}

#if OT_BLOCKCHAIN
auto Blockchain::GetChain(const Chain type) const noexcept(false)
    -> const opentxs::blockchain::Network&
{
    Lock lock(lock_);

    return *networks_.at(type);
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::GetKey(const blockchain::Key& id) const noexcept(false)
    -> const blockchain::BalanceNode::Element&
{
    const auto [str, subchain, index] = id;
    const auto account = api_.Factory().Identifier(str);

    switch (accounts_.Type(account)) {
        case AccountType::HD: {
            const auto& hd = HDSubaccount(accounts_.Owner(account), account);

            return hd.BalanceElement(subchain, index);
        }
        case AccountType::Error:
        case AccountType::Imported:
        case AccountType::PaymentCode:
        default: {
        }
    }

    throw std::out_of_range("key not found");
}

auto Blockchain::HDSubaccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept(false) -> const blockchain::HD&
{
    const auto type =
        api_.Storage().BlockchainAccountType(nymID.str(), accountID.str());

    if (proto::CITEMTYPE_ERROR == type) {
        throw std::out_of_range("Account does not exist");
    }

    auto& balanceList = balance_lists_.Get(Translate(type));
    auto& nym = balanceList.Nym(nymID);

    return nym.HDChain(accountID);
}

#if OT_BLOCKCHAIN
auto Blockchain::heartbeat() const noexcept -> void
{
    while (running_) {
        auto counter{-1};

        while (20 > ++counter) { Sleep(std::chrono::milliseconds{250}); }

        Lock lock(lock_);

        for (const auto& [key, value] : networks_) {
            if (false == running_) { return; }

            value->Heartbeat();
        }
    }
}

auto Blockchain::IndexItem(const ReadView bytes) const noexcept -> PatternID
{
    auto output = PatternID{};
    const auto hashed = api_.Crypto().Hash().HMAC(
        proto::HASHTYPE_SIPHASH24,
        db_.HashKey(),
        bytes,
        preallocated(sizeof(output), &output));

    OT_ASSERT(hashed);

    return output;
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::init_path(
    const std::string& root,
    const proto::ContactItemType chain,
    const Bip32Index account,
    const BlockchainAccountType standard,
    proto::HDPath& path) const noexcept -> void
{
    path.set_version(PATH_VERSION);
    path.set_root(root);

    switch (standard) {
        case BlockchainAccountType::BIP32: {
            path.add_child(HDIndex{account, Bip32Child::HARDENED});
        } break;
        case BlockchainAccountType::BIP44: {
            path.add_child(
                HDIndex{Bip43Purpose::HDWALLET, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

#if OT_BLOCKCHAIN
auto Blockchain::KeyGenerated(const Chain chain) const noexcept -> void
{
    auto work =
        MakeWork(api_, OTZMQWorkType{OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL});
    work->AddFrame(chain);
    key_updates_->Send(work);
}

auto Blockchain::load_transaction(const Lock& lock, const TxidHex& txid)
    const noexcept -> std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Transaction>
{
    return load_transaction(lock, api_.Factory().Data(txid, StringStyle::Hex));
}

auto Blockchain::load_transaction(const Lock& lock, const Txid& txid)
    const noexcept -> std::unique_ptr<
        opentxs::blockchain::block::bitcoin::internal::Transaction>
{
    const auto serialized = db_.LoadTransaction(txid.Bytes());

    if (false == serialized.has_value()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Transaction ")(txid.asHex())(
            " not found")
            .Flush();

        return {};
    }

    return factory::BitcoinTransaction(api_, *this, serialized.value());
}

auto Blockchain::LoadTransactionBitcoin(const TxidHex& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    Lock lock(lock_);

    return load_transaction(lock, txid);
}

auto Blockchain::LoadTransactionBitcoin(const Txid& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    Lock lock(lock_);

    return load_transaction(lock, txid);
}

auto Blockchain::LookupContacts(const std::string& address) const noexcept
    -> ContactList
{
    const auto [pubkeyHash, style, chain] = DecodeAddress(address);

    return LookupContacts(pubkeyHash);
}

auto Blockchain::LookupContacts(const Data& pubkeyHash) const noexcept
    -> ContactList
{
    return db_.LookupContact(pubkeyHash);
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::NewHDSubaccount(
    const identifier::Nym& nymID,
    const BlockchainAccountType standard,
    const Chain chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    if (false == validate_nym(nymID)) { return Identifier::Factory(); }

    if (Chain::Unknown == chain) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid chain").Flush();

        return Identifier::Factory();
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym does not exist.").Flush();

        return Identifier::Factory();
    }

    proto::HDPath nymPath{};

    if (false == nym->Path(nymPath)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No nym path.").Flush();

        return Identifier::Factory();
    }

    if (0 == nymPath.root().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing root.").Flush();

        return Identifier::Factory();
    }

    if (2 > nymPath.child().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid path.").Flush();

        return Identifier::Factory();
    }

    proto::HDPath accountPath{};
    init_path(
        nymPath.root(),
        Translate(chain),
        HDIndex{nymPath.child(1), Bip32Child::HARDENED},
        standard,
        accountPath);

    try {
        auto accountID = Identifier::Factory();
        auto& tree = balance_lists_.Get(chain).Nym(nymID);
        tree.AddHDNode(accountPath, accountID);
        accounts_.New(chain, accountID, nymID);
#if OT_BLOCKCHAIN
        balances_.UpdateBalance(chain, {});
#endif  // OT_BLOCKCHAIN

        return accountID;
    } catch (...) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Failed to create account")
            .Flush();

        return Identifier::Factory();
    }
}

auto Blockchain::p2pkh(const Chain chain, const Data& pubkeyHash) const noexcept
    -> std::string
{
    try {
        auto preimage = address_prefix(Style::P2PKH, chain);

        OT_ASSERT(1 == preimage->size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage->size());

        return api_.Crypto().Encode().IdentifierEncode(preimage);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain (")(
            static_cast<std::uint32_t>(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::p2sh(const Chain chain, const Data& pubkeyHash) const noexcept
    -> std::string
{
    try {
        auto preimage = address_prefix(Style::P2SH, chain);

        OT_ASSERT(1 == preimage->size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage->size());

        return api_.Crypto().Encode().IdentifierEncode(preimage);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain (")(
            static_cast<std::uint32_t>(chain))(")")
            .Flush();

        return "";
    }
}

#if OT_BLOCKCHAIN
auto Blockchain::ProcessContact(const Contact& contact) const noexcept -> bool
{
    broadcast_update_signal(db_.UpdateContact(contact));

    return true;
}

auto Blockchain::ProcessMergedContact(
    const Contact& parent,
    const Contact& child) const noexcept -> bool
{
    broadcast_update_signal(db_.UpdateMergedContact(parent, child));

    return true;
}
auto Blockchain::ProcessTransaction(
    const Chain chain,
    const Tx& in,
    const PasswordPrompt& reason) const noexcept -> bool
{
    Lock lock(lock_);
    auto pTransaction = in.clone();

    OT_ASSERT(pTransaction);

    auto& transaction = *pTransaction;
    const auto& id = transaction.ID();
    const auto txid = id.Bytes();

    if (const auto tx = db_.LoadTransaction(txid); tx.has_value()) {
        transaction.MergeMetadata(*this, chain, tx.value());
        auto updated = transaction.Serialize(*this);

        OT_ASSERT(updated.has_value());

        if (false == db_.StoreTransaction(updated.value())) { return false; }
    } else {
        auto serialized = transaction.Serialize(*this);

        OT_ASSERT(serialized.has_value());

        if (false == db_.StoreTransaction(serialized.value())) { return false; }
    }

    if (false == db_.AssociateTransaction(id, transaction.GetPatterns())) {
        return false;
    }

    return reconcile_activity_threads(lock, transaction);
}

#endif  // OT_BLOCKCHAIN

auto Blockchain::PubkeyHash(
    [[maybe_unused]] const Chain chain,
    const Data& pubkey) const noexcept(false) -> OTData
{
    if (pubkey.empty()) { throw std::runtime_error("Empty pubkey"); }

    if (COMPRESSED_PUBKEY_SIZE != pubkey.size()) {
        throw std::runtime_error("Incorrect pubkey size");
    }

    auto output = Data::Factory();

    if (false ==
        api_.Crypto().Hash().Digest(
            proto::HASHTYPE_BITCOIN, pubkey.Bytes(), output->WriteInto())) {
        throw std::runtime_error("Unable to calculate hash.");
    }

    return output;
}

auto Blockchain::reverse(const StyleMap& in) noexcept -> StyleReverseMap
{
    auto output = StyleReverseMap{};
    std::for_each(std::begin(in), std::end(in), [&](const auto& data) {
        const auto& [metadata, prefixData] = data;
        const auto& [preferred, additional] = prefixData;
        output[preferred].emplace(metadata);

        for (const auto& prefix : additional) {
            output[prefix].emplace(metadata);
        }
    });

    return output;
}

#if OT_BLOCKCHAIN
auto Blockchain::reconcile_activity_threads(const Lock& lock, const Txid& txid)
    const noexcept -> bool
{
    const auto tx = load_transaction(lock, txid);

    if (false == bool(tx)) { return false; }

    return reconcile_activity_threads(lock, *tx);
}

auto Blockchain::reconcile_activity_threads(
    const Lock& lock,
    const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
    const noexcept -> bool
{
    if (!activity_.AddBlockchainTransaction(*this, tx)) { return false; }

    broadcast_update_signal(tx);

    return true;
}

auto Blockchain::Start(const Chain type, const std::string& seednode)
    const noexcept -> bool
{
    if (0 == opentxs::blockchain::SupportedChains().count(type)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported chain").Flush();

        return false;
    }

    Lock lock(lock_);

    if (0 != networks_.count(type)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Chain already running").Flush();

        return true;
    }

    switch (type) {
        case Chain::Bitcoin:
        case Chain::Bitcoin_testnet3:
        case Chain::BitcoinCash:
        case Chain::BitcoinCash_testnet3: {
            thread_pool_.Reset(type);
            auto [it, added] = networks_.emplace(
                type,
                factory::BlockchainNetworkBitcoin(
                    api_, *this, type, seednode, ""));

            return it->second->Connect();
        }
        default: {
        }
    }

    return false;
}

auto Blockchain::Stop(const Chain type) const noexcept -> bool
{
    thread_pool_.Stop(type).get();
    Lock lock(lock_);
    auto it = networks_.find(type);

    if (networks_.end() == it) { return false; }

    OT_ASSERT(it->second);

    it->second->Shutdown();

    networks_.erase(it);

    return true;
}
#endif  // OT_BLOCKCHAIN

auto Blockchain::UpdateElement([
    [maybe_unused]] std::vector<ReadView>& pubkeyHashes) const noexcept -> void
{
#if OT_BLOCKCHAIN
    auto patterns = std::vector<PatternID>{};
    std::for_each(
        std::begin(pubkeyHashes),
        std::end(pubkeyHashes),
        [&](const auto& bytes) { patterns.emplace_back(IndexItem(bytes)); });
    LogTrace(OT_METHOD)(__FUNCTION__)(": ")(patterns.size())(
        " pubkey hashes have changed:")
        .Flush();
    auto transactions = std::vector<pTxid>{};
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& pattern) {
            LogTrace("    * ")(pattern).Flush();
            auto matches = db_.LookupTransactions(pattern);
            transactions.reserve(transactions.size() + matches.size());
            std::move(
                std::begin(matches),
                std::end(matches),
                std::back_inserter(transactions));
        });
    dedup(transactions);
    Lock lock(lock_);
    std::for_each(
        std::begin(transactions),
        std::end(transactions),
        [&](const auto& txid) { reconcile_activity_threads(lock, txid); });
#endif  // OT_BLOCKCHAIN
}

#if OT_BLOCKCHAIN
auto Blockchain::UpdatePeer(
    const opentxs::blockchain::Type chain,
    const std::string& address) const noexcept -> void
{
    auto work = MakeWork(api_, OTZMQWorkType{OT_ZMQ_NEW_PEER_SIGNAL});
    work->AddFrame(chain);
    work->AddFrame(address);
    peer_updates_->Send(work);
}

#endif  // OT_BLOCKCHAIN
auto Blockchain::validate_nym(const identifier::Nym& nymID) const noexcept
    -> bool
{
    if (false == nymID.empty()) {
        if (0 < api_.Wallet().LocalNyms().count(nymID)) { return true; }
    }

    return false;
}

#if OT_BLOCKCHAIN
Blockchain::ThreadPoolManager::~ThreadPoolManager()
{
    if (running_.load()) { Shutdown(); }
}
#endif  // OT_BLOCKCHAIN

Blockchain::~Blockchain()
{
#if OT_BLOCKCHAIN
    running_ = false;

    if (heartbeat_.joinable()) { heartbeat_.join(); }

    LogVerbose("Shutting down ")(networks_.size())(" blockchain clients")
        .Flush();
    thread_pool_.Shutdown();

    for (auto& [chain, network] : networks_) { network->Shutdown().get(); }

    networks_.clear();
    io_.Shutdown();

#endif  // OT_BLOCKCHAIN
}
}  // namespace opentxs::api::client::implementation
