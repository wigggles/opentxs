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

#ifndef OPENTXS_CORE_OTACCTFUNCTOR_HPP
#define OPENTXS_CORE_OTACCTFUNCTOR_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Identifier.hpp"

#include <map>
#include <string>

namespace opentxs
{

class Account;

typedef std::map<std::string, Account*> mapOfAccounts;

class AccountVisitor
{
public:
    EXPORT AccountVisitor(
        const Identifier& notaryID,
        mapOfAccounts* loadedAccounts = nullptr)
        : notaryID_(notaryID)
        , loadedAccounts_(loadedAccounts)
    {
    }

    EXPORT virtual ~AccountVisitor() {}

    EXPORT Identifier* GetNotaryID() { return &notaryID_; }

    EXPORT mapOfAccounts* GetLoadedAccts() { return loadedAccounts_; }

    EXPORT virtual bool Trigger(Account& account) = 0;

protected:
    Identifier notaryID_;
    mapOfAccounts* loadedAccounts_;
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTACCTFUNCTOR_HPP
