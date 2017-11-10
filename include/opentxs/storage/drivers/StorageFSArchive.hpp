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

#include "opentxs/Version.hpp"

#if OT_STORAGE_FS

#include "opentxs/storage/drivers/StorageFS.hpp"

#include <memory>

namespace opentxs
{

class StorageConfig;
class StorageMultiplex;
class SymmetricKey;

class StorageFSArchive : public StorageFS, public virtual StorageDriver
{
private:
    typedef StorageFS ot_super;

public:
    bool EmptyBucket(const bool bucket) const override;

    void Cleanup() override;

    ~StorageFSArchive();

private:
    friend class StorageMultiplex;

    const std::unique_ptr<SymmetricKey> encryption_key_;
    const bool encrypted_{false};

    std::string calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const override;
    std::string prepare_read(const std::string& ciphertext) const override;
    std::string prepare_write(const std::string& plaintext) const override;
    std::string root_filename() const override;

    void Init_StorageFSArchive();
    void Cleanup_StorageFSArchive();

    StorageFSArchive(
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const std::atomic<bool>& bucket,
        const std::string& folder,
        std::unique_ptr<SymmetricKey>& key);
    StorageFSArchive() = delete;
    StorageFSArchive(const StorageFSArchive&) = delete;
    StorageFSArchive(StorageFSArchive&&) = delete;
    StorageFSArchive& operator=(const StorageFSArchive&) = delete;
    StorageFSArchive& operator=(StorageFSArchive&&) = delete;
};
}  // namespace opentxs

#endif  // OT_STORAGE_FS
#endif  // OPENTXS_STORAGE_STORAGEFSARCHIVE_HPP
