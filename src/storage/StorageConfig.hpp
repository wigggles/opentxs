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

#ifndef OPENTXS_STORAGE_STORAGECONFIG_HPP
#define OPENTXS_STORAGE_STORAGECONFIG_HPP

#include "Internal.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

#define OT_STORAGE_PRIMARY_PLUGIN_SQLITE "sqlite"
#define OT_STORAGE_PRIMARY_PLUGIN_FS "fs"
#define STORAGE_CONFIG_PRIMARY_PLUGIN_KEY "primary_plugin"
#define STORAGE_CONFIG_FS_BACKUP_DIRECTORY_KEY "fs_backup_directory"
#define STORAGE_CONFIG_FS_ENCRYPTED_BACKUP_DIRECTORY_KEY "fs_encrypted_backup"

namespace C = std::chrono;

namespace opentxs
{

typedef std::function<void(const std::string&, const std::string&)> InsertCB;

class StorageConfig
{
public:
    bool auto_publish_nyms_ = true;
    bool auto_publish_servers_ = true;
    bool auto_publish_units_ = true;
    std::int64_t gc_interval_ =
        C::duration_cast<C::seconds>(C::hours(1)).count();
    std::string path_{};
    InsertCB dht_callback_{};

#if OT_STORAGE_SQLITE
    std::string primary_plugin_ = OT_STORAGE_PRIMARY_PLUGIN_SQLITE;
#elif OT_STORAGE_FS
    std::string primary_plugin_ = OT_STORAGE_PRIMARY_PLUGIN_FS;
#else
    std::string primary_plugin_{};
#endif

#ifdef OT_STORAGE_FS
    std::string fs_primary_bucket_ = "a";
    std::string fs_secondary_bucket_ = "b";
    std::string fs_root_file_ = "root";
    std::string fs_backup_directory_{""};
    std::string fs_encrypted_backup_directory_{""};
#endif

#ifdef OT_STORAGE_SQLITE
    std::string sqlite3_primary_bucket_ = "a";
    std::string sqlite3_secondary_bucket_ = "b";
    std::string sqlite3_control_table_ = "control";
    std::string sqlite3_root_key_ = "a";
    std::string sqlite3_db_file_ = "opentxs.sqlite3";
#endif
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGECONFIG_HPP
