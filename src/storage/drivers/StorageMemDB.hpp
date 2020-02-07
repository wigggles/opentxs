// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::storage::implementation
{
// In-memory implementation of opentxs::storage
class StorageMemDB final : public Plugin,
                           virtual public opentxs::api::storage::Driver,
                           Lockable
{
public:
    bool EmptyBucket(const bool bucket) const final;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const final;
    std::string LoadRoot() const final;
    bool StoreRoot(const bool commit, const std::string& hash) const final;

    void Cleanup() final {}

    ~StorageMemDB() final = default;

private:
    using ot_super = Plugin;

    friend opentxs::Factory;

    mutable std::string root_{""};
    mutable std::map<std::string, std::string> a_{};
    mutable std::map<std::string, std::string> b_{};

    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;

    StorageMemDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageMemDB() = delete;
    StorageMemDB(const StorageMemDB&) = delete;
    StorageMemDB(StorageMemDB&&) = delete;
    StorageMemDB& operator=(const StorageMemDB&) = delete;
    StorageMemDB& operator=(StorageMemDB&&) = delete;
};
}  // namespace opentxs::storage::implementation
