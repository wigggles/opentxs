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

#include "opentxs/storage/Storage.hpp"

#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif  // ANDROID
#include "opentxs/core/Log.hpp"
#include "opentxs/storage/tree/Credentials.hpp"
#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/tree/Nyms.hpp"
#include "opentxs/storage/tree/PeerReplies.hpp"
#include "opentxs/storage/tree/PeerRequests.hpp"
#include "opentxs/storage/tree/Seeds.hpp"
#include "opentxs/storage/tree/Servers.hpp"
#include "opentxs/storage/tree/Thread.hpp"
#include "opentxs/storage/tree/Threads.hpp"
#include "opentxs/storage/tree/Tree.hpp"
#include "opentxs/storage/tree/Units.hpp"

#include <assert.h>
#include <stdint.h>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace opentxs
{
Storage::Storage(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random)
    : gc_interval_(config.gc_interval_)
    , config_(config)
    , digest_(hash)
    , random_(random)
{
    std::time_t time = std::time(nullptr);
    last_gc_ = static_cast<std::int64_t>(time);

    Init();
}

void Storage::Cleanup_Storage()
{
    if (gc_thread_) {
        if (gc_thread_->joinable()) {
            gc_thread_->join();
        }

        gc_thread_.reset();
    }
}

void Storage::Cleanup()
{
    shutdown_.store(true);

    Cleanup_Storage();
}

void Storage::CollectGarbage()
{
    const bool resume = gc_resume_.exchange(false);
    std::unique_lock<std::mutex> lock(write_lock_);

    bool oldLocation = false;

    if (resume) {
        oldLocation = !current_bucket_.load();
    } else {
        gc_root_ = tree_->Root();
        oldLocation = current_bucket_.load();
        current_bucket_.store(!oldLocation);
        save(lock);
    }

    lock.unlock();
    bool success = false;

    if (!gc_root_.empty()) {
        std::function<bool(const std::string&)> migrate =
            std::bind(&Storage::MigrateKey, this, std::placeholders::_1);
        const storage::Tree tree(*this, migrate, gc_root_, 0);
        success = tree.Migrate();
    }

    if (success) {
        EmptyBucket(oldLocation);
    } else {
        std::cerr << __FUNCTION__ << ": Garbage collection failed. "
                  << "Will retry next cycle." << std::endl;
    }

    std::unique_lock<std::mutex> gcLock(gc_lock_, std::defer_lock);
    std::lock(gcLock, lock);
    gc_running_.store(false);
    gc_root_ = "";
    last_gc_ = static_cast<std::int64_t>(std::time(nullptr));
    save(lock);
    lock.unlock();
    gcLock.unlock();
}

ObjectList Storage::ContextList(const std::string& nymID) {

    return tree_->NymNode().Nym(nymID).Contexts().List();
}

std::string Storage::DefaultSeed() { return tree_->SeedNode().Default(); }

void Storage::Init()
{
    current_bucket_.store(false);
    shutdown_.store(false);
    gc_running_.store(false);
    gc_resume_.store(false);
}

bool Storage::Load(
    const std::string& nym,
    const std::string& id,
    std::shared_ptr<proto::Context>& context,
    const bool checking)
{
    std::string notUsed;

    return tree_->NymNode().Nym(nym).Contexts()
        .Load(id, context, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred,
    const bool checking)
{
    return tree_->CredentialNode().Load(id, cred, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::CredentialIndex>& nym,
    const bool checking)
{
    std::string notUsed;

    return Load(id, nym, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::CredentialIndex>& nym,
    std::string& alias,
    const bool checking)
{
    return tree_->NymNode().Nym(id).Load(nym, alias, checking);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::string& output,
    std::string& alias,
    const bool checking)
{
    switch (box) {
        case StorageBox::MAILINBOX: {
            return tree_->NymNode().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case StorageBox::MAILOUTBOX: {
            return tree_->NymNode().Nym(nymID).MailOutbox().Load(
                id, output, alias, checking);
        }
        default: {
            return false;
        }
    }
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::shared_ptr<proto::PeerReply>& reply,
    const bool checking)
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return tree_->NymNode().Nym(nymID).SentReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return tree_->NymNode().Nym(nymID).IncomingReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return tree_->NymNode().Nym(nymID).FinishedReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return tree_->NymNode().Nym(nymID).ProcessedReplyBox().Load(
                id, reply, checking);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box,
    std::shared_ptr<proto::PeerRequest>& request,
    std::time_t& time,
    const bool checking)
{
    bool output = false;
    std::string alias;

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            output = tree_->NymNode().Nym(nymID).SentRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            output = tree_->NymNode().Nym(nymID).IncomingRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            output = tree_->NymNode().Nym(nymID).FinishedRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            output = tree_->NymNode().Nym(nymID).ProcessedRequestBox().Load(
                id, request, alias, checking);
        } break;
        default: { }
    }

    if (output) {
        try {
            time = std::stoi(alias);
        } catch (std::invalid_argument) {
            time = 0;
        } catch (std::out_of_range) {
            time = 0;
        }
    }

    return output;
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& seed,
    const bool checking)
{
    std::string notUsed;

    return Load(id, seed, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Seed>& seed,
    std::string& alias,
    const bool checking)
{
    return tree_->SeedNode().Load(id, seed, alias, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::ServerContract>& contract,
    const bool checking)
{
    std::string notUsed;

    return Load(id, contract, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::ServerContract>& contract,
    std::string& alias,
    const bool checking)
{
    return tree_->ServerNode().Load(id, contract, alias, checking);
}

bool Storage::Load(
    const std::string& nymId,
    const std::string& threadId,
    std::shared_ptr<proto::StorageThread>& thread)
{
    const bool exists = tree_->NymNode().Nym(nymId).Threads().Exists(threadId);

    if (!exists) { return false; }

    thread.reset(new proto::StorageThread);

    if (!thread) { return false; }

    *thread = tree_->NymNode().Nym(nymId).Threads().Thread(threadId).Items();

    return bool(thread);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::UnitDefinition>& contract,
    const bool checking)
{
    std::string notUsed;

    return Load(id, contract, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::UnitDefinition>& contract,
    std::string& alias,
    const bool checking)
{
    return tree_->UnitNode().Load(id, contract, alias, checking);
}

bool Storage::LoadRaw(
    const std::string& hash,
    std::string& output,
    const bool checking) const
{
    if (hash.empty()) {
        if (!checking) {
            std::cout << "Error:: Tried to load empty key" << std::endl;
        }

        return false;
    }

    bool valid = false;
    const bool bucket = current_bucket_.load();

    if (Load(hash, output, bucket)) {
        valid = 0 < output.size();
    }

    if (!valid) {
        // try again in the other bucket
        if (Load(hash, output, !bucket)) {
            valid = 0 < output.size();
        } else {
            // just in case...
            if (Load(hash, output, bucket)) {
                valid = 0 < output.size();
            }
        }
    }

    if (!valid && !checking) {
        std::cerr << "Specified object is not found." << std::endl
                  << "Hash: " << hash << std::endl
                  << "Size: " << output.size() << std::endl;
    }

    return valid;
}

// Applies a lambda to all public nyms in the database in a detached thread.
void Storage::MapPublicNyms(NymLambda& lambda)
{
    std::thread bgMap(&Storage::RunMapPublicNyms, this, lambda);
    bgMap.detach();
}

// Applies a lambda to all server contracts in the database in a detached
// thread.
void Storage::MapServers(ServerLambda& lambda)
{
    std::thread bgMap(&Storage::RunMapServers, this, lambda);
    bgMap.detach();
}

// Applies a lambda to all unit definitions in the database in a detached
// thread.
void Storage::MapUnitDefinitions(UnitLambda& lambda)
{
    std::thread bgMap(&Storage::RunMapUnits, this, lambda);
    bgMap.detach();
}

bool Storage::MigrateKey(const std::string& key) const
{
    assert(!key.empty());

    std::string value;

    // try to load the key from the inactive bucket
    if (Load(key, value, !(current_bucket_.load()))) {

        // save to the active bucket
        if (Store(key, value, current_bucket_.load())) {
            return true;
        } else {
            std::cerr << __FUNCTION__ << ": Save failure." << std::endl;
            abort();
        }
    }

    // If the key is not in the inactive bucket, it should be in the active
    // bucket
    const bool exists = Load(key, value, current_bucket_.load());

    if (!exists) {
        std::cerr << __FUNCTION__ << ": Missing key (" << key
                  << "). Database is corrupt." << std::endl;
        abort();
    }

    return true;
}

ObjectList Storage::NymBoxList(const std::string& nymID, const StorageBox box)
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentRequestBox().It()
                .List();
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingRequestBox().It()
                .List();
        } break;
        case StorageBox::SENTPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentReplyBox().It()
                .List();
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingReplyBox().It()
                .List();
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedRequestBox().It()
                .List();
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedReplyBox().It()
                .List();
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedRequestBox().It()
                .List();
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedReplyBox().It()
                .List();
        } break;
        case StorageBox::MAILINBOX: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_MailInbox().It()
                .List();
        }
        case StorageBox::MAILOUTBOX: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_MailOutbox().It()
                .List();
        }
        default: {
            return {};
        }
    }
}

