// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PeerReplies.hpp"

#include "storage/Plugin.hpp"

namespace opentxs
{
namespace storage
{
PeerReplies::PeerReplies(
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

bool PeerReplies::Delete(const std::string& id) { return delete_item(id); }

void PeerReplies::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load peer reply index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) { version_ = 2; }

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool PeerReplies::Load(
    const std::string& id,
    std::shared_ptr<proto::PeerReply>& output,
    const bool checking) const
{
    std::string notUsed;

    bool loaded = load_proto<proto::PeerReply>(id, output, notUsed, true);

    if (loaded) { return true; }

    // The provided ID might actually be a request ID instead of a reply ID.

    std::unique_lock<std::mutex> lock(write_lock_);
    std::string realID;

    for (const auto& it : item_map_) {
        const auto& reply = it.first;
        const auto& alias = std::get<1>(it.second);

        if (id == alias) {
            realID = reply;
            break;
        }
    }

    lock.unlock();

    if (realID.empty()) { return false; }

    return load_proto<proto::PeerReply>(realID, output, notUsed, checking);
    ;
}

bool PeerReplies::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageNymList PeerReplies::serialize() const
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

bool PeerReplies::Store(const proto::PeerReply& data)
{
    return store_proto(data, data.id(), data.cookie());
}
}  // namespace storage
}  // namespace opentxs
