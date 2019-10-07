// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::storage::implementation
{
// Content-aware storage module for opentxs
//
// Storage accepts serialized opentxs objects in protobuf form, writes them
// to persistant storage, and retrieves them on demand.
//
// All objects are stored in a key-value database. The keys are always the
// hash of the object being stored.
//
// This class maintains a set of index objects which map logical identifiers
// to object hashes. These index objects are stored in the same K-V namespace
// as the opentxs objects.
//
// The interface to a particular KV database is provided by child classes
// implementing this interface. Implementations need only provide methods for
// storing/retrieving arbitrary key-value pairs, and methods for setting and
// retrieving the hash of the root index object.
//
// The implementation of this interface must support the concept of "buckets"
// Objects are either stored and retrieved from either the primary bucket, or
// the alternate bucket. This allows for garbage collection of outdated keys
// to be implemented.
class Storage final : public opentxs::api::storage::StorageInternal
{
public:
    std::string AccountAlias(const Identifier& accountID) const final;
    ObjectList AccountList() const final;
    OTUnitID AccountContract(const Identifier& accountID) const final;
    OTNymID AccountIssuer(const Identifier& accountID) const final;
    OTNymID AccountOwner(const Identifier& accountID) const final;
    OTServerID AccountServer(const Identifier& accountID) const final;
    OTNymID AccountSigner(const Identifier& accountID) const final;
    proto::ContactItemType AccountUnit(const Identifier& accountID) const final;
    std::set<OTIdentifier> AccountsByContract(
        const identifier::UnitDefinition& contract) const final;
    std::set<OTIdentifier> AccountsByIssuer(
        const identifier::Nym& issuerNym) const final;
    std::set<OTIdentifier> AccountsByOwner(
        const identifier::Nym& ownerNym) const final;
    std::set<OTIdentifier> AccountsByServer(
        const identifier::Server& server) const final;
    std::set<OTIdentifier> AccountsByUnit(
        const proto::ContactItemType unit) const final;
    OTIdentifier Bip47AddressToChannel(
        const identifier::Nym& nymID,
        const std::string& address) const final;
    proto::ContactItemType Bip47Chain(
        const identifier::Nym& nymID,
        const Identifier& channelID) const final;
    Bip47ChannelList Bip47ChannelsByContact(
        const identifier::Nym& nymID,
        const Identifier& contactID) const final;
    Bip47ChannelList Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const proto::ContactItemType chain) const final;
    Bip47ChannelList Bip47ChannelsByLocalPaymentCode(
        const identifier::Nym& nymID,
        const std::string& code) const final;
    Bip47ChannelList Bip47ChannelsByRemotePaymentCode(
        const identifier::Nym& nymID,
        const std::string& code) const final;
    ObjectList Bip47ChannelsList(const identifier::Nym& nymID) const final;
    OTIdentifier Bip47Contact(
        const identifier::Nym& nymID,
        const Identifier& channelID) const final;
    std::string Bip47LocalPaymentCode(
        const identifier::Nym& nymID,
        const Identifier& channelID) const final;
    std::string Bip47RemotePaymentCode(
        const identifier::Nym& nymID,
        const Identifier& channelID) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::set<std::string> BlockchainAccountList(
        const std::string& nymID,
        const proto::ContactItemType type) const final;
    proto::ContactItemType BlockchainAccountType(
        const std::string& nymID,
        const std::string& accountID) const final;
    std::string BlockchainAddressOwner(
        proto::ContactItemType chain,
        std::string address) const final;
    ObjectList BlockchainTransactionList() const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CASH
    bool CheckTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const final;
#endif
    std::string ContactAlias(const std::string& id) const final;
    ObjectList ContactList() const final;
    ObjectList ContextList(const std::string& nymID) const final;
    std::string ContactOwnerNym(const std::string& nymID) const final;
    void ContactSaveIndices() const final;
    VersionNumber ContactUpgradeLevel() const final;
    bool CreateThread(
        const std::string& nymID,
        const std::string& threadID,
        const std::set<std::string>& participants) const final;
    bool DeleteAccount(const std::string& id) const final;
    std::string DefaultSeed() const final;
    bool DeleteContact(const std::string& id) const final;
    bool DeletePaymentWorkflow(
        const std::string& nymID,
        const std::string& workflowID) const final;
    std::uint32_t HashType() const final;
    ObjectList IssuerList(const std::string& nymID) const final;
    bool Load(
        const std::string& accountID,
        std::string& output,
        std::string& alias,
        const bool checking = false) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const std::string& nymID,
        const std::string& accountID,
        std::shared_ptr<proto::HDAccount>& output,
        const bool checking = false) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        std::shared_ptr<proto::Bip47Channel>& output,
        const bool checking = false) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& transaction,
        const bool checking = false) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        std::string& alias,
        const bool checking = false) const final;
    bool Load(
        const std::string& nym,
        const std::string& id,
        std::shared_ptr<proto::Context>& context,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Credential>& cred,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Nym>& nym,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Nym>& nym,
        std::string& alias,
        const bool checking = false) const final;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        std::shared_ptr<proto::Issuer>& issuer,
        const bool checking = false) const final;
    bool Load(
        const std::string& nymID,
        const std::string& workflowID,
        std::shared_ptr<proto::PaymentWorkflow>& workflow,
        const bool checking = false) const final;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false) const final;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerReply>& request,
        const bool checking = false) const final;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerRequest>& request,
        std::time_t& time,
        const bool checking = false) const final;
    bool Load(
        const identifier::Nym& nym,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        std::shared_ptr<proto::Purse>& output,
        const bool checking) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        std::string& alias,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        std::string& alias,
        const bool checking = false) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const identifier::Nym& nym,
        const api::client::blockchain::Coin& id,
        std::shared_ptr<proto::StorageBlockchainTxo>& output,
        const bool checking = false) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Load(
        const std::string& nymId,
        const std::string& threadId,
        std::shared_ptr<proto::StorageThread>& thread) const final;
    bool Load(
        std::shared_ptr<proto::Ciphertext>& output,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        const bool checking = false) const final;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        std::string& alias,
        const bool checking = false) const final;
    const std::set<std::string> LocalNyms() const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::set<OTNymID> LookupBlockchainTransaction(
        const std::string& txid) const final;
    std::set<api::client::blockchain::Coin> LookupElement(
        const identifier::Nym& nym,
        const Data& element) const noexcept final;
    std::set<api::client::blockchain::Coin> LookupTxid(
        const identifier::Nym& nym,
        const std::string& txid) const noexcept final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    void MapPublicNyms(NymLambda& lambda) const final;
    void MapServers(ServerLambda& lambda) const final;
    void MapUnitDefinitions(UnitLambda& lambda) const final;
