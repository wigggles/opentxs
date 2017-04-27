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

#include "opentxs/storage/tree/Nyms.hpp"

#include "opentxs/storage/tree/Nym.hpp"
#include "opentxs/storage/Storage.hpp"
#include "opentxs/storage/StoragePlugin.hpp"

#include <functional>

namespace opentxs
{
namespace storage
{
Nyms::Nyms(
    const StorageDriver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 2;
        root_ = Node::BLANK_HASH;
    }
}

void Nyms::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load nym list index file."
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

void Nyms::Map(NymLambda lambda) const {
    std::unique_lock<std::mutex> lock(write_lock_);
    const auto copy = item_map_;
    write_lock_.unlock();

    for (const auto it : copy) {
        const auto& id = it.first;
        const auto& node = *nym(id);
        const auto& hash = node.credentials_;

        std::shared_ptr<proto::CredentialIndex> serialized;

        if (Node::BLANK_HASH == hash) { continue; }

        if (driver_.LoadProto(hash, serialized, false)) {
            lambda(*serialized);
        }
    }
}

bool Nyms::Migrate() const
{
    for (const auto index : item_map_) {
        const auto& id = index.first;
        const auto& node = *nym(id);
        node.Migrate();
    }

    return Node::migrate(root_);
}

Editor<class Nym> Nyms::mutable_Nym(const std::string& id)
{
    std::function<void(class Nym*, std::unique_lock<std::mutex>&)> callback =
        [&](class Nym* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return Editor<class Nym>(write_lock_, nym(id), callback);
}

class Nym* Nyms::nym(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = nyms_[id];

    if (!node) {
        node.reset(new class Nym(driver_, id, hash, alias));

        if (!node) {
            std::cerr << __FUNCTION__ << ": Failed to instantiate nym."
                      << std::endl;
            abort();
        }
    }

    lock.unlock();

    return node.get();
}

const class Nym& Nyms::Nym(const std::string& id) const { return *nym(id); }

bool Nyms::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Check(serialized, version_, version_)) {
        return false;
    }

    return driver_.StoreProto(serialized, root_);
}

void Nyms::save(
    class Nym* nym,
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

proto::StorageNymList Nyms::serialize() const
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
