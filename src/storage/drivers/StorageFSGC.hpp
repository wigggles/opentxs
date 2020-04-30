// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/storage/drivers/StorageFSGC.cpp"

#pragma once

#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "storage/drivers/StorageFS.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Plugin;
class Storage;
}  // namespace storage
}  // namespace api

class Factory;
class Flag;
class StorageConfig;
}  // namespace opentxs

namespace opentxs::storage::implementation
{
// Simple filesystem implementation of opentxs::storage
class StorageFSGC final : public StorageFS,
                          public virtual opentxs::api::storage::Driver
{
private:
    typedef StorageFS ot_super;

public:
    bool EmptyBucket(const bool bucket) const final;

    void Cleanup() final;

    ~StorageFSGC() final;

private:
    friend Factory;

    std::string bucket_name(const bool bucket) const;
    std::string calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const final;
    void purge(const std::string& path) const;
    std::string root_filename() const final;

    void Cleanup_StorageFSGC();
    void Init_StorageFSGC();

    StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageFSGC() = delete;
    StorageFSGC(const StorageFSGC&) = delete;
    StorageFSGC(StorageFSGC&&) = delete;
    StorageFSGC& operator=(const StorageFSGC&) = delete;
    StorageFSGC& operator=(StorageFSGC&&) = delete;
};
}  // namespace opentxs::storage::implementation
