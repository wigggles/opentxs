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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/storage/Storage.hpp"

#if OT_STORAGE_FS
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#endif
#include "opentxs/core/Log.hpp"
#include "opentxs/interface/storage/StoragePlugin.hpp"
#if OT_STORAGE_FS
#include "opentxs/storage/drivers/StorageFS.hpp"
#include "opentxs/storage/drivers/StorageFSArchive.hpp"
#endif
#if OT_STORAGE_SQLITE
#include "opentxs/storage/drivers/StorageSqlite3.hpp"
#endif
#include "opentxs/storage/tree/BlockchainTransactions.hpp"
#include "opentxs/storage/tree/Contacts.hpp"
#include "opentxs/storage/tree/Credentials.hpp"
#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/tree/Nyms.hpp"
#include "opentxs/storage/tree/PeerReplies.hpp"
#include "opentxs/storage/tree/PeerRequests.hpp"
#include "opentxs/storage/tree/Root.hpp"
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

#define OT_METHOD "opentxs::Storage::"

namespace opentxs
{
const std::uint32_t Storage::HASH_TYPE = 2;  // BTC160

Storage::Storage(
    const StorageConfig& config,
    CryptoEngine& crypto,
    const Digest& hash,
    const Random& random)
    : crypto_(crypto)
    , gc_interval_(config.gc_interval_)
    , config_(config)
    , digest_(hash)
    , random_(random)
{
    if (OT_STORAGE_PRIMARY_PLUGIN_SQLITE == config_.primary_plugin_) {
#if OT_STORAGE_SQLITE
        primary_plugin_.reset(
            new StorageSqlite3(config_, digest_, random_, primary_bucket_));
#endif
    } else if (OT_STORAGE_PRIMARY_PLUGIN_FS == config_.primary_plugin_) {
#if OT_STORAGE_FS
        primary_plugin_.reset(
            new StorageFS(config_, digest_, random_, primary_bucket_));
#endif
    }

    OT_ASSERT(primary_plugin_);

    Init();
}

std::set<std::string> Storage::BlockchainAccountList(
    const std::string& nymID,
    const proto::ContactItemType type)
{
    return Meta().Tree().NymNode().Nym(nymID).BlockchainAccountList(type);
}

std::string Storage::BlockchainAddressOwner(
    proto::ContactItemType chain,
    std::string address)
{
    return Meta().Tree().ContactNode().AddressOwner(chain, address);
}

ObjectList Storage::BlockchainTransactionList()
{

    return Meta().Tree().BlockchainNode().List();
}

void Storage::Cleanup_Storage()
{
    for (auto& thread : background_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (meta_) {
        meta_->cleanup();
    }
}

void Storage::Cleanup()
{
    shutdown_.store(true);

    Cleanup_Storage();
}

void Storage::CollectGarbage() { Meta().Migrate(*primary_plugin_); }

std::string Storage::ContactAlias(const std::string& id)
{
    return Meta().Tree().ContactNode().Alias(id);
}

ObjectList Storage::ContactList() { return Meta().Tree().ContactNode().List(); }

ObjectList Storage::ContextList(const std::string& nymID)
{

    return Meta().Tree().NymNode().Nym(nymID).Contexts().List();
}

bool Storage::CreateThread(
    const std::string& nymID,
    const std::string& threadID,
    const std::set<std::string>& participants)
{
    const auto id = mutable_Meta()
                        .It()
                        .mutable_Tree()
                        .It()
                        .mutable_Nyms()
                        .It()
                        .mutable_Nym(nymID)
                        .It()
                        .mutable_Threads()
                        .It()
                        .Create(threadID, participants);

    return (false == id.empty());
}

std::string Storage::DefaultSeed()
{
    return Meta().Tree().SeedNode().Default();
}

bool Storage::EmptyBucket(const bool bucket) const
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->EmptyBucket(bucket);
    }

    return primary_plugin_->EmptyBucket(bucket);
}

void Storage::Init()
{
    shutdown_.store(false);
    InitPlugins();
}

void Storage::InitBackup()
{
    if (config_.fs_backup_directory_.empty()) {

        return;
    }

#if OT_STORAGE_FS
    std::unique_ptr<SymmetricKey> null(nullptr);
    backup_plugins_.emplace_back(new StorageFSArchive(
        config_,
        digest_,
        random_,
        primary_bucket_,
        config_.fs_backup_directory_,
        null));
#else
    return;
#endif
}

