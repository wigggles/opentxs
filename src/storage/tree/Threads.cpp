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

#include "opentxs/storage/tree/Threads.hpp"

#include "opentxs/storage/tree/Thread.hpp"
#include "opentxs/storage/Storage.hpp"

#include <functional>

namespace opentxs
{
namespace storage
{
Threads::Threads(
    const Storage& storage,
    const keyFunction& migrate,
    const std::string& hash,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, migrate, hash)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 2;
        root_ = Node::BLANK_HASH;
    }
}

std::string Threads::Create(const std::set<std::string>& participants)
{
    std::unique_ptr<class Thread> newThread(new class Thread(
        storage_, migrate_, participants, mail_inbox_, mail_outbox_));

    if (!newThread) {
        std::cerr << __FUNCTION__ << ": Failed to instantiate thread."
                    << std::endl;
        abort();
    }

    const std::string id = newThread->ID();

    std::unique_lock<std::mutex> lock(write_lock_);

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (!node) {
        std::unique_lock<std::mutex> threadLock(newThread->write_lock_);
        newThread->save(threadLock);
        node.swap(newThread);
        save(lock);
    }

    lock.unlock();

    return id;
}

bool Threads::Exists(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return threads_.find(id) != threads_.end();
}

bool Threads::FindAndDeleteItem(const std::string& itemID)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    bool found = false;

    for (const auto index : item_map_) {
        const auto& id = index.first;
        auto& node = *thread(id, lock);
        const bool hasItem = node.Check(itemID);

        if (hasItem) {
            node.Remove(itemID);
            found = true;
        }
    }

    if (found) {
        save(lock);
    }

    return found;
}

void Threads::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    storage_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load thread list index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) {
        version_ = 2;
    }

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Threads::Migrate() const
{
    for (const auto index : item_map_) {
        const auto& id = index.first;
        const auto& node = *thread(id);
        node.Migrate();
    }

    return Node::migrate(root_);
}

Editor<class Thread> Threads::mutable_Thread(const std::string& id)
{
    std::function<void(class Thread*, std::unique_lock<std::mutex>&)> callback =
        [&](class Thread* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return Editor<class Thread>(write_lock_, thread(id), callback);
}

class Thread* Threads::thread(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return thread(id, lock);
}

class Thread* Threads::thread(
    const std::string& id,
    const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (!node) {
        node.reset(new class Thread(
            storage_, migrate_, id, hash, alias, mail_inbox_, mail_outbox_));

        if (!node) {
            std::cerr << __FUNCTION__ << ": Failed to instantiate thread."
                      << std::endl;
            abort();
        }
    }

    return node.get();
}

const class Thread& Threads::Thread(const std::string& id) const
{
    return *thread(id);
}

bool Threads::save(const std::unique_lock<std::mutex>& lock)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Check(serialized, version_, version_)) {
        return false;
    }

    return storage_.StoreProto(serialized, root_);
}

void Threads::save(
    class Thread* nym,
    const std::unique_lock<std::mutex>& lock,
    const std::string& id)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == nym) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

proto::StorageNymList Threads::serialize() const
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_nym());
        }
    }

    return serialized;
}
}  // namespace storage
}  // namespace opentxs
