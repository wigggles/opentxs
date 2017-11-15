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

#ifndef OPENTXS_API_STORAGE_PLUGIN_HPP
#define OPENTXS_API_STORAGE_PLUGIN_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/storage/Driver.hpp"

#include <string>

namespace opentxs
{
namespace api
{
namespace storage
{

class Plugin : public virtual Driver
{
public:
    virtual bool EmptyBucket(const bool bucket) const = 0;

    virtual std::string LoadRoot() const = 0;

    virtual bool StoreRoot(const bool commit, const std::string& hash)
        const = 0;

    virtual ~Plugin() = default;

protected:
    Plugin() = default;

private:
    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(Plugin&&) = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_STORAGE_PLUGIN_HPP