void Storage::InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key)
{
    if (config_.fs_encrypted_backup_directory_.empty()) {

        return;
    }

#if OT_STORAGE_FS
    backup_plugins_.emplace_back(new StorageFSArchive(
        config_,
        digest_,
        random_,
        primary_bucket_,
        config_.fs_encrypted_backup_directory_,
        key));
#else
    return;
#endif
}

void Storage::InitPlugins()
{
    synchronize_root();
    synchronize_plugins();
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& accountID,
    std::shared_ptr<proto::Bip44Account>& output,
    const bool checking)
{
    return Meta().Tree().NymNode().Nym(nymID).Load(accountID, output, checking);
}

bool Storage::Load(
    const std::string& key,
    const bool checking,
    std::string& value) const
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Load(key, checking, value)) {

        return true;
    }

    if (false == checking) {
        otErr << OT_METHOD << __FUNCTION__
              << ": key not found by primary storage plugin." << std::endl;
    }

    std::size_t count{0};

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Load(key, checking, value)) {
            auto notUsed = key;
            primary_plugin_->Store(value, notUsed);

            return true;
        }

        if (false == checking) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": key not found by backup storage plugin " << count
                  << std::endl;
        }

        ++count;
    }

    if (false == checking) {
        otErr << OT_METHOD << __FUNCTION__ << ": key not found by any plugin."
              << std::endl;

        throw std::runtime_error("");
    }

    return false;
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::BlockchainTransaction>& transaction,
    const bool checking)
{
    return Meta().Tree().BlockchainNode().Load(id, transaction, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& contact,
    const bool checking)
{
    std::string notUsed{};

    return Load(id, contact, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& contact,
    std::string& alias,
    const bool checking)
{
    return Meta().Tree().ContactNode().Load(id, contact, alias, checking);
}

bool Storage::Load(
    const std::string& nym,
    const std::string& id,
    std::shared_ptr<proto::Context>& context,
    const bool checking)
{
    std::string notUsed;

    return Meta().Tree().NymNode().Nym(nym).Contexts().Load(
        id, context, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred,
    const bool checking)
{
    return Meta().Tree().CredentialNode().Load(id, cred, checking);
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
    return Meta().Tree().NymNode().Nym(id).Load(nym, alias, checking);
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
            return Meta().Tree().NymNode().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case StorageBox::MAILOUTBOX: {
            return Meta().Tree().NymNode().Nym(nymID).MailOutbox().Load(
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
            return Meta().Tree().NymNode().Nym(nymID).SentReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).IncomingReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).FinishedReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).ProcessedReplyBox().Load(
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
            output = Meta().Tree().NymNode().Nym(nymID).SentRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            output =
                Meta().Tree().NymNode().Nym(nymID).IncomingRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            output =
                Meta().Tree().NymNode().Nym(nymID).FinishedRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            output =
                Meta().Tree().NymNode().Nym(nymID).ProcessedRequestBox().Load(
                    id, request, alias, checking);
        } break;
        default: {
        }
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
    return Meta().Tree().SeedNode().Load(id, seed, alias, checking);
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
    return Meta().Tree().ServerNode().Load(id, contract, alias, checking);
}

bool Storage::Load(
    const std::string& nymId,
    const std::string& threadId,
    std::shared_ptr<proto::StorageThread>& thread)
{
    const bool exists =
        Meta().Tree().NymNode().Nym(nymId).Threads().Exists(threadId);

    if (!exists) {
        return false;
    }

    thread.reset(new proto::StorageThread);

    if (!thread) {
        return false;
    }

    *thread =
        Meta().Tree().NymNode().Nym(nymId).Threads().Thread(threadId).Items();

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
    return Meta().Tree().UnitNode().Load(id, contract, alias, checking);
}

bool Storage::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->LoadFromBucket(key, value, bucket)) {

        return true;
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->LoadFromBucket(key, value, bucket)) {

            return true;
        }
    }

    return false;
}

