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

#ifndef OPENTXS_STORAGE_STORAGEMULTIPLEX_HPP
#define OPENTXS_STORAGE_STORAGEMULTIPLEX_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <vector>

namespace opentxs
{
class StorageConfig;
class SymmetricKey;

namespace api
{
namespace storage
{
class Plugin;
class Storage;

namespace implementation
{
class Storage;
}  // namespace implementation
}  // namespace storage
}  // namespace api

namespace storage
{
class Root;
class Tree;
}  // namespace storage

class StorageMultiplex : virtual public opentxs::api::storage::Driver
{
public:
    bool EmptyBucket(const bool bucket) const override;
    bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const override;
    bool Load(const std::string& key, const bool checking, std::string& value)
        const override;
    std::string LoadRoot() const override;
    bool Migrate(
        const std::string& key,
        const opentxs::api::storage::Driver& to) const override;
    bool Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const override;
    void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const override;
    bool Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const override;
    bool StoreRoot(const bool commit, const std::string& hash) const override;

    ~StorageMultiplex();

private:
    friend class api::storage::implementation::Storage;

    const api::storage::Storage& storage_;
    const std::atomic<bool>& primary_bucket_;
    const StorageConfig& config_;
    std::unique_ptr<opentxs::api::storage::Plugin> primary_plugin_;
    std::vector<std::unique_ptr<opentxs::api::storage::Plugin>> backup_plugins_;
    const Digest digest_;
    const Random random_;

    StorageMultiplex(
        const api::storage::Storage& storage,
        const std::atomic<bool>& primary_bucket_,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random);
    StorageMultiplex() = delete;
    StorageMultiplex(const StorageMultiplex&) = delete;
    StorageMultiplex(StorageMultiplex&&) = delete;
    StorageMultiplex& operator=(const StorageMultiplex&) = delete;
    StorageMultiplex& operator=(StorageMultiplex&&) = delete;

    void Cleanup();
    void Cleanup_StorageMultiplex();
    void Init_StorageMultiplex();
    void InitBackup();
    void InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key);
    opentxs::api::storage::Driver& Primary();
    void synchronize_plugins(
        const std::string& hash,
        const storage::Root& root,
        const bool syncPrimary);
    std::string best_root(bool& primaryOutOfSync);
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEMULTIPLEX_HPP
