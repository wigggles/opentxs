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

#include "opentxs/storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{

const std::string Node::BLANK_HASH = "blankblankblankblankblank";

Node::Node(
    const Storage& storage,
    const keyFunction& migrate,
    const std::string& key)
    : storage_(storage)
    , migrate_(migrate)
    , root_(key)
{
}

bool Node::check_hash(const std::string& hash) const
{
    const bool empty = hash.empty();
    const bool blank = (Node::BLANK_HASH == hash);

    return !(empty || blank);
}

bool Node::delete_item(const std::string& id)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    const auto items = item_map_.erase(id);

    if (0 == items) {
        return false;
    }

    return save(lock);
}

std::string Node::get_alias(const std::string& id) const
{
    std::string output;
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);

    if (item_map_.end() != it) {
        output = std::get<1>(it->second);
    }

    return output;
}

ObjectList Node::List() const
{
    ObjectList output;
    std::unique_lock<std::mutex> lock(write_lock_);

    for (const auto it : item_map_) {
        output.push_back({it.first, std::get<1>(it.second)});
    }

    lock.unlock();

    return output;
}

bool Node::load_raw(
    const std::string& id,
    std::string& output,
    std::string& alias,
    const bool checking) const
{
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);
    const bool exists = (item_map_.end() != it);

    if (!exists) {
        if (!checking) {
            std::cout << __FUNCTION__ << ": Error: item with id " << id
                        << " does not exist." << std::endl;
        }

        return false;
    }

    alias = std::get<1>(it->second);

    return storage_.LoadRaw(std::get<0>(it->second), output, checking);
}

bool Node::migrate(const std::string& hash) const
{
    if (!check_hash(hash)) {
        return true;
    }

    return migrate_(hash);
}

bool Node::Migrate() const
{
    for (const auto item : item_map_) {
        if (!migrate(std::get<0>(item.second))) {
            return false;
        }
    }

    return migrate(root_);
}

std::string Node::Root() const
{
    std::lock_guard<std::mutex> lock_(write_lock_);

    return root_;
}

void Node::serialize_index(
    const std::string& id,
    const Metadata& metadata,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    set_hash(version_, id, std::get<0>(metadata), output, type);
    output.set_alias(std::get<1>(metadata));
}

bool Node::set_alias(const std::string& id, const std::string& alias)
{
    std::string output;
    std::unique_lock<std::mutex> lock(write_lock_);

    const bool exists = (item_map_.end() != item_map_.find(id));

    if (!exists) {
        return false;
    }

    std::get<1>(item_map_[id]) = alias;

    return save(lock);
}

void Node::set_hash(
    const std::uint32_t version,
    const std::string& id,
    const std::string& hash,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    output.set_version(version);
    output.set_itemid(id);

    if (hash.empty()) {
        output.set_hash(Node::BLANK_HASH);
    } else {
        output.set_hash(hash);
    }

    output.set_type(type);
}

bool Node::store_raw(
    const std::string& data,
    const std::string& id,
    const std::string& alias)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (!storage_.StoreRaw(data, hash)) {
        return false;
    }

    if (!alias.empty()) {
        std::get<1>(metadata) = alias;
    }

    return save(lock);
}

bool Node::verify_write_lock(const std::unique_lock<std::mutex>& lock)
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
}  // namespace storage
}  // namespace opentxs
