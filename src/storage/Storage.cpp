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

#include "opentxs/stdafx.hpp"

#include "opentxs/storage/Storage.hpp"

#include "opentxs/storage/drivers/StorageMultiplex.hpp"
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

namespace opentxs::api
{
const std::uint32_t Storage::HASH_TYPE = 2;  // BTC160

Storage::Storage(
    const std::atomic<bool>& shutdown,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random)
    : shutdown_(shutdown)
    , gc_interval_(config.gc_interval_)
    , write_lock_()
    , root_(nullptr)
    , primary_bucket_(false)
    , background_threads_()
    , config_(config)
    , multiplex_p_(new StorageMultiplex(primary_bucket_, config_, hash, random))
    , multiplex_(*multiplex_p_)
{
    OT_ASSERT(multiplex_p_);
}

std::set<std::string> Storage::BlockchainAccountList(
    const std::string& nymID,
    const proto::ContactItemType type)
{
    return Root().Tree().NymNode().Nym(nymID).BlockchainAccountList(type);
}

std::string Storage::BlockchainAddressOwner(
    proto::ContactItemType chain,
    std::string address)
{
    return Root().Tree().ContactNode().AddressOwner(chain, address);
}

ObjectList Storage::BlockchainTransactionList()
{

    return Root().Tree().BlockchainNode().List();
}

void Storage::Cleanup_Storage()
{
    for (auto& thread : background_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (root_) {
        root_->cleanup();
    }
}

void Storage::Cleanup() { Cleanup_Storage(); }

void Storage::CollectGarbage() { Root().Migrate(multiplex_.Primary()); }

std::string Storage::ContactAlias(const std::string& id)
{
    return Root().Tree().ContactNode().Alias(id);
}

ObjectList Storage::ContactList() { return Root().Tree().ContactNode().List(); }

std::string Storage::ContactOwnerNym(const std::string& nymID)
{
    return Root().Tree().ContactNode().NymOwner(nymID);
}

void Storage::ContactSaveIndices()
{
    mutable_Root().It().mutable_Tree().It().mutable_Contacts().It().Save();
}

std::uint32_t Storage::ContactUpgradeLevel() const
{
    return Root().Tree().ContactNode().UpgradeLevel();
}

ObjectList Storage::ContextList(const std::string& nymID)
{
    return Root().Tree().NymNode().Nym(nymID).Contexts().List();
}

bool Storage::CreateThread(
    const std::string& nymID,
    const std::string& threadID,
    const std::set<std::string>& participants)
{
    const auto id = mutable_Root()
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
    return Root().Tree().SeedNode().Default();
}

bool Storage::DeleteContact(const std::string& id)
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .Delete(id);
}

void Storage::InitBackup() { multiplex_.InitBackup(); }

void Storage::InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key)
{
    multiplex_.InitEncryptedBackup(key);
}

void Storage::InitPlugins()
{
    bool syncPrimary{false};
    const auto hash = multiplex_.best_root(syncPrimary);

    if (hash.empty()) {

        return;
    }

    std::unique_ptr<storage::Root> root{new storage::Root(
        multiplex_,
        hash,
        std::numeric_limits<std::int64_t>::max(),
        primary_bucket_)};

    OT_ASSERT(root);

    multiplex_.synchronize_plugins(hash, *root, syncPrimary);
}

bool Storage::Load(
    const std::string& nymID,
    const std::string& accountID,
    std::shared_ptr<proto::Bip44Account>& output,
    const bool checking)
{
    return Root().Tree().NymNode().Nym(nymID).Load(accountID, output, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::BlockchainTransaction>& transaction,
    const bool checking)
{
    return Root().Tree().BlockchainNode().Load(id, transaction, checking);
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
    return Root().Tree().ContactNode().Load(id, contact, alias, checking);
}

bool Storage::Load(
    const std::string& nym,
    const std::string& id,
    std::shared_ptr<proto::Context>& context,
    const bool checking)
{
    std::string notUsed;

    return Root().Tree().NymNode().Nym(nym).Contexts().Load(
        id, context, notUsed, checking);
}

bool Storage::Load(
    const std::string& id,
    std::shared_ptr<proto::Credential>& cred,
    const bool checking)
{
    return Root().Tree().CredentialNode().Load(id, cred, checking);
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
    return Root().Tree().NymNode().Nym(id).Load(nym, alias, checking);
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
            return Root().Tree().NymNode().Nym(nymID).MailInbox().Load(
                id, output, alias, checking);
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailOutbox().Load(
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
            return Root().Tree().NymNode().Nym(nymID).SentReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).IncomingReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).FinishedReplyBox().Load(
                id, reply, checking);
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).ProcessedReplyBox().Load(
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
            output = Root().Tree().NymNode().Nym(nymID).SentRequestBox().Load(
                id, request, alias, checking);
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).IncomingRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).FinishedRequestBox().Load(
                    id, request, alias, checking);
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            output =
                Root().Tree().NymNode().Nym(nymID).ProcessedRequestBox().Load(
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
    return Root().Tree().SeedNode().Load(id, seed, alias, checking);
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
    return Root().Tree().ServerNode().Load(id, contract, alias, checking);
}

