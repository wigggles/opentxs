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

#include <opentxs/storage/Storage.hpp>

#include <ctime>
#include <iostream>

#ifdef OT_STORAGE_FS
#include <opentxs/storage/StorageFS.hpp>
#elif defined OT_STORAGE_SQLITE
#include <opentxs/storage/StorageSqlite3.hpp>
#endif

namespace opentxs
{
Storage* Storage::instance_pointer_ = nullptr;

Storage::Storage(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random)
        : gc_interval_(config.gc_interval_)
{
    std::time_t time = std::time(nullptr);
    last_gc_ = static_cast<int64_t>(time);

    Init(hash, random);
}

void Storage::Init(const Digest& hash, const Random& random)
{
    digest_ = hash;
    random_ = random;
}

Storage& Storage::It(
    const Digest& hash,
    const Random& random,
    const StorageConfig& config)
{

    if (nullptr == instance_pointer_) {
#ifdef OT_STORAGE_FS
        instance_pointer_ = new StorageFS(config, hash, random);
#elif defined OT_STORAGE_SQLITE
        instance_pointer_ = new StorageSqlite3(config, hash, random);
#endif
    }

    assert(nullptr != instance_pointer_);

    return *instance_pointer_;
}

void Storage::Read()
{
    std::lock_guard<std::mutex> readLock(init_lock_);

    if (!isLoaded_) {
        isLoaded_ = true;

        root_hash_ = LoadRoot();

        if (root_hash_.empty()) { return; }

        std::shared_ptr<proto::StorageRoot> root;

        if (!LoadProto<proto::StorageRoot>(root_hash_, root)) { return; }

        items_ = root->items();
        current_bucket_ = root->altlocation();
        last_gc_ = root->lastgc();
        gc_resume_ = root->gc();
        old_gc_root_ = root->gcroot();

        std::shared_ptr<proto::StorageItems> items;

        if (!LoadProto<proto::StorageItems>(items_, items)) { return; }

        if (!items->creds().empty()) {
            std::shared_ptr<proto::StorageCredentials> creds;

            if (!LoadProto<proto::StorageCredentials>(items->creds(), creds)) {

                return;
            }

            for (auto& it : creds->cred()) {
                credentials_.insert(std::pair<std::string, std::string>(
                    it.itemid(),
                    it.hash()));
            }
        }

        if (!items->nyms().empty()) {
            std::shared_ptr<proto::StorageNymList> nyms;

            if (!LoadProto<proto::StorageNymList>(items->nyms(), nyms)) {
                return;
            }

            for (auto& it : nyms->nym()) {
                nyms_.insert(std::pair<std::string, std::string>(
                    it.itemid(),
                    it.hash()));
            }
        }
    }
}

bool Storage::UpdateNymCreds(const std::string& id, const std::string& hash)
{
    // Reuse existing object, since it may contain more than just creds
    if (!id.empty() && !hash.empty()) {
        std::shared_ptr<proto::StorageNym> nym;

        if (!LoadProto<proto::StorageNym>(id, nym)) {
            nym = std::make_shared<proto::StorageNym>();
            nym->set_version(1);
            nym->set_nymid(id);
        } else {
            nym->clear_credlist();
        }

        proto::StorageItemHash* item = nym->mutable_credlist();
        item->set_version(1);
        item->set_itemid(id);
        item->set_hash(hash);

        assert(Verify(*nym));

        if (StoreProto<proto::StorageNym>(*nym)) {
            return UpdateNyms(*nym);
        }
    }

    return false;
}

bool Storage::UpdateCredentials(const std::string& id, const std::string& hash)
{
    // Do not test for existing object - we always regenerate from scratch
    if (!id.empty() && !hash.empty()) {

        // Block reads while updating credential map
        std::unique_lock<std::mutex> credlock(cred_lock_);
        credentials_.insert(std::pair<std::string, std::string>(id, hash));
        proto::StorageCredentials credIndex;
        credIndex.set_version(1);
        for (auto& cred : credentials_) {
            if (!cred.first.empty() && !cred.second.empty()) {
                proto::StorageItemHash* item = credIndex.add_cred();
                item->set_version(1);
                item->set_itemid(cred.first);
                item->set_hash(cred.second);
            }
        }
        credlock.unlock();

        assert(Verify(credIndex));

        if (StoreProto<proto::StorageCredentials>(credIndex)) {
            return UpdateItems(credIndex);
        }
    }

    return false;
}

bool Storage::UpdateNyms(const proto::StorageNym& nym)
{
    // Do not test for existing object - we always regenerate from scratch
    if (digest_) {
        std::string id = nym.nymid();
        std::string plaintext = ProtoAsString<proto::StorageNym>(nym);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        // Block reads while updating nym map
        std::unique_lock<std::mutex> nymLock(nym_lock_);
        nyms_.insert(std::pair<std::string, std::string>(id, hash));
        proto::StorageNymList nymIndex;
        nymIndex.set_version(1);
        for (auto& nym : nyms_) {
            if (!nym.first.empty() && !nym.second.empty()) {
                proto::StorageItemHash* item = nymIndex.add_nym();
                item->set_version(1);
                item->set_itemid(nym.first);
                item->set_hash(nym.second);
            }
        }
        nymLock.unlock();

        assert(Verify(nymIndex));

        if (StoreProto<proto::StorageNymList>(nymIndex)) {
            return UpdateItems(nymIndex);
        }
    }

    return false;
}

bool Storage::UpdateItems(const proto::StorageCredentials& creds)
{
    // Reuse existing object, since it may contain more than just creds
    std::shared_ptr<proto::StorageItems> items;

    if (!LoadProto<proto::StorageItems>(items_, items)) {
        items = std::make_shared<proto::StorageItems>();
        items->set_version(1);
    } else {
        items->clear_creds();
    }

    if (digest_) {
        std::string plaintext = ProtoAsString<proto::StorageCredentials>(creds);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        items->set_creds(hash);

        assert(Verify(*items));

        if (StoreProto<proto::StorageItems>(*items)) {
            return UpdateRoot(*items);
        }
    }

    return false;
}

bool Storage::UpdateItems(const proto::StorageNymList& nyms)
{
    // Reuse existing object, since it may contain more than just nyms
    std::shared_ptr<proto::StorageItems> items;

    if (!LoadProto<proto::StorageItems>(items_, items)) {
        items = std::make_shared<proto::StorageItems>();
        items->set_version(1);
    } else {
        items->clear_nyms();
    }

    if (digest_) {
        std::string plaintext = ProtoAsString<proto::StorageNymList>(nyms);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        items->set_nyms(hash);

        assert(Verify(*items));

        if (StoreProto<proto::StorageItems>(*items)) {
            return UpdateRoot(*items);
        }
    }

    return false;
}

bool Storage::UpdateRoot(const proto::StorageItems& items)
{
    // Reuse existing object to preserve current settings
    std::shared_ptr<proto::StorageRoot> root;

    if (!LoadProto<proto::StorageRoot>(root_hash_, root)) {
        root = std::make_shared<proto::StorageRoot>();
        root->set_version(1);
        root->set_altlocation(false);
        std::time_t time = std::time(nullptr);
        root->set_lastgc(static_cast<int64_t>(time));
    } else {
        root->clear_items();
    }

    if (digest_) {
        std::string plaintext = ProtoAsString<proto::StorageItems>(items);
        std::string hash;
        digest_(Storage::HASH_TYPE, plaintext, hash);

        items_ = hash;

        root->set_version(1);
        root->set_items(hash);

        assert(Verify(*root));

        if (StoreProto<proto::StorageRoot>(*root)) {
            plaintext = ProtoAsString<proto::StorageRoot>(*root);
            digest_(Storage::HASH_TYPE, plaintext, hash);

            root_hash_ = hash;

            return StoreRoot(hash);
        }
    }

    return false;
}

// this version is for starting garbage collection only
bool Storage::UpdateRoot(
    proto::StorageRoot& root,
    const std::string& gcroot)
{
    if (digest_) {
        root.set_altlocation(current_bucket_);

        std::time_t time = std::time(nullptr);
        last_gc_ = static_cast<int64_t>(time);
        root.set_lastgc(last_gc_);
        root.set_gc(true);
        root.set_gcroot(gcroot);

        assert(Verify(root));

        if (StoreProto<proto::StorageRoot>(root)) {
            std::string hash;
            std::string plaintext = ProtoAsString<proto::StorageRoot>(root);
            digest_(Storage::HASH_TYPE, plaintext, hash);

            root_hash_ = hash;

            return StoreRoot(hash);
        }
    }

    return false;
}

// this version is for ending garbage collection only
bool Storage::UpdateRoot()
{
    std::shared_ptr<proto::StorageRoot> root;

    bool loaded = LoadProto<proto::StorageRoot>(root_hash_, root);

    assert(loaded);

    if (loaded && digest_) {
        gc_running_ = false;
        root->set_gc(false);

        assert(Verify(*root));

        if (StoreProto<proto::StorageRoot>(*root)) {
            std::string hash;
            std::string plaintext = ProtoAsString<proto::StorageRoot>(*root);
            digest_(Storage::HASH_TYPE, plaintext, hash);

            root_hash_ = hash;

            return StoreRoot(hash);
        }
    }

    return false;
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred)
{
    if (!isLoaded_) { Read(); }

    bool found = false;
    std::string hash;

    // block writes while searching credential map
    std::unique_lock<std::mutex> credlock(cred_lock_);
    auto it = credentials_.find(id);
    if (it != credentials_.end()) {
        found = true;
        hash = it->second;
    }
    credlock.unlock();

    if (found) { return LoadProto<proto::Credential>(hash, cred); }

    return false;
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::CredentialIndex>& credList)
{
    if (!isLoaded_) { Read(); }

    bool found = false;
    std::string nymHash;

    // block writes while searching nym map
    std::unique_lock<std::mutex> nymLock(nym_lock_);
    auto it = nyms_.find(id);
    if (it != nyms_.end()) {
        found = true;
        nymHash = it->second;
    }
    nymLock.unlock();

    if (found) {
        std::shared_ptr<proto::StorageNym> nym;

        if (LoadProto<proto::StorageNym>(nymHash, nym)) {
            std::string credListHash = nym->credlist().hash();

            if (LoadProto<proto::CredentialIndex>(credListHash, credList)) {

                return Verify(*credList);
            }
        }
    }

    return false;
}

bool Storage::Store(const proto::Credential& data)
{
    if (!isLoaded_) { Read(); }

    // Avoid overwriting private credentials with public credentials
    bool existingPrivate = false;
    std::shared_ptr<proto::Credential> existing;

    if (Load(data.id(), existing)) {
        existingPrivate = (proto::KEYMODE_PRIVATE == existing->mode());
    }

    if (existingPrivate && (proto::KEYMODE_PRIVATE != data.mode())) {
        std::cout << "Skipping update of existing private credential with "
                  << "non-private version." << std::endl;

        return true;
    }

    if (digest_) {
        std::string plaintext = ProtoAsString<proto::Credential>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        std::lock_guard<std::mutex> writeLock(write_lock_);
        bool savedCredential = Store(
            key,
            plaintext,
            current_bucket_);

        if (savedCredential) {
            return UpdateCredentials(data.id(), key);
        }
    }
    return false;
}

bool Storage::Store(const proto::CredentialIndex& data)
{
    if (!isLoaded_) { Read(); }

    if (digest_) {
        std::string plaintext = ProtoAsString<proto::CredentialIndex>(data);
        std::string key;
        digest_(Storage::HASH_TYPE, plaintext, key);

        std::lock_guard<std::mutex> writeLock(write_lock_);
        bool saved = Store(
            key,
            plaintext,
            current_bucket_);

        if (saved) {
            return UpdateNymCreds(data.nymid(), key);
        }
    }

    return false;
}

void Storage::CollectGarbage()
{
    std::unique_lock<std::mutex> llock(location_lock_);
    bool oldLocation = current_bucket_;
    current_bucket_ = !current_bucket_;
    llock.unlock();

    std::shared_ptr<proto::StorageRoot> root;
    std::string gcroot, gcitems;
    bool updated;

    if (!gc_resume_) {
        // Do not allow changes to root index object until we've updated it.
        std::unique_lock<std::mutex> writeLock(write_lock_);
        gcroot = root_hash_;

        if (!LoadProto<proto::StorageRoot>(root_hash_, root)) {
            // If there is no root object, then there's nothing to gc
            gc_running_ = false;
            return;
        }
        gcitems = root->items();
        updated = UpdateRoot(*root, gcroot);
        writeLock.unlock();
    } else {
        gcroot = old_gc_root_;

        if (!LoadProto<proto::StorageRoot>(old_gc_root_, root)) {
            // If this branch is reached, the data store is corrupted
            assert(false);
        }
        gcitems = root->items();
        updated = true;
        gc_resume_ = false;
    }

    if (!updated) {
        gc_running_ = false;
        return;
    }
    MigrateKey(gcitems);
    std::shared_ptr<proto::StorageItems> items;

    if (!LoadProto<proto::StorageItems>(gcitems, items)) {
        gc_running_ = false;
        return;
    }

    if (!items->creds().empty()) {
        MigrateKey(items->creds());
        std::shared_ptr<proto::StorageCredentials> creds;

        if (!LoadProto<proto::StorageCredentials>(items->creds(), creds)) {
            gc_running_ = false;
            return;
        }

        for (auto& it : creds->cred()) {
            MigrateKey(it.hash());
        }
    }

    if (!items->nyms().empty()) {
        MigrateKey(items->nyms());
        std::shared_ptr<proto::StorageNymList> nyms;

        if (!LoadProto<proto::StorageNymList>(items->nyms(), nyms)) {
            gc_running_ = false;
            return;
        }

        for (auto& it : nyms->nym()) {
            MigrateKey(it.hash());
            std::shared_ptr<proto::StorageNym> nym;

            if (!LoadProto<proto::StorageNym>(it.hash(), nym)) {
                gc_running_ = false;
                return;
            }

            MigrateKey(nym->credlist().hash());
        }
    }

    std::unique_lock<std::mutex> writeLock(write_lock_);
    UpdateRoot();
    writeLock.unlock();

    std::unique_lock<std::mutex> bucketLock(bucket_lock_);
    EmptyBucket(oldLocation);
    bucketLock.unlock();

    gc_running_ = false;
}

bool Storage::MigrateKey(const std::string& key)
{
    std::string value;

    // try to load the key from the inactive bucket
    if (Load(key, value, !current_bucket_)) {

        // save to the active bucket
        if (Store(key, value, current_bucket_)) {
            return true;
        } else {
            return false;
        }
    }

    return true; // the key must have already been in the active bucket
}

void Storage::RunGC()
{
    if (!isLoaded_) { return; }

    std::lock_guard<std::mutex> gclock(gc_lock_);
    std::time_t time = std::time(nullptr);
    const bool intervalExceeded =
        ((time - last_gc_) > gc_interval_);

    if (!gc_running_ && ( gc_resume_ || intervalExceeded)) {
        assert (!gc_running_);
        gc_running_ = true;
        gc_thread_ = new std::thread(&Storage::CollectGarbage, this);
    }
}

void Storage::Storage::Cleanup_Storage()
{
    if ((nullptr != gc_thread_) && gc_thread_->joinable()) {
        gc_thread_->join();
        delete gc_thread_;
    }
}

void Storage::Storage::Cleanup()
{
    Cleanup_Storage();
}

Storage::~Storage()
{
    Cleanup_Storage();
}


} // namespace opentxs
