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

#ifndef OPENTXS_STORAGE_STORAGEFSARCHIVE_HPP
#define OPENTXS_STORAGE_STORAGEFSARCHIVE_HPP

#include "opentxs/storage/StoragePlugin.hpp"

#include <atomic>
#include <memory>

namespace opentxs
{

class Storage;
class StorageConfig;

class StorageFSArchive : public virtual StoragePlugin_impl,
                         public virtual StorageDriver
{
private:
    typedef StoragePlugin_impl ot_super;

public:
    void Cleanup() override;
    bool EmptyBucket(const bool bucket) const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    std::string LoadRoot() const override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;
    bool StoreRoot(const std::string& hash) const override;

    ~StorageFSArchive();

private:
    friend class Storage;

    const std::string folder_{};
    const std::string path_seperator_{};
    std::atomic<bool> ready_{false};

    std::string calculate_path(const std::string& key) const;
    std::string read_file(const std::string& filename) const;
    bool write_file(const std::string& filename, const std::string& contents)
        const;

    void Init_StorageFSArchive();
    void Cleanup_StorageFSArchive();

    StorageFSArchive(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        std::atomic<bool>& bucket,
        const std::string& folder);
    StorageFSArchive() = delete;
    StorageFSArchive(const StorageFSArchive&) = delete;
    StorageFSArchive(StorageFSArchive&&) = delete;
    StorageFSArchive& operator=(const StorageFSArchive&) = delete;
    StorageFSArchive& operator=(StorageFSArchive&&) = delete;
};

}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEFSARCHIVE_HPP
