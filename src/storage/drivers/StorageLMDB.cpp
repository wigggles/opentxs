// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_STORAGE_LMDB
#include "opentxs/core/Log.hpp"

#include "storage/Plugin.hpp"
#include "storage/StorageConfig.hpp"

#include "lmdb.h"

extern "C" {
#include <sys/stat.h>
}

#include <string>
#include <vector>

#include "StorageLMDB.hpp"

#define OT_LMDB_TABLES 3
#define OT_LMDB_SIZE 1UL * 1024UL * 1024UL * 1024UL

#define OT_METHOD "opentxs::StorageLMDB::"

namespace opentxs
{
opentxs::api::storage::Plugin* Factory::StorageLMDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
{
    return new opentxs::storage::implementation::StorageLMDB(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageLMDB::StorageLMDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , environment_(nullptr)
    , databases_()
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Using ")(config_.path_).Flush();
    Init_StorageLMDB();
}

void StorageLMDB::Cleanup() { Cleanup_StorageLMDB(); }

void StorageLMDB::Cleanup_StorageLMDB()
{
    if (nullptr != environment_) {
        mdb_env_close(environment_);
        environment_ = nullptr;
    }
}

bool StorageLMDB::EmptyBucket(const bool bucket) const
{
    MDB_txn* transaction{nullptr};
    bool status = 0 == mdb_txn_begin(environment_, nullptr, 0, &transaction);

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    auto& database = get_database(get_folder(bucket));
    status = 0 == mdb_drop(transaction, database, 0);
    mdb_txn_commit(transaction);

    return status;
}

MDB_dbi& StorageLMDB::get_database(const Table table) const
{
    return databases_[static_cast<std::size_t>(table)];
}

StorageLMDB::Table StorageLMDB::get_folder(const bool bucket) const
{
    return (bucket) ? Table::A : Table::B;
}

std::string StorageLMDB::get_key(const std::string& input, const Table table)
    const
{
    std::string output{};
    MDB_txn* transaction{nullptr};
    bool status =
        0 == mdb_txn_begin(environment_, nullptr, MDB_RDONLY, &transaction);

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    auto& database{get_database(table)};
    MDB_val key, value;
    key.mv_size = input.size();
    key.mv_data = const_cast<char*>(input.c_str());
    status = 0 == mdb_get(transaction, database, &key, &value);

    if (status) {
        output.assign(static_cast<char*>(value.mv_data), value.mv_size);
    }

    mdb_txn_abort(transaction);

    return output;
}

MDB_dbi StorageLMDB::init_db(const std::string& table)
{
    MDB_txn* transaction{nullptr};
    bool status = 0 == mdb_txn_begin(environment_, nullptr, 0, &transaction);

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    MDB_dbi database;
    status =
        0 == mdb_dbi_open(transaction, table.c_str(), MDB_CREATE, &database);

    OT_ASSERT(status);

    mdb_txn_commit(transaction);

    return database;
}

void StorageLMDB::init_environment(const std::string& folder)
{
    bool set = 0 == mdb_env_create(&environment_);

    OT_ASSERT(set);
    OT_ASSERT(nullptr != environment_);

    set = 0 == mdb_env_set_mapsize(environment_, OT_LMDB_SIZE);

    OT_ASSERT(set);

    set = 0 == mdb_env_set_maxdbs(environment_, OT_LMDB_TABLES);

    OT_ASSERT(set);

    set = 0 == mdb_env_open(environment_, folder.c_str(), 0, 0664);

    OT_ASSERT(set);
}

void StorageLMDB::Init_StorageLMDB()
{
    init_environment(config_.path_);
    databases_.emplace_back(init_db(config_.lmdb_control_table_));
    databases_.emplace_back(init_db(config_.lmdb_primary_bucket_));
    databases_.emplace_back(init_db(config_.lmdb_secondary_bucket_));
    LogOutput(OT_METHOD)(__FUNCTION__)(": Database initialized.").Flush();
}

bool StorageLMDB::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    value = get_key(key, get_folder(bucket));

    return false == value.empty();
}

std::string StorageLMDB::LoadRoot() const
{
    return get_key(config_.lmdb_root_key_, Table::Control);
}

void StorageLMDB::store(
    [[maybe_unused]] const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    const auto output = store_key(key, get_folder(bucket), value);
    promise->set_value(output);
}

bool StorageLMDB::store_key(
    const std::string& index,
    const Table table,
    const std::string& input) const
{
    MDB_txn* transaction{nullptr};
    bool status = 0 == mdb_txn_begin(environment_, nullptr, 0, &transaction);

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    auto& database{get_database(table)};
    MDB_val key, value;
    key.mv_size = index.size();
    key.mv_data = const_cast<char*>(index.c_str());
    value.mv_size = input.size();
    value.mv_data = const_cast<char*>(input.c_str());
    status = 0 == mdb_put(transaction, database, &key, &value, 0);
    mdb_txn_commit(transaction);

    return status;
}

bool StorageLMDB::StoreRoot(
    [[maybe_unused]] const bool commit,
    const std::string& hash) const
{
    return store_key(config_.lmdb_root_key_, Table::Control, hash);
}

StorageLMDB::~StorageLMDB() { Cleanup_StorageLMDB(); }
}  // namespace opentxs::storage::implementation
#endif
