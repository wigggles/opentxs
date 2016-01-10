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

// SQLite3 implementation of opentxs::storage
class StorageSqlite3 : public Storage
{
private:
    typedef Storage ot_super;

    friend Storage;

    std::string folder_ = "";
    ::sqlite3* db_;

    static const std::string primaryTable;
    static const std::string secondaryTable;
    static const std::string controlTable;
    static const std::string rootKey;

    static std::string GetTableName(const bool altLocation)
    {
        return altLocation ?
        StorageSqlite3::secondaryTable
        : StorageSqlite3::primaryTable;
    }

    StorageSqlite3() = delete;
    // param is interpreted to mean a full path to the folder where the
    // database file should be stored
    StorageSqlite3(
        const std::string& param,
        const Digest& hash,
        const Random& random);
    StorageSqlite3(StorageSqlite3 const&) = delete;
    StorageSqlite3& operator=(StorageSqlite3 const&) = delete;

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

    using ot_super::Init;
    void Init(const std::string& param);

public:
    std::string LoadRoot() override;
    bool StoreRoot(const std::string& hash) override;
    using ot_super::Load;
    bool Load(
        const std::string& key,
        std::string& value,
        const bool altLocation) override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool altLocation) override;
    bool EmptyBucket(const bool altLocation) override;

    void Cleanup() override;
    ~StorageSqlite3();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGESQLITE3_HPP
