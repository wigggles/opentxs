// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#if OT_STORAGE_FS

#include "StorageFS.hpp"

#include <memory>

namespace opentxs
{
class StorageFSArchive : public StorageFS,
                         public virtual opentxs::api::storage::Driver
{
private:
    typedef StorageFS ot_super;

public:
    bool EmptyBucket(const bool bucket) const override;

    void Cleanup() override;

    ~StorageFSArchive();

private:
    friend class StorageMultiplex;

    crypto::key::Symmetric& encryption_key_;
    const bool encrypted_{false};

    std::string calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const override;
    std::string prepare_read(const std::string& ciphertext) const override;
    std::string prepare_write(const std::string& plaintext) const override;
    std::string root_filename() const override;

    void Init_StorageFSArchive();
    void Cleanup_StorageFSArchive();

    StorageFSArchive(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket,
        const std::string& folder,
        crypto::key::Symmetric& key);
    StorageFSArchive() = delete;
    StorageFSArchive(const StorageFSArchive&) = delete;
    StorageFSArchive(StorageFSArchive&&) = delete;
    StorageFSArchive& operator=(const StorageFSArchive&) = delete;
    StorageFSArchive& operator=(StorageFSArchive&&) = delete;
};
}  // namespace opentxs

#endif  // OT_STORAGE_FS
