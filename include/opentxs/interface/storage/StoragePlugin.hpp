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

#ifndef OPENTXS_STORAGE_STORAGEPLUGIN_HPP
#define OPENTXS_STORAGE_STORAGEPLUGIN_HPP

#include "opentxs/interface/storage/StorageDriver.hpp"

#include <string>

namespace opentxs
{
class StoragePlugin
    : public virtual StorageDriver
{
public:
    virtual bool EmptyBucket(const bool bucket) const = 0;

    virtual std::string LoadRoot() const = 0;

    virtual bool StoreRoot(const std::string& hash) const = 0;

    virtual ~StoragePlugin() = default;

protected:
    StoragePlugin() = default;

private:
    StoragePlugin(const StoragePlugin&) = delete;
    StoragePlugin(StoragePlugin&&) = delete;
    StoragePlugin& operator=(const StoragePlugin&) = delete;
    StoragePlugin& operator=(StoragePlugin&&) = delete;
};
}  // namespace opentxs
#endif  // OPENTXS_STORAGE_STORAGEPLUGIN_HPP
