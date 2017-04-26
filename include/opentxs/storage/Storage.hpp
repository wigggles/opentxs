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
namespace storage
{
class Root;
}  // namespace storage

typedef std::function<bool(const uint32_t, const std::string&, std::string&)>
    Digest;
typedef std::function<std::string()> Random;
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
class Storage
{
public:
    template <class T>
    bool LoadProto(
        const std::string& hash,
        std::shared_ptr<T>& serialized,
        const bool checking = false) const
    {
        std::string raw;
        const bool loaded = LoadRaw(hash, raw, checking);
        bool valid = false;

        if (loaded) {
            serialized.reset(new T);
            serialized->ParseFromArray(raw.data(), raw.size());
            valid = proto::Check<T>(*serialized, 1, serialized->version());
        }

        if (!valid) {
            if (loaded) {
                std::cerr << "Specified object was located but could not be "
                          << "validated. Database is corrupt." << std::endl
                          << "Hash: " << hash << std::endl
                          << "Size: " << raw.size() << std::endl;
            } else {
                std::cerr << "Specified object is missing. Database is "
                          << "corrupt." << std::endl
                          << "Hash: " << hash << std::endl
                          << "Size: " << raw.size() << std::endl;
            }
        }

        OT_ASSERT(valid);

        return valid;
    }

    template <class T>
    bool StoreProto(const T& data, std::string& key, std::string& plaintext)
        const
    {
        const auto version = data.version();

        if (!proto::Check<T>(data, version, version)) {

            return false;
        }

        plaintext = proto::ProtoAsString<T>(data);

        return StoreRaw(plaintext, key);
    }

    template <class T>
    bool StoreProto(const T& data, std::string& key) const
    {
        std::string notUsed;

        return StoreProto<T>(data, key, notUsed);
    }

    template <class T>
    bool StoreProto(const T& data) const
    {
        std::string notUsed;

        return StoreProto<T>(data, notUsed);
    }

private:
    friend class Root;
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
    mutable std::atomic<bool> current_bucket_;
    mutable std::unique_ptr<storage::Root> meta_;
    std::function<bool(const std::string&)> migrate_;

    bool MigrateKey(const std::string& key) const;
    void save(storage::Root* in, const Lock& lock);
    bool verify_write_lock(const std::unique_lock<std::mutex>& lock) const;

    void Cleanup_Storage();
    void CollectGarbage();
    storage::Root* meta() const;
    const storage::Root& Meta() const;
    Editor<storage::Root> mutable_Meta();
    void RunMapPublicNyms(NymLambda lambda);
    void RunMapServers(ServerLambda lambda);
    void RunMapUnits(UnitLambda lambda);

    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage& operator=(Storage&&) = delete;

protected:
    const std::uint32_t HASH_TYPE = 2;  // BTC160
    StorageConfig config_;
    Digest digest_;
    Random random_;

    mutable std::mutex write_lock_;
    std::atomic<bool> shutdown_;

    virtual void Init();

    virtual std::string LoadRoot() const = 0;

    Storage(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random);

public:
    ObjectList ContextList(const std::string& nymID);
    std::string DefaultSeed();
    virtual bool EmptyBucket(const bool bucket) const = 0;
    virtual bool Load(
        const std::string& key,
        std::string& value,
        const bool bucket) const = 0;
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
    bool LoadRaw(
        const std::string& hash,
        std::string& output,
        const bool checking = false) const;
    void MapPublicNyms(NymLambda& lambda);
    void MapServers(ServerLambda& lambda);
    void MapUnitDefinitions(UnitLambda& lambda);
    ObjectList NymBoxList(const std::string& nymID, const StorageBox box);
    ObjectList NymList() const;
    bool RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID);
    bool RemoveServer(const std::string& id);
    bool RemoveUnitDefinition(const std::string& id);
    void RunGC();
    std::string ServerAlias(const std::string& id);
    ObjectList ServerList();
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
    virtual bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const = 0;
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
    bool StoreRaw(const std::string& data, std::string& key) const;
    virtual bool StoreRoot(const std::string& hash) const = 0;
    ObjectList ThreadList(const std::string& nymID);
    std::string ThreadAlias(
        const std::string& nymID,
        const std::string& threadID);
    std::string UnitDefinitionAlias(const std::string& id);
    ObjectList UnitDefinitionList();

    virtual void Cleanup();
    virtual ~Storage();
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGE_HPP
