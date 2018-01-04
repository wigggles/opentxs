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

#ifndef OPENTXS_API_CLIENT_PAIR_HPP
#define OPENTXS_API_CLIENT_PAIR_HPP

#include "opentxs/Version.hpp"

#include <cstdint>
#include <set>
#include <string>

namespace opentxs
{
class Identifier;

namespace api
{
namespace client
{

class Pair
{
public:
    virtual bool AddIssuer(
        const Identifier& localNymID,
        const Identifier& issuerNymID,
        const std::string& pairingCode) const = 0;
    virtual std::string IssuerDetails(
        const Identifier& localNymID,
        const Identifier& issuerNymID) const = 0;
    virtual std::set<Identifier> IssuerList(
        const Identifier& localNymID,
        const bool onlyTrusted) const = 0;

    virtual ~Pair() = default;

protected:
    Pair() = default;

private:
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    Pair& operator=(const Pair&) = delete;
    Pair& operator=(Pair&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_PAIR_HPP
