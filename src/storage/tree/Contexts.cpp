// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Contexts.hpp"

#include "storage/Plugin.hpp"

namespace opentxs
{
namespace storage
{
Contexts::Contexts(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

bool Contexts::Delete(const std::string& id) { return delete_item(id); }

void Contexts::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load servers index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Contexts::Load(
    const std::string& id,
    std::shared_ptr<proto::Context>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::Context>(id, output, alias, checking);
}

bool Contexts::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageNymList Contexts::serialize() const
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_nym());
        }
    }

    return serialized;
}

bool Contexts::Store(const proto::Context& data, const std::string& alias)
{
    return store_proto(data, data.remotenym(), alias);
}
}  // namespace storage
}  // namespace opentxs
