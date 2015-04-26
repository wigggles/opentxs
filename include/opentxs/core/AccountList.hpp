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

#ifndef OPENTXS_CORE_ACCOUNTLIST_HPP
#define OPENTXS_CORE_ACCOUNTLIST_HPP

#include "Account.hpp" // only necessary because of OTAccount::AccountType
#include <string>
#include <map>
#include <memory>

namespace opentxs
{

class Nym;
class Tag;

// The server needs to store a list of accounts, by instrument definition ID, to
// store the
// backing funds for vouchers. The below class is useful for that. It's also
// useful for the same purpose for stashes, in smart contracts.
// Eventually will add expiration dates, possibly, to this class. (To have
// series, just like cash already does now.)
//
class AccountList
{
public:
    EXPORT AccountList();
    AccountList(Account::AccountType acctType);
    EXPORT ~AccountList();

    EXPORT int32_t GetCountAccountIDs() const
    {
        return static_cast<int32_t>(mapAcctIDs_.size());
    }

    EXPORT void Release();

    EXPORT void Release_AcctList();

    EXPORT void Serialize(Tag& parent) const;
    EXPORT int32_t ReadFromXMLNode(irr::io::IrrXMLReader*& xml,
                                   const String& acctType,
                                   const String& acctCount);

    void SetType(Account::AccountType acctType)
    {
        acctType_ = acctType;
    }

    EXPORT std::shared_ptr<Account> GetOrRegisterAccount(
        Nym& serverNym, const Identifier& ACCOUNT_OWNER_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID, const Identifier& NOTARY_ID,
        bool& wasAcctCreated, // this will be set to true if the acct is
        // created here. Otherwise set to false;
        int64_t stashTransNum = 0);

private:
    typedef std::map<std::string, std::weak_ptr<Account>> MapOfWeakAccounts;

private:
    Account::AccountType acctType_;
    // AcctIDs as second mapped by ASSET TYPE ID as first.
    String::Map mapAcctIDs_;
    // If someone calls GetOrRegisterAccount(), we pass them a shared pointer.
    // We
    // store the weak pointer here only to make sure accounts don't get loaded
    // twice.
    MapOfWeakAccounts mapWeakAccts_;
};

} // namespace opentxs

#endif // OPENTXS_CORE_ACCOUNTLIST_HPP