#if OT_CASH
    bool MarkTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const final;
#endif
    bool MoveThreadItem(
        const std::string& nymId,
        const std::string& fromThreadID,
        const std::string& toThreadID,
        const std::string& itemID) const final;
    ObjectList NymBoxList(const std::string& nymID, const StorageBox box)
        const final;
    ObjectList NymList() const final;
    ObjectList PaymentWorkflowList(const std::string& nymID) const final;
    std::string PaymentWorkflowLookup(
        const std::string& nymID,
        const std::string& sourceID) const final;
    std::set<std::string> PaymentWorkflowsByAccount(
        const std::string& nymID,
        const std::string& accountID) const final;
    std::set<std::string> PaymentWorkflowsByState(
        const std::string& nymID,
        const proto::PaymentWorkflowType type,
        const proto::PaymentWorkflowState state) const final;
    std::set<std::string> PaymentWorkflowsByUnit(
        const std::string& nymID,
        const std::string& unitID) const final;
    std::pair<proto::PaymentWorkflowType, proto::PaymentWorkflowState>
    PaymentWorkflowState(
        const std::string& nymID,
        const std::string& workflowID) const final;
    bool RelabelThread(const std::string& threadID, const std::string& label)
        const final;
    bool RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID) const final;
    bool RemoveServer(const std::string& id) const final;
    bool RemoveThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& id) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool RemoveTxo(
        const identifier::Nym& nym,
        const api::client::blockchain::Coin& id) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool RemoveUnitDefinition(const std::string& id) const final;
    bool RenameThread(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& newID) const final;
    void RunGC() const final;
    ObjectList SeedList() const final;
    std::string ServerAlias(const std::string& id) const final;
    ObjectList ServerList() const final;
    bool SetAccountAlias(const std::string& id, const std::string& alias)
        const final;
    bool SetContactAlias(const std::string& id, const std::string& alias)
        const final;
    bool SetDefaultSeed(const std::string& id) const final;
    bool SetNymAlias(const std::string& id, const std::string& alias)
        const final;
    bool SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box) const final;
    bool SetReadState(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& itemId,
        const bool unread) const final;
    bool SetSeedAlias(const std::string& id, const std::string& alias)
        const final;
    bool SetServerAlias(const std::string& id, const std::string& alias)
        const final;
    bool SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias) const final;
    bool SetUnitDefinitionAlias(const std::string& id, const std::string& alias)
        const final;
    bool Store(
        const std::string& accountID,
        const std::string& data,
        const std::string& alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Server& server,
        const identifier::UnitDefinition& contract,
        const proto::ContactItemType unit) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(
        const std::string& nymID,
        const proto::ContactItemType type,
        const proto::HDAccount& data) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(
        const identifier::Nym& nymID,
        const proto::Bip47Channel& data,
        Identifier& channelID) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(
        const identifier::Nym& nym,
        const proto::BlockchainTransaction& data) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(
        const proto::Contact& data,
        std::map<OTData, OTIdentifier>& changed) const final;
    bool Store(const proto::Context& data) const final;
    bool Store(const proto::Credential& data) const final;
    bool Store(
        const proto::Nym& data,
        const std::string& alias = std::string("")) const final;
    bool Store(const std::string& nymID, const proto::Issuer& data) const final;
    bool Store(const std::string& nymID, const proto::PaymentWorkflow& data)
        const final;
    bool Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box,
        const std::string& account = std::string("")) const final;
    bool Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box) const final;
    bool Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box) const final;
    bool Store(const identifier::Nym& nym, const proto::Purse& purse)
        const final;
    bool Store(
        const proto::Seed& data,
        const std::string& alias = std::string("")) const final;
    bool Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string("")) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(
        const identifier::Nym& nym,
        const proto::StorageBlockchainTxo& data) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    bool Store(const proto::Ciphertext& serialized) const final;
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string("")) const final;
    ObjectList ThreadList(const std::string& nymID, const bool unreadOnly)
        const final;
    std::string ThreadAlias(
        const std::string& nymID,
        const std::string& threadID) const final;
    std::string UnitDefinitionAlias(const std::string& id) const final;
    ObjectList UnitDefinitionList() const final;
    std::size_t UnreadCount(
        const std::string& nymId,
        const std::string& threadId) const final;
    void UpgradeNyms() final;

    ~Storage();

private:
    friend opentxs::Factory;

    static const std::uint32_t HASH_TYPE;

    const Flag& running_;
    std::int64_t gc_interval_{std::numeric_limits<std::int64_t>::max()};
    mutable std::mutex write_lock_;
    mutable std::unique_ptr<opentxs::storage::Root> root_;
    mutable OTFlag primary_bucket_;
    std::vector<std::thread> background_threads_;
    const StorageConfig config_;
    std::unique_ptr<Multiplex> multiplex_p_;
    Multiplex& multiplex_;

    opentxs::storage::Root* root() const;
    const opentxs::storage::Root& Root() const;
    bool verify_write_lock(const Lock& lock) const;

    void Cleanup();
    void Cleanup_Storage();
    void CollectGarbage() const;
    void InitBackup() final;
    void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) final;
    void InitPlugins();
    Editor<opentxs::storage::Root> mutable_Root() const;
    void RunMapPublicNyms(NymLambda lambda) const;
    void RunMapServers(ServerLambda lambda) const;
    void RunMapUnits(UnitLambda lambda) const;
    void save(opentxs::storage::Root* in, const Lock& lock) const;
    void start() final;

    Storage(
        const Flag& running,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;
};
}  // namespace opentxs::api::storage::implementation
