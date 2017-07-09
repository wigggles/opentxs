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

#ifndef OPENTXS_STORAGE_STORAGEDRIVER_HPP
#define OPENTXS_STORAGE_STORAGEDRIVER_HPP

#include <memory>
#include <string>

namespace opentxs
{
class StorageDriver
{
public:
    virtual bool EmptyBucket(const bool bucket) const = 0;

    virtual bool Load(
        const std::string& key,
        const bool checking,
        std::string& value) const = 0;
    virtual bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const = 0;

    virtual bool Store(
        const std::string& key,
        const std::string& value,
        const bool bucket) const = 0;
    virtual bool Store(const std::string& value, std::string& key) const = 0;

    virtual bool Migrate(const std::string& key, const StorageDriver& to)
        const = 0;

    virtual std::string LoadRoot() const = 0;
    virtual bool StoreRoot(const std::string& hash) const = 0;

    virtual ~StorageDriver() = default;

    template <class T>
    bool LoadProto(
        const std::string& hash,
        std::shared_ptr<T>& serialized,
        const bool checking = false) const;

    template <class T>
    bool StoreProto(const T& data, std::string& key, std::string& plaintext)
        const;

    template <class T>
    bool StoreProto(const T& data, std::string& key) const;

    template <class T>
    bool StoreProto(const T& data) const;

protected:
    StorageDriver() = default;

private:
    StorageDriver(const StorageDriver&) = delete;
    StorageDriver(StorageDriver&&) = delete;
    StorageDriver& operator=(const StorageDriver&) = delete;
    StorageDriver& operator=(StorageDriver&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEDRIVER_HPP
