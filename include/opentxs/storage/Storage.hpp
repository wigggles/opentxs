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

#ifndef OPENTXS_STORAGE_STORAGE_HPP
#define OPENTXS_STORAGE_STORAGE_HPP

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/interface/storage/StorageDriver.hpp"
#include "opentxs/storage/StorageConfig.hpp"

#include <atomic>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>

namespace opentxs
{

class OT;
class StoragePlugin;

namespace storage
{
class Root;
}  // namespace storage

typedef std::function<void(const proto::CredentialIndex&)> NymLambda;
typedef std::function<void(const proto::ServerContract&)> ServerLambda;
typedef std::function<void(const proto::UnitDefinition&)> UnitLambda;

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
class Storage : public virtual StorageDriver
{
private:
    friend class OT;
    typedef std::unique_lock<std::mutex> Lock;

    /** A set of metadata associated with a stored object
     *  * string: hash
     *  * string: alias
     */
    typedef std::pair<std::string, std::string> Metadata;

    /** Maps a logical id to the stored metadata for the object
     *  * string: id of the stored object
     *  * Metadata: metadata for the stored object
     */
    typedef std::map<std::string, Metadata> Index;

    std::uint32_t version_{0};
    std::int64_t gc_interval_{std::numeric_limits<int64_t>::max()};
    mutable std::unique_ptr<storage::Root> meta_;
    std::unique_ptr<StoragePlugin> primary_plugin_;
    mutable std::atomic<bool> primary_bucket_;

    void Cleanup_Storage();
    void CollectGarbage();
    bool EmptyBucket(const bool bucket) const override;
    bool Load(const std::string& key, const bool checking, std::string& value)
        const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    std::string LoadRoot() const override;
    storage::Root* meta() const;
    const storage::Root& Meta() const;
    bool Migrate(const std::string& key) const override;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;
    bool Store(const std::string& value, std::string& key) const override;
    bool StoreRoot(const std::string& hash) const override;
    bool verify_write_lock(const std::unique_lock<std::mutex>& lock) const;

    Editor<storage::Root> mutable_Meta();
    void RunMapPublicNyms(NymLambda lambda);
    void RunMapServers(ServerLambda lambda);
    void RunMapUnits(UnitLambda lambda);
    void save(storage::Root* in, const Lock& lock);

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;

protected:
    StorageConfig config_;
    Digest digest_;
    Random random_;

    mutable std::mutex write_lock_;
    std::atomic<bool> shutdown_;

    void Init();

    Storage(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random);

public:
    static const std::uint32_t HASH_TYPE;
    ObjectList ContextList(const std::string& nymID);
    std::string DefaultSeed();
    bool Load(
        const std::string& nym,
        const std::string& id,
        std::shared_ptr<proto::Context>& context,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Credential>& cred,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::CredentialIndex>& nym,
        std::string& alias,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerReply>& request,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::shared_ptr<proto::PeerRequest>& request,
        std::time_t& time,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& seed,
        std::string& alias,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& contract,
        std::string& alias,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& nymId,
        const std::string& threadId,
        std::shared_ptr<proto::StorageThread>& thread);
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        const bool checking = false);  // If true, suppress "not found" errors
    bool Load(
        const std::string& id,
        std::shared_ptr<proto::UnitDefinition>& contract,
        std::string& alias,
        const bool checking = false);  // If true, suppress "not found" errors
    void MapPublicNyms(NymLambda& lambda);
    void MapServers(ServerLambda& lambda);
    void MapUnitDefinitions(UnitLambda& lambda);
    ObjectList NymBoxList(const std::string& nymID, const StorageBox box) const;
    ObjectList NymList() const;
    bool RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID);
    bool RemoveServer(const std::string& id);
    bool RemoveUnitDefinition(const std::string& id);
    void RunGC();
    std::string ServerAlias(const std::string& id);
    ObjectList ServerList() const;
    bool SetDefaultSeed(const std::string& id);
    bool SetNymAlias(const std::string& id, const std::string& alias);
    bool SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box);
    bool SetSeedAlias(const std::string& id, const std::string& alias);
    bool SetServerAlias(const std::string& id, const std::string& alias);
    bool SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias);
    bool SetUnitDefinitionAlias(
        const std::string& id,
        const std::string& alias);
    bool Store(const proto::Context& data);
    bool Store(const proto::Credential& data);
    bool Store(
        const proto::CredentialIndex& data,
        const std::string& alias = std::string(""));
    bool Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box);
    bool Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box);
    bool Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box);
    bool Store(
        const proto::Seed& data,
        const std::string& alias = std::string(""));
    bool Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string(""));
    bool Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string(""));
    ObjectList ThreadList(const std::string& nymID) const;
    std::string ThreadAlias(
        const std::string& nymID,
        const std::string& threadID);
    std::string UnitDefinitionAlias(const std::string& id);
    ObjectList UnitDefinitionList() const;

    void Cleanup();
    ~Storage();
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGE_HPP
