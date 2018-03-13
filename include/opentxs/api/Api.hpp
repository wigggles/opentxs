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

#ifndef OPENTXS_API_API_HPP
#define OPENTXS_API_API_HPP

#include "opentxs/Forward.hpp"

#include <mutex>
#include <string>

namespace opentxs
{
namespace api
{
class Sync;
class Api
{
public:
    EXPORT virtual std::recursive_mutex& Lock() const = 0;

    EXPORT virtual const OTAPI_Exec& Exec(
        const std::string& wallet = "") const = 0;
    EXPORT virtual const OT_API& OTAPI(
        const std::string& wallet = "") const = 0;
    EXPORT virtual const client::Cash& Cash() const = 0;
    EXPORT virtual const client::Pair& Pair() const = 0;
    EXPORT virtual const client::ServerAction& ServerAction() const = 0;
    EXPORT virtual const client::Sync& Sync() const = 0;

    EXPORT virtual ~Api() = default;

protected:
    Api() = default;

private:
    Api(const Api&) = delete;
    Api(Api&&) = delete;
    Api& operator=(const Api&) = delete;
    Api& operator=(Api&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_API_HPP
