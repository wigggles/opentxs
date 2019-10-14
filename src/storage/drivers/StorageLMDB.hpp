// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_STORAGE_LMDB
namespace opentxs::storage::implementation
{
// LMDB implementation of opentxs::storage
class StorageLMDB final : public virtual Plugin,
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
    void Cleanup_StorageLMDB();

    ~StorageLMDB() final;

private:
    using ot_super = Plugin;
    using Databases = std::vector<MDB_dbi>;

    friend Factory;

    enum class Table : std::size_t {
        Control = 0,
        A = 1,
        B = 2,
    };

    mutable MDB_env* environment_;
    mutable Databases databases_;

    Table get_folder(const bool bucket) const;
    MDB_dbi& get_database(const Table table) const;
    std::string get_key(const std::string& key, const Table table) const;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;
    bool store_key(
        const std::string& key,
        const Table table,
        const std::string& value) const;

    MDB_dbi init_db(const std::string& table);
    void init_environment(const std::string& folder);
    void Init_StorageLMDB();

    StorageLMDB(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageLMDB() = delete;
    StorageLMDB(const StorageLMDB&) = delete;
    StorageLMDB(StorageLMDB&&) = delete;
    StorageLMDB& operator=(const StorageLMDB&) = delete;
    StorageLMDB& operator=(StorageLMDB&&) = delete;
};
}  // namespace opentxs::storage::implementation
#endif  // OT_STORAGE_LMDB