ObjectList Storage::NymList() const { return tree_->NymNode().List(); }

void Storage::read_root()
{
    root_hash_ = LoadRoot();

    if (!root_hash_.empty()) {
        std::shared_ptr<proto::StorageRoot> root;

        if (!LoadProto(root_hash_, root)) {
            std::cerr << __FUNCTION__ << ": Failed to load root object file."
                      << std::endl;
            abort();
        }

        version_ = root->version();

        // Upgrade to version 2
        if (2 > version_) {
            version_ = 2;
        }

        items_ = root->items();
        current_bucket_.store(root->altlocation());
        last_gc_ = root->lastgc();
        gc_resume_.store(root->gc());
        gc_root_ = root->gcroot();
        sequence_ = root->sequence();
    } else {
        version_ = 2;
        items_ = "";
        current_bucket_.store(false);
        last_gc_ = static_cast<std::int64_t>(std::time(nullptr));
        gc_running_.store(false);
        gc_resume_.store(false);
    }

    std::function<bool(const std::string&)> migrate =
        std::bind(&Storage::MigrateKey, this, std::placeholders::_1);
    tree_.reset(new storage::Tree(*this, migrate, items_, sequence_));

    OT_ASSERT(tree_);
}

bool Storage::RemoveNymBoxItem(
    const std::string& nymID,
    const StorageBox box,
    const std::string& itemID)
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentRequestBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingRequestBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::SENTPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentReplyBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingReplyBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedRequestBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedReplyBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedRequestBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedReplyBox().It()
                .Delete(itemID);
        } break;
        case StorageBox::MAILINBOX: {
            const bool foundInThread = tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_Threads().It()
                .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = tree().It()
                    .mutable_Nyms().It()
                    .mutable_Nym(nymID).It()
                    .mutable_MailInbox().It()
                    .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        case StorageBox::MAILOUTBOX: {
            const bool foundInThread = tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_Threads().It()
                .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = tree().It()
                    .mutable_Nyms().It()
                    .mutable_Nym(nymID).It()
                    .mutable_MailOutbox().It()
                    .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        default: {
            return false;
        }
    }
}

