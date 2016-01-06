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
#ifdef OT_STORAGE_SQLITE
#include <opentxs/storage/StorageSqlite3.hpp>

#include <string>

namespace opentxs
{

    const std::string StorageSqlite3::primaryTable = "a";
    const std::string StorageSqlite3::secondaryTable = "b";
    const std::string StorageSqlite3::controlTable = "control";
    const std::string StorageSqlite3::rootKey = "root";

StorageSqlite3::StorageSqlite3(const std::string& param, const Digest&hash)
    : ot_super(hash)
{
    Init(param);
}

bool StorageSqlite3::Select(
    const std::string& key,
    const std::string& tablename,
    std::string& value)
{
    sqlite3_stmt *statement;
    const std::string query =
        "select v from " + tablename + " where k=?1;";

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

bool StorageSqlite3::Upsert(
    const std::string& key,
    const std::string& tablename,
    const std::string& value)
{
    sqlite3_stmt *statement;
    const std::string query =
        "insert or replace into " + tablename +
        "(k, v) values (?1, ?2);";

    sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, 0);
    sqlite3_bind_text(statement, 1, key.c_str(), key.size(), SQLITE_STATIC);
    sqlite3_bind_blob(statement, 2, value.c_str(), value.size(), SQLITE_STATIC);
    int result = sqlite3_step(statement);
    sqlite3_finalize(statement);

    return (result == SQLITE_DONE);
}

void StorageSqlite3::Init(const std::string& param)
{
    folder_ = param;

    const std::string filename = folder_ + "/opentxs.sqlite3";
    const std::string tableFormat = " (k text PRIMARY KEY, v BLOB);";
    const std::string createTable = "create table if not exists ";
    const std::string primary =
        createTable + StorageSqlite3::primaryTable + tableFormat;
    const std::string secondary =
        createTable + StorageSqlite3::secondaryTable + tableFormat;
    const std::string control =
        createTable + StorageSqlite3::controlTable + tableFormat;

    if (SQLITE_OK == sqlite3_open(filename.c_str(), &db_)) {
        sqlite3_exec(db_, primary.c_str(), nullptr, nullptr, nullptr);
        sqlite3_exec(db_, secondary.c_str(), nullptr, nullptr, nullptr);
        sqlite3_exec(db_, control.c_str(), nullptr, nullptr, nullptr);
        sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    } else {
        std::cout << "Failed to initialize database." << std::endl;
        assert(false);
    }

}

std::string StorageSqlite3::LoadRoot()
{
    std::string value;
    if (Select(StorageSqlite3::rootKey, StorageSqlite3::controlTable, value)) {

        return value;
    }

    return "";
}

bool StorageSqlite3::Load(
    const std::string& key,
    std::string& value,
    const bool altLocation)
{
    const std::string tablename = altLocation ?
            StorageSqlite3::secondaryTable
            : StorageSqlite3::primaryTable;

    return Select(key, tablename, value);
}

bool StorageSqlite3::StoreRoot(const std::string& hash)
{
    return Upsert(StorageSqlite3::rootKey, StorageSqlite3::controlTable, hash);
}

bool StorageSqlite3::Store(
    const std::string& key,
    const std::string& value,
    const bool altLocation)
{
    const std::string tablename = altLocation ?
    StorageSqlite3::secondaryTable
    : StorageSqlite3::primaryTable;

    return Upsert(key, tablename, value);
}

void StorageSqlite3::Cleanup()
{
    sqlite3_close(db_);
}

StorageSqlite3::~StorageSqlite3()
{
    Cleanup();
}

} // namespace opentxs
#endif
