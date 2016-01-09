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

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <opentxs-proto/verify/VerifyCredentials.hpp>
#include <opentxs-proto/verify/VerifyStorage.hpp>

namespace opentxs
{

typedef std::function<bool(const uint32_t, const std::string&, std::string&)>
    Digest;

template<class T>
std::string ProtoAsString(const T& serialized)
{
    int size = serialized.ByteSize();
    char* protoArray = new char [size];

    serialized.SerializeToArray(protoArray, size);
    std::string serializedData(protoArray, size);
    delete[] protoArray;
    return serializedData;
}

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
template<class T>
bool LoadProto(
    const std::string hash,
    std::shared_ptr<T>& serialized)
{
    if (hash.empty()) { return false; }

    std::unique_lock<std::mutex> llock(location_lock_);
    bool attemptFirst = gc_running_ ? !alt_location_ : alt_location_;
    llock.unlock();

    std::string data;

    bool foundInPrimary = false;
    if (Load(hash, data, attemptFirst)) {
        serialized = std::make_shared<T>();
        serialized->ParseFromArray(data.c_str(), data.size());

        foundInPrimary = Verify(*serialized);
    }

    bool foundInSecondary = false;
    if (!foundInPrimary) {
        // try again in the other bucket
        if (Load(hash, data, !attemptFirst)) {
            serialized = std::make_shared<T>();
            serialized->ParseFromArray(data.c_str(), data.size());

            foundInSecondary = Verify(*serialized);
        }
    }

    return (foundInPrimary || foundInSecondary);
}

template<class T>
bool StoreProto(const T data)
{
    if (nullptr != digest_) {
        std::string plaintext = opentxs::ProtoAsString<T>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        return Store(
            key,
            plaintext,
            alt_location_);
    }
    return false;
}

private:
    static Storage* instance_pointer_;
    static const uint32_t GC_INTERVAL;

    std::thread* gc_thread_ = nullptr;
    // Regenerate in-memory indices by recursively loading index objects starting
    // from the root hash
    void Read();

    // Methods for updating index objects
    bool UpdateNymCreds(const std::string& id, const std::string& hash);
    bool UpdateCredentials(const std::string& id, const std::string& hash);
    bool UpdateNyms(const proto::StorageNym& nym);
    bool UpdateItems(const proto::StorageCredentials& creds);
    bool UpdateItems(const proto::StorageNymList& nyms);
    bool UpdateRoot(const proto::StorageItems& items);
    bool UpdateRoot(proto::StorageRoot& root, const std::string& gcroot);
    bool UpdateRoot();

    void CollectGarbage();
    bool MigrateKey(const std::string& key);
    void RunGC();

    Storage(Storage const&) = delete;
    Storage& operator=(Storage const&) = delete;

protected:
    const uint32_t HASH_TYPE = 2; // BTC160
    Digest digest_ = nullptr;

    std::mutex init_lock_; // controls access to Read() method
    std::mutex cred_lock_; // ensures atomic writes to credentials_
    std::mutex nym_lock_; // ensures atomic writes to nyms_
    std::mutex write_lock_; // ensure atomic writes
    std::mutex gc_lock_; // prevents multiple garbage collection threads
    std::mutex gc_check_lock_; // controls access to RunGC()
    std::mutex location_lock_; // ensures atomic updates of alt_location_

    std::string root_ = "";
    std::string items_ = "";
    bool alt_location_ = false;
    bool isLoaded_ = false;
    bool gc_running_ = false;
    bool gc_resume_ = false;
    int64_t last_gc_ = 0;

    std::map<std::string, std::string> credentials_{{}};
    std::map<std::string, std::string> nyms_{{}};

    Storage(const Digest& hash);
    virtual void Init(const Digest& hash);

    // Pure virtual functions for implementation by child classes
    virtual std::string LoadRoot() = 0;
    virtual bool StoreRoot(const std::string& hash) = 0;
    virtual bool Load(
        const std::string& key,
        std::string& value,
        const bool altLocation) = 0;
    virtual bool Store(
        const std::string& key,
        const std::string& value,
        const bool altLocation) = 0;

public:
    // Factory method for instantiating the singleton. param is a child
    // class-defined instantiation parameter.
    static Storage& Factory(
        const Digest& hash,
        const std::string& param = "");

    bool Load(
        const std::string id,
        std::shared_ptr<proto::Credential>& cred);
    bool Load(
        const std::string id,
        std::shared_ptr<proto::CredentialIndex>& cred);
    bool Store(const proto::Credential& data);
    bool Store(const proto::CredentialIndex& data);

    virtual void Cleanup();
    virtual ~Storage();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGE_HPP
