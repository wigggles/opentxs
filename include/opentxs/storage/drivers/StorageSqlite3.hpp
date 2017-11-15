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

#include "opentxs/Version.hpp"

#if OT_STORAGE_SQLITE

#include "opentxs/storage/Plugin.hpp"

extern "C" {
#include <sqlite3.h>
}

#include <atomic>
#include <mutex>
#include <tuple>
#include <vector>

namespace opentxs
{
class StorageConfig;
class StorageMultiplex;

// SQLite3 implementation of opentxs::storage
class StorageSqlite3 : public virtual Plugin,
                       public virtual opentxs::api::storage::Driver
{
public:
    bool EmptyBucket(const bool bucket) const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    std::string LoadRoot() const override;
    bool StoreRoot(const bool commit, const std::string& hash) const override;

    void Cleanup() override;
    void Cleanup_StorageSqlite3();

    ~StorageSqlite3();

private:
    typedef Plugin ot_super;

    friend class StorageMultiplex;

    std::string folder_;
    mutable std::mutex transaction_lock_;
    mutable std::atomic<bool> transaction_bucket_;
    mutable std::vector<std::pair<const std::string, const std::string>>
        pending_;
    sqlite3* db_{nullptr};

    std::string GetTableName(const bool bucket) const;

    bool commit() const;
    bool commit_transaction(const std::string& rootHash) const;
    bool Create(const std::string& tablename) const;
    bool Select(
        const std::string& key,
        const std::string& tablename,
        std::string& value) const;
    bool Purge(const std::string& tablename) const;
    void rollback() const;
    bool set_data() const;
    bool set_root(const std::string& rootHash) const;
    bool start_transaction() const;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const override;
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
        const std::atomic<bool>& bucket);
    StorageSqlite3() = delete;
    StorageSqlite3(const StorageSqlite3&) = delete;
    StorageSqlite3(StorageSqlite3&&) = delete;
    StorageSqlite3& operator=(const StorageSqlite3&) = delete;
    StorageSqlite3& operator=(StorageSqlite3&&) = delete;
};
}  // namespace opentxs
#endif  // OT_STORAGE_SQLITE
#endif  // OPENTXS_STORAGE_STORAGESQLITE3_HPP