bool Storage::Load(
    const std::string& nymId,
    const std::string& threadId,
    std::shared_ptr<proto::StorageThread>& thread)
{
    const bool exists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(threadId);

    if (!exists) {
        return false;
    }

    thread.reset(new proto::StorageThread);

    if (!thread) {
        return false;
    }

    *thread =
        Root().Tree().NymNode().Nym(nymId).Threads().Thread(threadId).Items();

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
    return Root().Tree().UnitNode().Load(id, contract, alias, checking);
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

storage::Root* Storage::root() const
{
    Lock lock(write_lock_);

    if (!root_) {
        root_.reset(new storage::Root(
            multiplex_, multiplex_.LoadRoot(), gc_interval_, primary_bucket_));
    }

    OT_ASSERT(root_);

    lock.unlock();

    return root_.get();
}

const storage::Root& Storage::Root() const { return *root(); }

bool Storage::MoveThreadItem(
    const std::string& nymId,
    const std::string& fromThreadID,
    const std::string& toThreadID,
    const std::string& itemID)
{
    const bool fromExists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(fromThreadID);

    if (false == fromExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": From thread does not exist."
              << std::endl;

        return false;
    }

    const bool toExists =
        Root().Tree().NymNode().Nym(nymId).Threads().Exists(toThreadID);

    if (false == toExists) {
        otErr << OT_METHOD << __FUNCTION__ << ": To thread does not exist."
              << std::endl;

        return false;
    }

    auto& fromThread = mutable_Root()
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

    auto& toThread = mutable_Root()
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

Editor<storage::Root> Storage::mutable_Root()
{
    std::function<void(storage::Root*, Lock&)> callback =
        [&](storage::Root* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<storage::Root>(write_lock_, root(), callback);
}

ObjectList Storage::NymBoxList(const std::string& nymID, const StorageBox box)
    const
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return Root().Tree().NymNode().Nym(nymID).SentRequestBox().List();
        } break;
        case StorageBox::INCOMINGPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .IncomingRequestBox()
                .List();
        } break;
        case StorageBox::SENTPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).SentReplyBox().List();
        } break;
        case StorageBox::INCOMINGPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).IncomingReplyBox().List();
        } break;
        case StorageBox::FINISHEDPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .FinishedRequestBox()
                .List();
        } break;
        case StorageBox::FINISHEDPEERREPLY: {
            return Root().Tree().NymNode().Nym(nymID).FinishedReplyBox().List();
        } break;
        case StorageBox::PROCESSEDPEERREQUEST: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedRequestBox()
                .List();
        } break;
        case StorageBox::PROCESSEDPEERREPLY: {
            return Root()
                .Tree()
                .NymNode()
                .Nym(nymID)
                .ProcessedReplyBox()
                .List();
        } break;
        case StorageBox::MAILINBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailInbox().List();
        }
        case StorageBox::MAILOUTBOX: {
            return Root().Tree().NymNode().Nym(nymID).MailOutbox().List();
        }
        default: {
            return {};
        }
    }
}

ObjectList Storage::NymList() const { return Root().Tree().NymNode().List(); }

bool Storage::RelabelThread(
    const std::string& threadID,
    const std::string& label)
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Nyms()
        .It()
        .RelabelThread(threadID, label);
}

bool Storage::RemoveNymBoxItem(
    const std::string& nymID,
    const StorageBox box,
    const std::string& itemID)
{
    switch (box) {
        case StorageBox::SENTPEERREQUEST: {
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            const bool foundInThread = mutable_Root()
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
                foundInBox = mutable_Root()
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
            const bool foundInThread = mutable_Root()
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
                foundInBox = mutable_Root()
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
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Servers()
        .It()
        .Delete(id);
}

bool Storage::RemoveUnitDefinition(const std::string& id)
{
    return mutable_Root().It().mutable_Tree().It().mutable_Units().It().Delete(
        id);
}

bool Storage::RenameThread(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& newID)
{
    return mutable_Root()
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
    return Root().Tree().NymNode().Map(lambda);
}

void Storage::RunMapServers(ServerLambda lambda)
{
    return Root().Tree().ServerNode().Map(lambda);
}

void Storage::RunMapUnits(UnitLambda lambda)
{
    return Root().Tree().UnitNode().Map(lambda);
}

void Storage::save(storage::Root* in, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));
    OT_ASSERT(nullptr != in);

    multiplex_.StoreRoot(true, in->root_);
}

