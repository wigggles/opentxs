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

#include "opentxs/storage/tree/Thread.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/storage/tree/Mailbox.hpp"
#include "opentxs/storage/StoragePlugin.hpp"

namespace opentxs
{
namespace storage
{
Thread::Thread(
    const StorageDriver& storage,
    const std::string& id,
    const std::string& hash,
    const std::string& alias,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, hash)
    , id_(id)
    , alias_(alias)
    , index_(0)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , participants_({id})
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 1;
        root_ = Node::BLANK_HASH;
    }
}

Thread::Thread(
    const StorageDriver& storage,
    const std::set<std::string>& participants,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, Node::BLANK_HASH)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , participants_(participants)
{
    version_ = 1;
    root_ = Node::BLANK_HASH;
}

bool Thread::Add(
    const std::string& id,
    const std::uint64_t time,
    const StorageBox& box,
    const std::string& alias,
    const std::string& contents,
    const std::uint64_t index,
    const std::string& account)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    bool saved = false;

    switch (box) {
        case StorageBox::MAILINBOX : {
            saved = mail_inbox_.Store(id, contents, alias);
        } break;
        case StorageBox::MAILOUTBOX : {
            saved = mail_outbox_.Store(id, contents, alias);
        } break;
        default : {
            std::cerr << __FUNCTION__ << ": Warning: unknown box." << std::endl;
        }
    }

    if (!saved) {
        std::cerr << __FUNCTION__ << ": Unable to save item." << std::endl;

        return false;
    }

    auto& item = items_[id];
    item.set_version(version_);
    item.set_id(id);

    if (0 == index) {
        item.set_index(index_++);
    } else {
        item.set_index(index);
    }

    item.set_time(time);
    item.set_box(static_cast<std::uint32_t>(box));
    item.set_account(account);
    item.set_unread(true);

    const bool valid = proto::Check(item, version_, version_);

    if (!valid) {
        items_.erase(id);

        return false;
    }

    return save(lock);
}

std::string Thread::Alias() const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return alias_;
}

void Thread::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageThread> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load thread index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    if (1 > version_) {
        version_ = 1;
    }

    for (const auto& it : serialized->item()) {
        const auto& index = it.index();
        items_.emplace(it.id(), it);

        if (index >= index_) {
            index_ = index + 1;
        }
    }
}

bool Thread::Check(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return items_.end() != items_.find(id);
}

std::string Thread::ID() const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    if (id_.empty()) {
        String plaintext;

        for (const auto& id : participants_) {
            plaintext.Concatenate(String(id.c_str()));
        }

        Identifier id;
        id.CalculateDigest(plaintext);

        id_ = String(id).Get();
    }

    return id_;
}

proto::StorageThread Thread::Items() const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return serialize(lock);
}

bool Thread::Migrate() const
{
    return Node::migrate(root_);
}

bool Thread::Read(const std::string& id)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    auto it = items_.find(id);

    if (items_.end() == it) { return false; }

    auto& item = it->second;

    item.set_unread(false);

    return save(lock);
}

bool Thread::Remove(const std::string& id) {
    std::unique_lock<std::mutex> lock(write_lock_);

    auto it = items_.find(id);

    if (items_.end() == it) { return false; }

    auto& item = it->second;
    StorageBox box = static_cast<StorageBox>(item.box());
    items_.erase(it);

    switch (box) {
        case StorageBox::MAILINBOX : {
            mail_inbox_.Delete(id);
        } break;
        case StorageBox::MAILOUTBOX : {
            mail_outbox_.Delete(id);
        } break;
        default : {
            std::cerr << __FUNCTION__ << ": Warning: unknown box." << std::endl;
        }
    }

    return save(lock);
}

bool Thread::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize(lock);

    if (!proto::Check(serialized, version_, version_)) {
        return false;
    }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageThread Thread::serialize(
    const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    proto::StorageThread serialized;
    serialized.set_version(version_);
    serialized.set_id(id_);

    for (const auto nym : participants_) {
        if (!nym.empty()) {
            *serialized.add_participant() = nym;
        }
    }

    auto sorted = sort(lock);

    for (const auto& it : sorted) {
        OT_ASSERT(nullptr != it.second);

        const auto& item = *it.second;
        *serialized.add_item() = item;
    }

    return serialized;
}

bool Thread::SetAlias(const std::string& alias)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    alias_ = alias;

    return true;
}

Thread::SortedItems Thread::sort(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    SortedItems output;

    for (const auto& it : items_) {
        const auto& id = it.first;
        const auto& item = it.second;

        if (!id.empty()) {
            SortKey key{item.index(), item.time(), id};
            output.emplace(key, &item);
        }
    }

    return output;
}
}  // namespace storage
}  // namespace opentxs
