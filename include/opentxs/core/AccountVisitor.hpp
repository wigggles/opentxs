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

#ifndef OPENTXS_CORE_ACCOUNTVISITOR_HPP
#define OPENTXS_CORE_ACCOUNTVISITOR_HPP

#include "opentxs/Forward.hpp"

#include <map>
#include <string>

namespace opentxs
{
class AccountVisitor
{
public:
    using mapOfAccounts = std::map<std::string, const Account*>;

    OTIdentifier GetNotaryID() const { return notaryID_; }

    virtual bool Trigger(const Account& account) = 0;

    virtual ~AccountVisitor() = default;

protected:
    const OTIdentifier notaryID_;
    mapOfAccounts* loadedAccounts_;

    AccountVisitor(const Identifier& notaryID);
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_ACCOUNTVISITOR_HPP
