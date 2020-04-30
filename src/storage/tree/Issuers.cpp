// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "storage/tree/Issuers.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

#define CURRENT_VERSION 1

namespace opentxs
{
namespace storage
{
Issuers::Issuers(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CURRENT_VERSION);
    }
}

bool Issuers::Delete(const std::string& id) { return delete_item(id); }

void Issuers::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageIssuers> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load issuers index file."
                  << std::endl;
        abort();
    }

    init_version(CURRENT_VERSION, *serialized);

    for (const auto& it : serialized->issuer()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

bool Issuers::Load(
    const std::string& id,
    std::shared_ptr<proto::Issuer>& output,
    std::string& alias,
    const bool checking) const
{
    return load_proto<proto::Issuer>(id, output, alias, checking);
}

bool Issuers::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

proto::StorageIssuers Issuers::serialize() const
{
    proto::StorageIssuers serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_issuer());
        }
    }

    return serialized;
}

bool Issuers::Store(const proto::Issuer& data, const std::string& alias)
{
    return store_proto(data, data.id(), alias);
}
}  // namespace storage
}  // namespace opentxs