std::string Storage::LoadRoot() const
{
    OT_ASSERT(primary_plugin_);

    std::string root = primary_plugin_->LoadRoot();

    if (false == root.empty()) {

        return root;
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        root = plugin->LoadRoot();

        if (false == root.empty()) {

            return root;
        }
    }

    return root;
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

bool Storage::Migrate(const std::string& key, const StorageDriver& to) const
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Migrate(key, to)) {

        return true;
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Migrate(key, to)) {

            return true;
        }
    }

    return false;
}

bool Storage::MoveThreadItem(
    const std::string& nymId,
    const std::string& fromThreadID,
    const std::string& toThreadID,
    const std::string& itemID)
{
    const bool fromExists =
        Meta().Tree().NymNode().Nym(nymId).Threads().Exists(fromThreadID);

    if (false == fromExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": From thread does not exist."
              << std::endl;

        return false;
    }

    const bool toExists =
        Meta().Tree().NymNode().Nym(nymId).Threads().Exists(toThreadID);

    if (false == toExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": To thread does not exist."
              << std::endl;

        return false;
    }

    auto& fromThread = mutable_Meta()
                           .It()
                           .mutable_Tree()
                           .It()
                           .mutable_Nyms()
                           .It()
                           .mutable_Nym(nymId)
                           .It()
                           .mutable_Threads()
                           .It()
                           .mutable_Thread(fromThreadID)
                           .It();
    const auto thread = fromThread.Items();
    bool found = false;
    std::uint64_t time{};
    StorageBox box{};

    for (const auto& item : thread.item()) {
        if (item.id() == itemID) {
            found = true;
            time = item.time();
            box = static_cast<StorageBox>(item.box());

            break;
        }
    }

    if (false == found) {
        otErr << OT_METHOD << __FUNCTION__ << ": Item does not exist."
              << std::endl;

        return false;
    }

    if (false == fromThread.Remove(itemID)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to remove item."
              << std::endl;

        return false;
    }

    auto& toThread = mutable_Meta()
                         .It()
                         .mutable_Tree()
                         .It()
                         .mutable_Nyms()
                         .It()
                         .mutable_Nym(nymId)
                         .It()
                         .mutable_Threads()
                         .It()
                         .mutable_Thread(toThreadID)
                         .It();

    if (false == toThread.Add(itemID, time, box, {}, {})) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to insert item."
              << std::endl;

        return false;
    }

    return true;
}

Editor<storage::Root> Storage::mutable_Meta()
{
    std::function<void(storage::Root*, Lock&)> callback =
        [&](storage::Root* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<storage::Root>(write_lock_, meta(), callback);
}

ObjectList Storage::NymBoxList(const std::string& nymID, const StorageBox box)
    const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return Meta().Tree().NymNode().Nym(nymID).SentRequestBox().List();
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return Meta()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .IncomingRequestBox()
                .List();
        } break;
        case StorageBox::SENTPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).SentReplyBox().List();
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).IncomingReplyBox().List();
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return Meta()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .FinishedRequestBox()
                .List();
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Meta().Tree().NymNode().Nym(nymID).FinishedReplyBox().List();
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return Meta()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedRequestBox()
                .List();
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Meta()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedReplyBox()
                .List();
        } break;
        case StorageBox::MAILINBOX: {
            return Meta().Tree().NymNode().Nym(nymID).MailInbox().List();
        }
        case StorageBox::MAILOUTBOX: {
            return Meta().Tree().NymNode().Nym(nymID).MailOutbox().List();
        }
        default: {
            return {};
        }
    }
}

ObjectList Storage::NymList() const { return Meta().Tree().NymNode().List(); }

storage::Root* Storage::meta() const
{
    Lock lock(write_lock_);

    if (!meta_) {
        meta_.reset(new storage::Root(
            *this, LoadRoot(), gc_interval_, primary_bucket_));
    }

    OT_ASSERT(meta_);

    lock.unlock();

    return meta_.get();
}

const storage::Root& Storage::Meta() const { return *meta(); }

