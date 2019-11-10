// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"

#include "storage/Plugin.hpp"
#include "storage/StorageConfig.hpp"

#include <string>

#include "StorageMemDB.hpp"

//#define OT_METHOD "opentxs::StorageMemDB::"

namespace opentxs
{
opentxs::api::storage::Plugin* Factory::StorageMemDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
{
    return new opentxs::storage::implementation::StorageMemDB(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageMemDB::StorageMemDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , root_("")
    , a_()
    , b_()
{
}

bool StorageMemDB::EmptyBucket(const bool bucket) const
{
    eLock lock(shared_lock_);

    if (bucket) {
        a_.clear();
    } else {
        b_.clear();
    }

    return true;
}

bool StorageMemDB::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    sLock lock(shared_lock_);

    try {
        if (bucket) {
            value = a_.at(key);
        } else {
            value = b_.at(key);
        }
    } catch (...) {

        return false;
    }

    return (false == value.empty());
}

std::string StorageMemDB::LoadRoot() const
{
    sLock lock(shared_lock_);

    return root_;
}

void StorageMemDB::store(
    [[maybe_unused]] const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (bucket) {
        a_[key] = value;
    } else {
        b_[key] = value;
    }

    promise->set_value(true);
}

bool StorageMemDB::StoreRoot(
    [[maybe_unused]] const bool commit,
    const std::string& hash) const
{
    eLock lock(shared_lock_);
    root_ = hash;

    return true;
}
}  // namespace opentxs::storage::implementation
