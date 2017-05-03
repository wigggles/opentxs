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

#include "opentxs/storage/tree/Root.hpp"

#include "opentxs/storage/tree/Credentials.hpp"
#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/tree/Nyms.hpp"
#include "opentxs/storage/tree/Seeds.hpp"
#include "opentxs/storage/tree/Servers.hpp"
#include "opentxs/storage/tree/Tree.hpp"
#include "opentxs/storage/tree/Units.hpp"
#include "opentxs/storage/StoragePlugin.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"

#define OT_METHOD "opentxs::storage::Root::"

namespace opentxs
{
namespace storage
{
Root::Root(
    const StorageDriver& storage,
    const std::string& hash,
    const std::int64_t interval,
    const EmptyBucket& empty,
    std::atomic<bool>& bucket)
    : ot_super(storage, hash)
    , gc_interval_(interval)
    , empty_bucket_(empty)
    , current_bucket_(bucket)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 2;
        root_ = Node::BLANK_HASH;
        gc_root_ = Node::BLANK_HASH;
        current_bucket_.store(false);
        gc_running_.store(false);
        gc_resume_.store(false);
        last_gc_.store(static_cast<std::int64_t>(std::time(nullptr)));
        sequence_.store(0);
        tree_root_ = Node::BLANK_HASH;
    }
}

void Root::cleanup() const
{
    Lock gclock(gc_lock_);

    if (gc_thread_) {
        if (gc_thread_->joinable()) {
            gc_thread_->join();
        }

        gc_thread_.reset();
    }
}

void Root::collect_garbage() const
{
    Lock lock(write_lock_);
    otErr << OT_METHOD << __FUNCTION__ << ": Beginning garbage collection."
          << std::endl;
    const bool resume = gc_resume_.exchange(false);
    bool oldLocation = false;

    if (resume) {
        oldLocation = !current_bucket_.load();
    } else {
        gc_root_ = tree()->Root();
        oldLocation = current_bucket_.load();
        current_bucket_.store(!oldLocation);
        save(lock);
    }

    lock.unlock();
    bool success = false;

    if (!gc_root_.empty()) {
        const class Tree tree(driver_, gc_root_);
        success = tree.Migrate();
    }

    if (success) {
        empty_bucket_(oldLocation);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Garbage collection failed. "
              << "Will retry next cycle." << std::endl;
    }

    Lock gcLock(gc_lock_, std::defer_lock);
    std::lock(gcLock, lock);
    gc_running_.store(false);
    gc_root_ = "";
    last_gc_.store(std::time(nullptr));
    save(lock);
    lock.unlock();
    gcLock.unlock();
    otErr << OT_METHOD << __FUNCTION__ << ": Finished garbage collection."
          << std::endl;
}

void Root::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageRoot> serialized;

    if (!driver_.LoadProto(hash, serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load root object file." << std::endl;
        OT_FAIL;
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) {
        version_ = 2;
    }

    gc_root_ = serialized->gcroot();
    current_bucket_.store(serialized->altlocation());
    gc_running_.store(false);
    gc_resume_.store(serialized->gc());
    last_gc_.store(serialized->lastgc());
    sequence_.store(serialized->sequence());
    tree_root_ = serialized->items();
}

bool Root::Migrate() const
{
    const std::uint64_t time = std::time(nullptr);
    const bool intervalExceeded = ((time - last_gc_.load()) > gc_interval_);
    const bool resume = gc_resume_.load();
    const bool needToCollectGarbage = resume || intervalExceeded;

    if (needToCollectGarbage) {
        const bool running = gc_running_.exchange(true);

        if (!running) {
            cleanup();
            gc_thread_.reset(new std::thread(&Root::collect_garbage, this));

            return true;
        }
    }

    return false;
}

Editor<class Tree> Root::mutable_Tree()
{
    std::function<void(class Tree*, Lock&)> callback =
        [&](class Tree* in, Lock& lock) -> void {this->save(in, lock);};

    return Editor<class Tree>(write_lock_, tree(), callback);
}

bool Root::save(const std::unique_lock<std::mutex>& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    sequence_++;
    auto serialized = serialize();

    if (!proto::Check(serialized, version_, version_)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Root::save(class Tree* tree, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    OT_ASSERT(nullptr != tree);

    Lock treeLock(tree_lock_);
    tree_root_ = tree->Root();
    tree_lock_.unlock();

    const bool saved = save(lock);

    OT_ASSERT(saved);
}

proto::StorageRoot Root::serialize() const
{
    proto::StorageRoot output;
    output.set_version(version_);
    output.set_items(tree_root_);
    output.set_altlocation(current_bucket_.load());
    output.set_lastgc(last_gc_.load());
    output.set_gc(gc_running_.load());
    output.set_gcroot(gc_root_);
    output.set_sequence(sequence_);

    return output;
}

class Tree* Root::tree() const
{
    Lock lock(tree_lock_);

    if (!tree_) {
        tree_.reset(new class Tree(driver_, tree_root_));
    }

    OT_ASSERT(tree_);

    lock.unlock();

    return tree_.get();
}

const class Tree& Root::Tree() const { return *tree(); }
}  // namespace storage
}  // namespace opentxs
