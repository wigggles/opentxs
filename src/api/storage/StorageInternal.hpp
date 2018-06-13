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

#ifndef OPENTXS_API_STORAGE_STORAGEINTERNAL_HPP
#define OPENTXS_API_STORAGE_STORAGEINTERNAL_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/storage/Storage.hpp"

namespace opentxs::api::storage
{
class StorageInternal : virtual public Storage
{
public:
    virtual void InitBackup() = 0;
    virtual void InitEncryptedBackup(std::unique_ptr<SymmetricKey>& key) = 0;
    virtual void start() = 0;

    virtual ~StorageInternal() override = default;

protected:
    StorageInternal() = default;

private:
    StorageInternal(const StorageInternal&) = delete;
    StorageInternal(StorageInternal&&) = delete;
    StorageInternal& operator=(const StorageInternal&) = delete;
    StorageInternal& operator=(StorageInternal&&) = delete;
};
}  // namespace opentxs::api::storage
#endif  // OPENTXS_API_STORAGE_STORAGEINTERNAL_HPP
