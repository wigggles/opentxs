/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/
#include "opentxs/stdafx.hpp"

#include "opentxs/storage/drivers/StorageSqlite3.hpp"

#if OT_STORAGE_SQLITE
#include "opentxs/core/Log.hpp"
#include "opentxs/storage/Storage.hpp"
#include "opentxs/storage/StorageConfig.hpp"

#include <sqlite3.h>

#include <iostream>
#include <string>
#include <sstream>

#define OT_METHOD "opentxs::StorageSqlite3::"

namespace opentxs
{
StorageSqlite3::StorageSqlite3(
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const std::atomic<bool>& bucket)
    : ot_super(config, hash, random, bucket)
    , folder_(config.path_)
    , transaction_lock_()
    , transaction_bucket_(false)
    , pending_()
    , db_(nullptr)
{
    Init_StorageSqlite3();
}

void StorageSqlite3::Cleanup() { Cleanup_StorageSqlite3(); }

void StorageSqlite3::Cleanup_StorageSqlite3() { sqlite3_close(db_); }

bool StorageSqlite3::commit() const
{
    sqlite3_stmt* close{nullptr};
    const std::string closeSQL{"COMMIT TRANSACTION;"};
    sqlite3_prepare_v2(db_, closeSQL.c_str(), -1, &close, 0);
    const auto output = sqlite3_step(close);
    sqlite3_finalize(close);

    return (SQLITE_DONE == output);
}

bool StorageSqlite3::commit_transaction(const std::string& rootHash) const
{
    Lock lock(transaction_lock_);
    bool output{false};

    if (start_transaction()) {
        if (set_data()) {
            if (set_root(rootHash)) {
                if (commit()) {
                    output = true;
                } else {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Failed to commit transaction." << std::endl;
                    rollback();
                }
            } else {
                otErr << OT_METHOD << __FUNCTION__ << ": Failed to update root."
                      << std::endl;
                rollback();
            }
        } else {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to update data."
                  << std::endl;
            rollback();
        }
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to start transaction."
              << std::endl;
    }

    pending_.clear();

    return output;
}

bool StorageSqlite3::Create(const std::string& tablename) const
{
    const std::string createTable = "create table if not exists ";
    const std::string tableFormat = " (k text PRIMARY KEY, v BLOB);";
    const std::string sql = createTable + "`" + tablename + "`" + tableFormat;

    return (
        SQLITE_OK == sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr));
}

bool StorageSqlite3::EmptyBucket(const bool bucket) const
{
    return Purge(GetTableName(bucket));
}

std::string StorageSqlite3::GetTableName(const bool bucket) const
{
    return bucket ? config_.sqlite3_secondary_bucket_
                  : config_.sqlite3_primary_bucket_;
}

void StorageSqlite3::Init_StorageSqlite3()
{
    const std::string filename = folder_ + "/" + config_.sqlite3_db_file_;

    if (SQLITE_OK ==
        sqlite3_open_v2(
            filename.c_str(),
            &db_,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
            nullptr)) {
        Create(config_.sqlite3_primary_bucket_);
        Create(config_.sqlite3_secondary_bucket_);
        Create(config_.sqlite3_control_table_);
        sqlite3_exec(
            db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    } else {
        otErr << OT_METHOD << __FUNCTION__ << "Failed to initialize database."
              << std::endl;

        OT_FAIL
    }
}

bool StorageSqlite3::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const
{
    return Select(key, GetTableName(bucket), value);
}

std::string StorageSqlite3::LoadRoot() const
{
    std::string value{""};

    if (Select(
            config_.sqlite3_root_key_, config_.sqlite3_control_table_, value)) {

        return value;
    }

    return "";
}

bool StorageSqlite3::Purge(const std::string& tablename) const
{
    const std::string sql = "DROP TABLE `" + tablename + "`;";

    if (SQLITE_OK ==
        sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr)) {
        return Create(tablename);
    }

    return false;
}

void StorageSqlite3::rollback() const
{
    sqlite3_stmt* rollback{nullptr};
    const std::string rollbackSQL{"ROLLBACK;"};
    sqlite3_prepare_v2(db_, rollbackSQL.c_str(), -1, &rollback, 0);
    sqlite3_step(rollback);
    sqlite3_finalize(rollback);
}