bool Storage::RemoveServer(const std::string& id)
{
    return tree().It().mutable_Servers().It().Delete(id);
}

bool Storage::RemoveUnitDefinition(const std::string& id)
{
    return tree().It().mutable_Units().It().Delete(id);
}

void Storage::RunGC()
{
    if (shutdown_.load()) {
        return;
    }

    std::lock_guard<std::mutex> gclock(gc_lock_);
    const std::time_t time = std::time(nullptr);
    const bool intervalExceeded = ((time - last_gc_) > gc_interval_);
    const bool resume = gc_resume_.load();
    const bool needToCollectGarbage = resume || intervalExceeded;

    if (needToCollectGarbage) {
        const bool running = gc_running_.exchange(true);

        if (!running) {
            Cleanup_Storage();
            gc_thread_.reset(new std::thread(&Storage::CollectGarbage, this));
        }
    }
}

void Storage::RunMapPublicNyms(NymLambda lambda)
{
    return tree_->NymNode().Map(lambda);
}

void Storage::RunMapServers(ServerLambda lambda)
{
    return tree_->ServerNode().Map(lambda);
}

void Storage::RunMapUnits(UnitLambda lambda)
{
    return tree_->UnitNode().Map(lambda);
}

void Storage::save(const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (!StoreProto(serialize(), root_hash_)) {
        std::cerr << __FUNCTION__ << ": Save error." << std::endl;
        abort();
    }

    if (!StoreRoot(root_hash_)) {
        std::cerr << __FUNCTION__ << ": Root hash update error." << std::endl;
        abort();
    }
}

void Storage::save(storage::Tree* in, const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }
    if (nullptr == in) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    items_ = in->Root();
    sequence_ = in->Sequence();

    save(lock);
}

proto::StorageRoot Storage::serialize() const
{
    proto::StorageRoot output;
    output.set_version(version_);
    output.set_items(items_);
    output.set_altlocation(current_bucket_.load());
    output.set_lastgc(last_gc_);
    output.set_gc(gc_running_.load());
    output.set_gcroot(gc_root_);
    output.set_sequence(sequence_);
    std::cerr << sequence_ << std::endl;

    return output;
}

bool Storage::SetDefaultSeed(const std::string& id)
{
    return tree().It().mutable_Seeds().It().SetDefault(id);
}

bool Storage::SetNymAlias(const std::string& id, const std::string& alias)
{
    return tree().It().mutable_Nyms().It().mutable_Nym(id).It().SetAlias(alias);
}