bool Storage::RemoveNymBoxItem(
    const std::string& nymID,
    const StorageBox box,
    const std::string& itemID)
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::SENTPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedReplyBox()
                .It()
                .Delete(itemID);
        } break;
        case StorageBox::MAILINBOX: {
            const bool foundInThread = mutable_Meta()
                                           .It()
                                           .mutable_Tree()
                                           .It()
                                           .mutable_Nyms()
                                           .It()
                                           .mutable_Nym(nymID)
                                           .It()
                                           .mutable_Threads()
                                           .It()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Meta()
                                 .It()
                                 .mutable_Tree()
                                 .It()
                                 .mutable_Nyms()
                                 .It()
                                 .mutable_Nym(nymID)
                                 .It()
                                 .mutable_MailInbox()
                                 .It()
                                 .Delete(itemID);
            }

            return foundInThread || foundInBox;
        }
        case StorageBox::MAILOUTBOX: {
            const bool foundInThread = mutable_Meta()
                                           .It()
                                           .mutable_Tree()
                                           .It()
                                           .mutable_Nyms()
                                           .It()
                                           .mutable_Nym(nymID)
                                           .It()
                                           .mutable_Threads()
                                           .It()
                                           .FindAndDeleteItem(itemID);
            bool foundInBox = false;

            if (!foundInThread) {
                foundInBox = mutable_Meta()
                                 .It()
                                 .mutable_Tree()
                                 .It()
                                 .mutable_Nyms()
                                 .It()
                                 .mutable_Nym(nymID)
                                 .It()
                                 .mutable_MailOutbox()
                                 .It()
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
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Servers()
        .It()
        .Delete(id);
}

bool Storage::RemoveUnitDefinition(const std::string& id)
{
    return mutable_Meta().It().mutable_Tree().It().mutable_Units().It().Delete(
        id);
}

bool Storage::RenameThread(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& newID)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymId)
        .It()
        .mutable_Threads()
        .It()
        .Rename(threadId, newID);
}

void Storage::RunGC()
{
    if (shutdown_.load()) {
        return;
    }

    CollectGarbage();
}

void Storage::RunMapPublicNyms(NymLambda lambda)
{
    return Meta().Tree().NymNode().Map(lambda);
}

void Storage::RunMapServers(ServerLambda lambda)
{
    return Meta().Tree().ServerNode().Map(lambda);
}

void Storage::RunMapUnits(UnitLambda lambda)
{
    return Meta().Tree().UnitNode().Map(lambda);
}

void Storage::save(storage::Root* in, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nullptr != in);

    StoreRoot(in->root_);
}

bool Storage::SetContactAlias(const std::string& id, const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetDefaultSeed(const std::string& id)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetDefault(id);
}

bool Storage::SetNymAlias(const std::string& id, const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(id)
        .It()
        .SetAlias(alias);
}

bool Storage::SetPeerRequestTime(
    const std::string& nymID,
    const std::string& id,
    const StorageBox box)
{
    const std::string now = std::to_string(time(nullptr));

    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .SetAlias(id, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::SetSeedAlias(const std::string& id, const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetServerAlias(const std::string& id, const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Servers()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetThreadAlias(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymId)
        .It()
        .mutable_Threads()
        .It()
        .mutable_Thread(threadId)
        .It()
        .SetAlias(alias);
}

bool Storage::SetUnitDefinitionAlias(
    const std::string& id,
    const std::string& alias)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Units()
        .It()
        .SetAlias(id, alias);
}

std::string Storage::ServerAlias(const std::string& id)
{
    return Meta().Tree().ServerNode().Alias(id);
}

ObjectList Storage::ServerList() const
{
    return Meta().Tree().ServerNode().List();
}

bool Storage::Store(
    const std::string& key,
    const std::string& value,
    const bool bucket) const
{
    OT_ASSERT(primary_plugin_);

    bool output = primary_plugin_->Store(key, value, bucket);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        output |= plugin->Store(key, value, bucket);
    }

    return output;
}

bool Storage::Store(const std::string& key, std::string& value) const
{
    OT_ASSERT(primary_plugin_);

    bool output = primary_plugin_->Store(key, value);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        output |= plugin->Store(key, value);
    }

    return output;
}

bool Storage::Store(
    const std::string& nymID,
    const proto::ContactItemType type,
    const proto::Bip44Account& data)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymID)
        .It()
        .Store(type, data);
}

bool Storage::Store(const proto::BlockchainTransaction& data)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Blockchain()
        .It()
        .Store(data);
}

bool Storage::Store(const proto::Contact& data)
{
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .Store(data, data.label());
}