bool StorageSqlite3::Select(
    const std::string& key,
    const std::string& tablename,
    std::string& value) const
{
    sqlite3_stmt* statement{nullptr};
    const std::string query =
        "select v from `" + tablename + "` where k=?1 LIMIT 0,1;";
    sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
    sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_STATIC);
    int result = sqlite3_step(statement);
    bool success = false;

    if (result == SQLITE_ROW) {
        const void* pResult = sqlite3_column_blob(statement, 0);
        uint32_t size = sqlite3_column_bytes(statement, 0);
        value.assign(static_cast<const char*>(pResult), size);
        success = true;
    }

    sqlite3_finalize(statement);

    return success;
}

bool StorageSqlite3::set_data() const
{
    sqlite3_stmt* data{nullptr};
    std::stringstream dataSQL{};
    const std::string tablename{GetTableName(transaction_bucket_.load())};
    dataSQL << "INSERT OR REPLACE INTO '" << tablename << "' (k, v) VALUES ";
    std::size_t counter{0};
    const auto size = pending_.size();

    for (std::size_t i = 0; i < size; ++i) {
        dataSQL << "(?" << ++counter << ", ?";
        dataSQL << ++counter << ")";

        if (counter < (2 * size)) {
            dataSQL << ", ";
        } else {
            dataSQL << "; ";
        }
    }

    sqlite3_prepare_v2(db_, dataSQL.str().c_str(), -1, &data, 0);
    counter = 0;

    for (const auto& it : pending_) {
        const auto& key = it.first;
        const auto& value = it.second;
        auto bound = sqlite3_bind_text(
            data, ++counter, key.c_str(), key.size(), SQLITE_STATIC);

        OT_ASSERT(SQLITE_OK == bound);

        bound = sqlite3_bind_blob(
            data, ++counter, value.c_str(), value.size(), SQLITE_STATIC);

        OT_ASSERT(SQLITE_OK == bound);
    }

    const auto output = sqlite3_step(data);
    sqlite3_finalize(data);

    return (SQLITE_DONE == output);
}

bool StorageSqlite3::set_root(const std::string& rootHash) const
{
    sqlite3_stmt* root{nullptr};
    std::stringstream rootSQL{};
    rootSQL << "INSERT OR REPLACE INTO '" << config_.sqlite3_control_table_
            << "'  (k, v) VALUES (?1, ?2);";
    sqlite3_prepare_v2(db_, rootSQL.str().c_str(), -1, &root, 0);
    auto bound = sqlite3_bind_text(
        root,
        1,
        config_.sqlite3_root_key_.c_str(),
        config_.sqlite3_root_key_.size(),
        SQLITE_STATIC);

    OT_ASSERT(SQLITE_OK == bound)

    bound = sqlite3_bind_blob(
        root, 2, rootHash.c_str(), rootHash.size(), SQLITE_STATIC);

    OT_ASSERT(SQLITE_OK == bound)

    const auto output = sqlite3_step(root);
    sqlite3_finalize(root);

    return (SQLITE_DONE == output);
}

bool StorageSqlite3::start_transaction() const
{
    sqlite3_stmt* open{nullptr};
    const std::string openSQL{"BEGIN TRANSACTION;"};
    sqlite3_prepare_v2(db_, openSQL.c_str(), -1, &open, 0);
    const auto output = sqlite3_step(open);
    sqlite3_finalize(open);

    return (SQLITE_DONE == output);
}

void StorageSqlite3::store(
    const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (isTransaction) {
        Lock lock(transaction_lock_);
        transaction_bucket_.store(bucket);
        pending_.emplace_back(key, value);
        promise->set_value(true);
    } else {
        promise->set_value(Upsert(key, GetTableName(bucket), value));
    }
}

bool StorageSqlite3::StoreRoot(const bool commit, const std::string& hash) const
{
    if (commit) {

        return commit_transaction(hash);
    } else {

        return Upsert(
            config_.sqlite3_root_key_, config_.sqlite3_control_table_, hash);
    }
}

bool StorageSqlite3::Upsert(
    const std::string& key,
    const std::string& tablename,
    const std::string& value) const
{
    sqlite3_stmt* statement;
    const std::string query =
        "insert or replace into `" + tablename + "` (k, v) values (?1, ?2);";

    sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
    sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_STATIC);
    sqlite3_bind_blob(statement, 2, value.c_str(), value.size(), SQLITE_STATIC);
    int result = sqlite3_step(statement);
    sqlite3_finalize(statement);

    return (result == SQLITE_DONE);
}

StorageSqlite3::~StorageSqlite3() { Cleanup_StorageSqlite3(); }
}  // namespace opentxs
#endif
