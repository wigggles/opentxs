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

#ifndef OPENTXS_STORAGE_STORAGEFSGC_HPP
#define OPENTXS_STORAGE_STORAGEFSGC_HPP

#include "opentxs/Forward.hpp"

#if OT_STORAGE_FS

#include "opentxs/storage/drivers/StorageFS.hpp"

namespace opentxs
{

class StorageConfig;
class StorageMultiplex;

// Simple filesystem implementation of opentxs::storage
class StorageFSGC : public StorageFS,
                    public virtual opentxs::api::storage::Driver
{
private:
    typedef StorageFS ot_super;

public:
    bool EmptyBucket(const bool bucket) const override;

    void Cleanup() override;

    ~StorageFSGC();

private:
    friend class StorageMultiplex;

    std::string bucket_name(const bool bucket) const;
    std::string calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const override;
    void purge(const std::string& path) const;
    std::string root_filename() const override;

    void Cleanup_StorageFSGC();
    void Init_StorageFSGC();

    StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const std::atomic<bool>& bucket);
    StorageFSGC() = delete;
    StorageFSGC(const StorageFSGC&) = delete;
    StorageFSGC(StorageFSGC&&) = delete;
    StorageFSGC& operator=(const StorageFSGC&) = delete;
    StorageFSGC& operator=(StorageFSGC&&) = delete;
};
}  // namespace opentxs

#endif  // OT_STORAGE_FS
#endif  // OPENTXS_STORAGE_STORAGEFSGC_HPP
