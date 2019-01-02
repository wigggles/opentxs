// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Units.hpp"

#include "storage/Plugin.hpp"

namespace opentxs
{
namespace storage
{
Units::Units(
    const opentxs::api::storage::Driver& storage,
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

std::string Units::Alias(const std::string& id) const { return get_alias(id); }

bool Units::Delete(const std::string& id) { return delete_item(id); }

void Units::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageUnits> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load unit index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) { version_ = 2; }

    for (const auto& it : serialized->unit()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Units::Load(
    const std::string& id,
    std::shared_ptr<proto::UnitDefinition>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::UnitDefinition>(id, output, alias, checking);
}

void Units::Map(UnitLambda lambda) const { map<proto::UnitDefinition>(lambda); }

bool Units::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageUnits Units::serialize() const
{
    proto::StorageUnits serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_unit());
        }
    }

    return serialized;
}

bool Units::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

bool Units::Store(
    const proto::UnitDefinition& data,
    const std::string& alias,
    std::string& plaintext)
{
    return store_proto(data, data.id(), alias, plaintext);
}
}  // namespace storage
}  // namespace opentxs
