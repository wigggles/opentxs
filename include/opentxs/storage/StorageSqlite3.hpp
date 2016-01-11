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

#ifndef OPENTXS_STORAGE_STORAGESQLITE3_HPP
#define OPENTXS_STORAGE_STORAGESQLITE3_HPP

#include <opentxs/storage/Storage.hpp>


extern "C"
{
    #include <sqlite3.h>
}

namespace opentxs
{

class StorageConfig;

// SQLite3 implementation of opentxs::storage
class StorageSqlite3 : public Storage
{
private:
    typedef Storage ot_super;

    friend Storage;

    StorageConfig config_;
    std::string folder_;
    sqlite3* db_ = nullptr;

    std::string GetTableName(const bool bucket)
    {
        return bucket ?
        config_.sqlite3_secondary_bucket_
        : config_.sqlite3_primary_bucket_;
    }

    StorageSqlite3() = delete;
    StorageSqlite3(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random);
    StorageSqlite3(const StorageSqlite3&) = delete;
    StorageSqlite3& operator=(const StorageSqlite3&) = delete;

    bool Select(
        const std::string& key,
        const std::string& tablename,
        std::string& value);
    bool Upsert(
        const std::string& key,
        const std::string& tablename,
        const std::string& value);
    bool Create(const std::string& tablename);
    bool Purge(const std::string& tablename);

    void Init_StorageSqlite3();

public:
    std::string LoadRoot() override;
    bool StoreRoot(const std::string& hash) override;
    using ot_super::Load;
    bool Load(
        const std::string& key,
        std::string& value,
        const bool bucket) override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) override;
    bool EmptyBucket(const bool bucket) override;

    void Cleanup_StorageSqlite3();
    void Cleanup() override;
    ~StorageSqlite3();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGESQLITE3_HPP
