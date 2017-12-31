/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP
#define OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/storage/StorageConfig.hpp"

#include <atomic>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

namespace opentxs
{
class StorageDriver;
class StorageMultiplex;
class StoragePlugin;
class String;
class SymmetricKey;

namespace storage
{
class Root;
}  // namespace storage

namespace api
{
class Crypto;

namespace implementation
{
class Native;
}  // namespace implementation

namespace storage
{
namespace implementation
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
class Storage : public opentxs::api::storage::Storage
{
public:
    std::set<std::string> BlockchainAccountList(
        const std::string& nymID,
        const proto::ContactItemType type) override;
    std::string BlockchainAddressOwner(
        proto::ContactItemType chain,
        std::string address) override;
    ObjectList BlockchainTransactionList() override;
    std::string ContactAlias(const std::string& id) override;
    ObjectList ContactList() override;
    ObjectList ContextList(const std::string& nymID) override;
    std::string ContactOwnerNym(const std::string& nymID) override;
    void ContactSaveIndices() override;
    std::uint32_t ContactUpgradeLevel() const override;
    bool CreateThread(
        const std::string& nymID,
        const std::string& threadID,
        const std::set<std::string>& participants) override;
    std::string DefaultSeed() override;
    bool DeleteContact(const std::string& id) override;
    std::uint32_t HashType() const override;
    ObjectList IssuerList(const std::string& nymID) override;
    bool Load(
        const std::string& nymID,
        const std::string& accountID,
        std::shared_ptr<proto::Bip44Account>& output,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::BlockchainTransaction>& transaction,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Contact>& contact,
        std::string& alias,
        const bool checking = false) override;
    bool Load(
        const std::string& nym,
        const std::string& id,
        std::shared_ptr<proto::Context>& context,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Credential>& cred,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        std::string& alias,
        const bool checking = false) override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        std::shared_ptr<proto::Issuer>& issuer,
        const bool checking = false) override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false) override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerReply>& request,
        const bool checking = false) override;
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerRequest>& request,
        std::time_t& time,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        std::string& alias,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        std::string& alias,
        const bool checking = false) override;
    bool Load(
        const std::string& nymId,
        const std::string& threadId,
        std::shared_ptr<proto::StorageThread>& thread) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        const bool checking = false) override;
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        std::string& alias,
        const bool checking = false) override;
    void MapPublicNyms(NymLambda& lambda) override;
    void MapServers(ServerLambda& lambda) override;
    void MapUnitDefinitions(UnitLambda& lambda) override;
    bool MoveThreadItem(
        const std::string& nymId,
        const std::string& fromThreadID,
        const std::string& toThreadID,
        const std::string& itemID) override;
    ObjectList NymBoxList(const std::string& nymID, const StorageBox box)
        const override;
    ObjectList NymList() const override;
    bool RelabelThread(const std::string& threadID, const std::string& label)
        override;
    bool RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID) override;
    bool RemoveServer(const std::string& id) override;
    bool RemoveUnitDefinition(const std::string& id) override;
    bool RenameThread(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& newID) override;
    void RunGC() override;
    std::string ServerAlias(const std::string& id) override;
    ObjectList ServerList() const override;
    bool SetContactAlias(const std::string& id, const std::string& alias)
        override;
    bool SetDefaultSeed(const std::string& id) override;
    bool SetNymAlias(const std::string& id, const std::string& alias) override;
    bool SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box) override;
    bool SetReadState(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& itemId,
        const bool unread) override;
    bool SetSeedAlias(const std::string& id, const std::string& alias) override;
    bool SetServerAlias(const std::string& id, const std::string& alias)
        override;
    bool SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias) override;
    bool SetUnitDefinitionAlias(const std::string& id, const std::string& alias)
        override;
    bool Store(
        const std::string& nymID,
        const proto::ContactItemType type,
        const proto::Bip44Account& data) override;
    bool Store(const proto::BlockchainTransaction& data) override;
    bool Store(const proto::Contact& data) override;
    bool Store(const proto::Context& data) override;
    bool Store(const proto::Credential& data) override;
    bool Store(
        const proto::CredentialIndex& data,
        const std::string& alias = std::string("")) override;
    bool Store(const std::string& nymID, const proto::Issuer& data) override;
    bool Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box) override;
    bool Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box) override;
    bool Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box) override;
    bool Store(
        const proto::Seed& data,
        const std::string& alias = std::string("")) override;
    bool Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string("")) override;
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string("")) override;
    ObjectList ThreadList(const std::string& nymID, const bool unreadOnly)
        const override;
    std::string ThreadAlias(
        const std::string& nymID,
        const std::string& threadID) override;
    std::string UnitDefinitionAlias(const std::string& id) override;
    ObjectList UnitDefinitionList() const override;
    std::size_t UnreadCount(
        const std::string& nymId,
        const std::string& threadId) override;

    ~Storage();

private:
    friend class opentxs::api::implementation::Native;

    static const std::uint32_t HASH_TYPE;

    const std::atomic<bool>& shutdown_;
    std::int64_t gc_interval_{std::numeric_limits<std::int64_t>::max()};
    mutable std::mutex write_lock_;
    mutable std::unique_ptr<opentxs::storage::Root> root_;
    mutable std::atomic<bool> primary_bucket_;
    std::vector<std::thread> background_threads_;
    StorageConfig config_;
    std::unique_ptr<StorageMultiplex> multiplex_p_;
    StorageMultiplex& multiplex_;

    opentxs::storage::Root* root() const;
    const opentxs::storage::Root& Root() const;
    bool verify_write_lock(const Lock& lock) const;

    void Cleanup();
    void Cleanup_Storage();
    void CollectGarbage();
    void InitBackup();
    void InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key);
    void InitPlugins();
    Editor<opentxs::storage::Root> mutable_Root();
    void RunMapPublicNyms(NymLambda lambda);
    void RunMapServers(ServerLambda lambda);
    void RunMapUnits(UnitLambda lambda);
    void save(opentxs::storage::Root* in, const Lock& lock);
    void start();

    Storage(
        const std::atomic<bool>& shutdown,
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
}  // namespace implementation
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_STORAGE_IMPLEMENTATION_STORAGE_HPP
