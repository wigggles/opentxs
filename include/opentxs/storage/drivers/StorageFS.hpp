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

#include "opentxs/storage/Storage.hpp"

namespace opentxs
{

class StorageConfig;

// Simple filesystem implementation of opentxs::storage
class StorageFS : public Storage
{
private:
    typedef Storage ot_super;

    friend Storage;

    std::string folder_;

    std::string GetBucketName(const bool bucket) const
    {
        return bucket ?
            config_.fs_secondary_bucket_ : config_.fs_primary_bucket_;
    }

    StorageFS() = delete;
    StorageFS(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random);
    StorageFS(const StorageFS&) = delete;
    StorageFS& operator=(const StorageFS&) = delete;

    void Init_StorageFS();
    void Purge(const std::string& path);

    void Cleanup_StorageFS();
public:
    std::string LoadRoot() const override;
    bool StoreRoot(const std::string& hash) override;
    using ot_super::Load;
    bool Load(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    using ot_super::Store;
    bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;
    bool EmptyBucket(const bool bucket) override;

    void Cleanup() override;
    ~StorageFS();
};

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGEFS_HPP
