// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PeerRequests.hpp"

#include "storage/Plugin.hpp"

namespace opentxs
{
namespace storage
{
PeerRequests::PeerRequests(
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

bool PeerRequests::Delete(const std::string& id) { return delete_item(id); }

void PeerRequests::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load peer request index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool PeerRequests::Load(
    const std::string& id,
    std::shared_ptr<proto::PeerRequest>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::PeerRequest>(id, output, alias, checking);
}

bool PeerRequests::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageNymList PeerRequests::serialize() const
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
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

bool PeerRequests::SetAlias(const std::string& id, const std::string& alias)
{
    return set_alias(id, alias);
}

bool PeerRequests::Store(
    const proto::PeerRequest& data,
    const std::string& alias)
{
    return store_proto(data, data.id(), alias);
}
}  // namespace storage
}  // namespace opentxs