bool Storage::SetPeerRequestTime(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box)
{
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentRequestBox().It()
                .SetAlias(id, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingRequestBox().It()
                .SetAlias(id, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedRequestBox().It()
                .SetAlias(id, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedRequestBox().It()
                .SetAlias(id, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::SetSeedAlias(const std::string& id, const std::string& alias)
{
    return tree().It().mutable_Seeds().It().SetAlias(id, alias);
}

bool Storage::SetServerAlias(const std::string& id, const std::string& alias)
{
    return tree().It().mutable_Servers().It().SetAlias(id, alias);
}

bool Storage::SetThreadAlias(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& alias)
{
    return tree().It()
        .mutable_Nyms().It()
        .mutable_Nym(nymId).It()
        .mutable_Threads().It()
        .mutable_Thread(threadId).It()
        .SetAlias(alias);
}

bool Storage::SetUnitDefinitionAlias(
    const std::string& id,
    const std::string& alias)
{
    return tree().It().mutable_Units().It().SetAlias(id, alias);
}

std::string Storage::ServerAlias(const std::string& id)
{
    return tree_->ServerNode().Alias(id);
}

ObjectList Storage::ServerList() { return tree_->ServerNode().List(); }

bool Storage::Store(const proto::Context& data)
{
    std::string notUsed;

    return tree().It()
        .mutable_Nyms().It()
        .mutable_Nym(data.localnym()).It()
        .mutable_Contexts().It()
        .Store(data, notUsed);
}

bool Storage::Store(const proto::Credential& data)
{
    std::string notUsed;

    return tree().It().mutable_Credentials().It().Store(data, notUsed);
}

bool Storage::Store(
    const proto::CredentialIndex& data,
    const std::string& alias)
{
    std::string plaintext;
    const bool saved = tree()
                           .It()
                           .mutable_Nyms()
                           .It()
                           .mutable_Nym(data.nymid())
                           .It()
                           .Store(data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_nyms_ && config_.dht_callback_) {
            config_.dht_callback_(data.nymid(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::Store(
    const std::string& nymid,
    const std::string& threadid,
    const std::string& itemid,
    const std::uint64_t time,
    const std::string& alias,
    const std::string& data,
    const StorageBox box)
{
    return tree().It()
        .mutable_Nyms().It()
        .mutable_Nym(nymid).It()
        .mutable_Threads().It()
        .mutable_Thread(threadid).It()
        .Add(itemid, time, box, alias,data);
}

bool Storage::Store(
    const proto::PeerReply& data,
    const std::string& nymID,
    const StorageBox box)
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentReplyBox().It()
                .Store(data);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingReplyBox().It()
                .Store(data);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedReplyBox().It()
                .Store(data);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedReplyBox().It()
                .Store(data);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Store(
    const proto::PeerRequest& data,
    const std::string& nymID,
    const StorageBox box)
{
    // Use the alias field to store the time at which the request was saved.
    // Useful for managing retry logic in the high level api
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_SentRequestBox().It()
                .Store(data, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_IncomingRequestBox().It()
                .Store(data, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_FinishedRequestBox().It()
                .Store(data, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return tree().It()
                .mutable_Nyms().It()
                .mutable_Nym(nymID).It()
                .mutable_ProcessedRequestBox().It()
                .Store(data, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Store(const proto::Seed& data, const std::string& alias)
{
    return tree().It().mutable_Seeds().It().Store(data, alias);
}

bool Storage::Store(const proto::ServerContract& data, const std::string& alias)
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        tree().It().mutable_Servers().It().Store(data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_servers_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::Store(const proto::UnitDefinition& data, const std::string& alias)
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        tree().It().mutable_Units().It().Store(data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_units_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::StoreRaw(const std::string& data, std::string& key) const
{
    if (digest_) {
        digest_(Storage::HASH_TYPE, data, key);

        return Store(key, data, current_bucket_.load());
    }

    return false;
}

ObjectList Storage::ThreadList(const std::string& nymID)
{
    return tree_->NymNode().Nym(nymID).Threads().List();
}

std::string Storage::ThreadAlias(
    const std::string& nymID,
    const std::string& threadID)
{
    return tree_->NymNode().Nym(nymID).Threads().Thread(threadID).Alias();
}

Editor<storage::Tree> Storage::tree()
{
    std::function<void(storage::Tree*, std::unique_lock<std::mutex>&)>
        callback =
            [&](storage::Tree* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock);
    };

    return Editor<storage::Tree>(write_lock_, tree_.get(), callback);
}

std::string Storage::UnitDefinitionAlias(const std::string& id)
{
    return tree_->UnitNode().Alias(id);
}

ObjectList Storage::UnitDefinitionList() { return tree_->UnitNode().List(); }

bool Storage::verify_write_lock(const std::unique_lock<std::mutex>& lock)
{
    if (lock.mutex() != &write_lock_) {
        std::cerr << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        std::cerr << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

Storage::~Storage() { Cleanup_Storage(); }
}  // namespace opentxs
