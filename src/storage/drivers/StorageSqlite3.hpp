// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_STORAGE_SQLITE
namespace opentxs::storage::implementation
{
// SQLite3 implementation of opentxs::storage
class StorageSqlite3 final : public virtual Plugin,
                             public virtual opentxs::api::storage::Driver
{
public:
    bool EmptyBucket(const bool bucket) const final;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const final;
    std::string LoadRoot() const final;
    bool StoreRoot(const bool commit, const std::string& hash) const final;

    void Cleanup() final;
    void Cleanup_StorageSqlite3();

    ~StorageSqlite3() final;

private:
    typedef Plugin ot_super;

    friend Factory;

    std::string folder_;
    mutable std::mutex transaction_lock_;
    mutable OTFlag transaction_bucket_;
    mutable std::vector<std::pair<const std::string, const std::string>>
        pending_;
    sqlite3* db_{nullptr};

    std::string bind_key(
        const std::string& source,
        const std::string& key,
        const std::size_t start) const;
    void commit(std::stringstream& sql) const;
    bool commit_transaction(const std::string& rootHash) const;
    bool Create(const std::string& tablename) const;
    std::string expand_sql(sqlite3_stmt* statement) const;
    std::string GetTableName(const bool bucket) const;
    bool Select(
        const std::string& key,
        const std::string& tablename,
        std::string& value) const;
    bool Purge(const std::string& tablename) const;
    void set_data(std::stringstream& sql) const;
    void set_root(const std::string& rootHash, std::stringstream& sql) const;
    void start_transaction(std::stringstream& sql) const;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;
    bool Upsert(
        const std::string& key,
        const std::string& tablename,
        const std::string& value) const;

    void Init_StorageSqlite3();

    StorageSqlite3(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageSqlite3() = delete;
    StorageSqlite3(const StorageSqlite3&) = delete;
    StorageSqlite3(StorageSqlite3&&) = delete;
    StorageSqlite3& operator=(const StorageSqlite3&) = delete;
    StorageSqlite3& operator=(StorageSqlite3&&) = delete;
};
}  // namespace opentxs::storage::implementation
#endif  // OT_STORAGE_SQLITE