bool Storage::Store(const proto::Context& data)
{
    std::string notUsed;

    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(data.localnym())
        .It()
        .mutable_Contexts()
        .It()
        .Store(data, notUsed);
}

bool Storage::Store(const proto::Credential& data)
{
    std::string notUsed;

    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Credentials()
        .It()
        .Store(data, notUsed);
}

bool Storage::Store(
    const proto::CredentialIndex& data,
    const std::string& alias)
{
    std::string plaintext;
    const bool saved = mutable_Meta()
                           .It()
                           .mutable_Tree()
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
    return mutable_Meta()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .mutable_Nym(nymid)
        .It()
        .mutable_Threads()
        .It()
        .mutable_Thread(threadid)
        .It()
        .Add(itemid, time, box, alias, data);
}

bool Storage::Store(
    const proto::PeerReply& data,
    const std::string& nymID,
    const StorageBox box)
{
    switch (box) {
        case StorageBox::SENTPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedReplyBox()
                .It()
                .Store(data);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedReplyBox()
                .It()
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
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_SentRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_IncomingRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_FinishedRequestBox()
                .It()
                .Store(data, now);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return mutable_Meta()
                .It()
                .mutable_Tree()
                .It()
                .mutable_Nyms()
                .It()
                .mutable_Nym(nymID)
                .It()
                .mutable_ProcessedRequestBox()
                .It()
                .Store(data, now);
        } break;
        default: {
            return false;
        }
    }
}

bool Storage::Store(const proto::Seed& data, const std::string& alias)
{
    return mutable_Meta().It().mutable_Tree().It().mutable_Seeds().It().Store(
        data, alias);
}

bool Storage::Store(const proto::ServerContract& data, const std::string& alias)
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        mutable_Meta().It().mutable_Tree().It().mutable_Servers().It().Store(
            data, alias, plaintext);

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
        mutable_Meta().It().mutable_Tree().It().mutable_Units().It().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_units_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

bool Storage::StoreRoot(const std::string& hash) const
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->StoreRoot(hash);
    }

    return primary_plugin_->StoreRoot(hash);
}

void Storage::synchronize_plugins()
{
    OT_ASSERT(primary_plugin_);

    const auto root = primary_plugin_->LoadRoot();

    if (root.empty()) {

        return;
    }

    auto node = mutable_Meta();
    auto tree = node.It().mutable_Tree();

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (root == plugin->LoadRoot()) {

            continue;
        }

        tree.It().Migrate(*plugin);
    }
}

void Storage::synchronize_root()
{
    OT_ASSERT(primary_plugin_);

    std::string bestRoot = primary_plugin_->LoadRoot();
    std::uint64_t bestVersion{0};

    std::unique_ptr<storage::Root> root;

    try {
        root.reset(
            new storage::Root(*this, bestRoot, gc_interval_, primary_bucket_));
        bestVersion = root->Sequence();
    } catch (std::runtime_error&) {
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        std::string rootHash = plugin->LoadRoot();
        std::uint64_t localVersion{0};
        std::unique_ptr<storage::Root> localRoot;

        try {
            localRoot.reset(new storage::Root(
                *this, rootHash, gc_interval_, primary_bucket_));
            localVersion = localRoot->Sequence();
        } catch (std::runtime_error&) {
        }

        if (localVersion > bestVersion) {
            bestVersion = localVersion;
            bestRoot = rootHash;
        }
    }

    if (0 == bestVersion) {
        bestRoot = "";
    }

    primary_plugin_->StoreRoot(bestRoot);
}

ObjectList Storage::ThreadList(const std::string& nymID) const
{
    return Meta().Tree().NymNode().Nym(nymID).Threads().List();
}

std::string Storage::ThreadAlias(
    const std::string& nymID,
    const std::string& threadID)
{
    return Meta()
        .Tree()
        .NymNode()
        .Nym(nymID)
        .Threads()
        .Thread(threadID)
        .Alias();
}

std::string Storage::UnitDefinitionAlias(const std::string& id)
{
    return Meta().Tree().UnitNode().Alias(id);
}

ObjectList Storage::UnitDefinitionList() const
{
    return Meta().Tree().UnitNode().List();
}

bool Storage::verify_write_lock(const std::unique_lock<std::mutex>& lock) const
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
