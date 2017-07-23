/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anoseedous Numbered Accounts.
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

#include "opentxs/storage/tree/Contacts.hpp"

#include "opentxs/storage/StoragePlugin.hpp"

#define CURRENT_VERSION 1

#define OT_METHOD "opentxs::storage::Contacts::"

namespace opentxs
{
namespace storage
{
Contacts::Contacts(const StorageDriver& storage, const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

std::string Contacts::Alias(const std::string& id) const
{
    return get_alias(id);
}

bool Contacts::Delete(const std::string& id) { return delete_item(id); }

void Contacts::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageContacts> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to load contact index file." << std::endl;

        abort();
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) {
        version_ = CURRENT_VERSION;
    }

    for (const auto& parent : serialized->merge()) {
        auto& list = merge_[parent.id()];

        for (const auto& id : parent.list()) {
            list.emplace(id);
        }
    }

    reverse_merged();

    for (const auto& it : serialized->contact()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Contacts::Load(
    const std::string& id,
    std::shared_ptr<proto::Contact>& output,
    std::string& alias,
    const bool checking) const
{
    const auto& normalized = nomalize_id(id);

    return load_proto<proto::Contact>(normalized, output, alias, checking);
}

const std::string& Contacts::nomalize_id(const std::string& input) const
{
    Lock lock(write_lock_);

    const auto it = merged_.find(input);

    if (merged_.end() == it) {

        return input;
    }

    return it->second;
}

void Contacts::reconcile_maps(const Lock& lock, const proto::Contact& data)
{
    if (false == verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;

        abort();
    }

    const auto& contactID = data.id();

    for (const auto& merged : data.merged()) {
        auto& list = merge_[contactID];
        list.emplace(merged);
        merged_[merged].assign(contactID);
    }

    const auto& newParent = data.mergedto();

    if (false == newParent.empty()) {
        std::string oldParent{};
        const auto it = merged_.find(contactID);

        if (merged_.end() != it) {
            oldParent = it->second;
        }

        if (false == oldParent.empty()) {
            merge_[oldParent].erase(contactID);
        }

        merge_[newParent].emplace(contactID);
    }
}

void Contacts::reverse_merged()
{
    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;

        for (const auto& childID : list) {
            merged_[childID].assign(parentID);
        }
    }
}

bool Contacts::save(const Lock& lock) const
{
    if (false == verify_write_lock(lock)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock failure." << std::endl;

        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) {

        return false;
    }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageContacts Contacts::serialize() const
{
    proto::StorageContacts serialized;
    serialized.set_version(version_);

    for (const auto& parent : merge_) {
        const auto& parentID = parent.first;
        const auto& list = parent.second;
        auto& item = *serialized.add_merge();
        item.set_version(version_);
        item.set_id(parentID);

        for (const auto& child : list) {
            item.add_list(child);
        }
    }

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_contact());
        }
    }

    return serialized;
}

bool Contacts::SetAlias(const std::string& id, const std::string& alias)
{
    const auto& normalized = nomalize_id(id);

    return set_alias(normalized, alias);
}

bool Contacts::Store(const proto::Contact& data, const std::string& alias)
{
    if (false == proto::Validate(data, VERBOSE)) {

        return false;
    }

    Lock lock(write_lock_);

    const std::string id = data.id();
    const auto incomingRevision = data.revision();
    const bool existingKey = (item_map_.end() != item_map_.find(id));
    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (existingKey) {
        const bool revisionCheck = check_revision<proto::Contact>(
            OT_METHOD, incomingRevision, metadata);

        if (false == revisionCheck) {
            // We're trying to save a contact with a lower revision than has
            // already been saved. Just silently skip this update instead.

            return true;
        }
    }

    if (false == driver_.StoreProto(data, hash)) {

        return false;
    }

    if (false == alias.empty()) {
        std::get<1>(metadata) = alias;
    } else {
        if (false == data.label().empty()) {
            std::get<1>(metadata) = data.label();
        }
    }

    reconcile_maps(lock, data);

    return save(lock);
}
}  // namespace storage
}  // namespace opentxs
