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

#ifndef OPENTXS_STORAGE_STORAGEFS_HPP
#define OPENTXS_STORAGE_STORAGEFS_HPP

#include "opentxs/storage/StoragePlugin.hpp"

namespace opentxs
{

class Storage;
class StorageConfig;

// Simple filesystem implementation of opentxs::storage
class StorageFS : public virtual StoragePlugin_impl,
                  public virtual StorageDriver
{
private:
    typedef StoragePlugin_impl ot_super;

    friend class Storage;

    std::string folder_;

    std::string GetBucketName(const bool bucket) const;

    void Init_StorageFS();
    void Purge(const std::string& path) const;
    std::string read_file(const std::string& filename) const;
    bool write_file(const std::string& filename, const std::string& contents)
        const;

    void Cleanup_StorageFS();

    StorageFS(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        std::atomic<bool>& bucket);
    StorageFS() = delete;
    StorageFS(const StorageFS&) = delete;
    StorageFS(StorageFS&&) = delete;
    StorageFS& operator=(const StorageFS&) = delete;
    StorageFS& operator=(StorageFS&&) = delete;

public:
    std::string LoadRoot() const override;
    bool StoreRoot(const std::string& hash) const override;

    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;

    bool EmptyBucket(const bool bucket) const override;

    void Cleanup() override;
    ~StorageFS();
};

}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEFS_HPP