bool Storage::SetContactAlias(const std::string& id, const std::string& alias)
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Contacts()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetDefaultSeed(const std::string& id)
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetDefault(id);
}

bool Storage::SetNymAlias(const std::string& id, const std::string& alias)
{
    return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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

bool Storage::SetReadState(
    const std::string& nymId,
    const std::string& threadId,
    const std::string& itemId,
    const bool unread)
{
    auto& nyms = mutable_Root().It().mutable_Tree().It().mutable_Nyms().It();

    if (false == nyms.Exists(nymId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymId
              << " does not exist." << std::endl;

        return false;
    }

    auto& threads = nyms.mutable_Nym(nymId).It().mutable_Threads().It();

    if (false == threads.Exists(threadId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Thread " << threadId
              << " does not exist." << std::endl;

        return false;
    }

    return threads.mutable_Thread(threadId).It().Read(itemId, unread);
}

bool Storage::SetSeedAlias(const std::string& id, const std::string& alias)
{
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Seeds()
        .It()
        .SetAlias(id, alias);
}

bool Storage::SetServerAlias(const std::string& id, const std::string& alias)
{
    return mutable_Root()
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
    return mutable_Root()
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
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Units()
        .It()
        .SetAlias(id, alias);
}

std::string Storage::ServerAlias(const std::string& id)
{
    return Root().Tree().ServerNode().Alias(id);
}

ObjectList Storage::ServerList() const
{
    return Root().Tree().ServerNode().List();
}

void Storage::start() { InitPlugins(); }

bool Storage::Store(
    const std::string& nymID,
    const proto::ContactItemType type,
    const proto::Bip44Account& data)
{
    return mutable_Root()
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
    return mutable_Root()
        .It()
        .mutable_Tree()
        .It()
        .mutable_Blockchain()
        .It()
        .Store(data);
}

bool Storage::Store(const proto::Contact& data)
{
    return mutable_Root()
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

    return mutable_Root()
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

    return mutable_Root()
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
    const bool saved = mutable_Root()
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
    return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
            return mutable_Root()
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
    return mutable_Root().It().mutable_Tree().It().mutable_Seeds().It().Store(
        data, alias);
}

bool Storage::Store(const proto::ServerContract& data, const std::string& alias)
{
    auto storageVersion(data);
    storageVersion.clear_publicnym();
    std::string plaintext;
    const bool saved =
        mutable_Root().It().mutable_Tree().It().mutable_Servers().It().Store(
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
        mutable_Root().It().mutable_Tree().It().mutable_Units().It().Store(
            data, alias, plaintext);

    if (saved) {
        if (config_.auto_publish_units_ && config_.dht_callback_) {
            config_.dht_callback_(storageVersion.id(), plaintext);
        }

        return true;
    }

    return false;
}

ObjectList Storage::ThreadList(const std::string& nymID, const bool unreadOnly)
    const
{
    return Root().Tree().NymNode().Nym(nymID).Threads().List(unreadOnly);
}

std::string Storage::ThreadAlias(
    const std::string& nymID,
    const std::string& threadID)
{
    return Root()
        .Tree()
        .NymNode()
        .Nym(nymID)
        .Threads()
        .Thread(threadID)
        .Alias();
}

std::string Storage::UnitDefinitionAlias(const std::string& id)
{
    return Root().Tree().UnitNode().Alias(id);
}

ObjectList Storage::UnitDefinitionList() const
{
    return Root().Tree().UnitNode().List();
}

std::size_t Storage::UnreadCount(
    const std::string& nymId,
    const std::string& threadId)
{
    auto& nyms = Root().Tree().NymNode();

    if (false == nyms.Exists(nymId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymId
              << " does not exist." << std::endl;

        return 0;
    }

    auto& threads = nyms.Nym(nymId).Threads();

    if (false == threads.Exists(threadId)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Thread " << threadId
              << " does not exist." << std::endl;

        return 0;
    }

    return threads.Thread(threadId).UnreadCount();
}

bool Storage::verify_write_lock(const Lock& lock) const
{
    if (lock.mutex() != &write_lock_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

Storage::~Storage() { Cleanup_Storage(); }
}  // namespace opentxs::api
