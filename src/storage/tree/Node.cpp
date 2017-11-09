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

#include "opentxs/storage/tree/Node.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/storage/StoragePlugin.hpp"

#define OT_METHOD "opentxs::storage::Node::"

namespace opentxs::storage
{
const std::string Node::BLANK_HASH = "blankblankblankblankblank";

Node::Node(const StorageDriver& storage, const std::string& key)
    : driver_(storage)
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

std::uint64_t Node::extract_revision(const proto::Contact& input) const
{
    return input.revision();
}

std::uint64_t Node::extract_revision(const proto::CredentialIndex& input) const
{
    return input.revision();
}

std::uint64_t Node::extract_revision(const proto::Seed& input) const
{
    return input.index();
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
            otErr << OT_METHOD << __FUNCTION__ << ": Error: item with id " << id
                  << " does not exist." << std::endl;
        }

        return false;
    }

    alias = std::get<1>(it->second);

    return driver_.Load(std::get<0>(it->second), checking, output);
}

bool Node::migrate(const std::string& hash, const StorageDriver& to) const
{
    if (false == check_hash(hash)) {

        return true;
    }

    return driver_.Migrate(hash, to);
}

bool Node::Migrate(const StorageDriver& to) const
{
    if (std::string(BLANK_HASH) == root_) {
        if (0 < item_map_.size()) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Items present in object with blank root hash."
                  << std::endl;

            OT_FAIL;
        }

        return true;
    }

    bool output{true};
    output &= migrate(root_, to);

    for (const auto& item : item_map_) {
        const auto& hash = std::get<0>(item.second);
        output &= migrate(hash, to);
    }

    return output;
}

std::string Node::normalize_hash(const std::string& hash)
{
    if (hash.empty()) {

        return BLANK_HASH;
    }

    if (proto::MIN_PLAUSIBLE_IDENTIFIER > hash.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Blanked out short hash "
              << hash << "\n"
              << std::endl;

        return BLANK_HASH;
    }

    if (proto::MAX_PLAUSIBLE_IDENTIFIER < hash.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Blanked out long hash " << hash
              << "\n"
              << std::endl;

        return BLANK_HASH;
    }

    return hash;
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
    if (2 > version) {
        if (proto::STORAGEHASH_ERROR != type) {
            output.set_version(2);
        } else {
            output.set_version(version);
        }
    } else {
        output.set_version(version);
    }

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

    if (!driver_.Store(true, data, hash)) {
        return false;
    }

    if (!alias.empty()) {
        std::get<1>(metadata) = alias;
    }

    return save(lock);
}

std::uint32_t Node::UpgradeLevel() const { return original_version_; }

bool Node::verify_write_lock(const std::unique_lock<std::mutex>& lock) const
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
}  // namespace opentxs::storage
