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

#include "opentxs/core/AccountList.hpp"

#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Helpers.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"

#include <irrxml/irrXML.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

using namespace irr;
using namespace io;

namespace opentxs
{

class Nym;

AccountList::AccountList()
    : acctType_(Account::voucher)
{
}

AccountList::AccountList(Account::AccountType acctType)
    : acctType_(acctType)
{
}

AccountList::~AccountList()
{
    Release_AcctList();
}

void AccountList::Serialize(Tag& parent) const
{
    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    uint32_t sizeMapAcctIDs = mapAcctIDs_.size();

    TagPtr pTag(new Tag("accountList"));

    pTag->add_attribute("type", acctType.Get());
    pTag->add_attribute("count", formatUint(sizeMapAcctIDs));

    for (auto& it : mapAcctIDs_) {
        std::string instrumentDefinitionID = it.first;
        std::string accountId = it.second;
        OT_ASSERT((instrumentDefinitionID.size() > 0) &&
                  (accountId.size() > 0));

        TagPtr pTagEntry(new Tag("accountEntry"));

        pTagEntry->add_attribute("instrumentDefinitionID",
                                 instrumentDefinitionID);
        pTagEntry->add_attribute("accountID", accountId);

        pTag->add_tag(pTagEntry);
    }

    parent.add_tag(pTag);
}

int32_t AccountList::ReadFromXMLNode(irr::io::IrrXMLReader*& xml,
                                     const String& acctType,
                                     const String& acctCount)
{
    if (!acctType.Exists()) {
        otErr << "AccountList::ReadFromXMLNode: Failed: Empty accountList "
                 "'type' attribute.\n";
        return -1;
    }

    acctType_ = TranslateAccountTypeStringToEnum(acctType);

    if (Account::err_acct == acctType_) {
        otErr << "AccountList::ReadFromXMLNode: Failed: accountList 'type' "
                 "attribute contains unknown value.\n";
        return -1;
    }

    // Load up the account IDs.
    int32_t count = acctCount.Exists() ? atoi(acctCount.Get()) : 0;
    if (count > 0) {
        while (count-- > 0) {
            if (!Contract::SkipToElement(xml)) {
                otOut
                    << "AccountList::ReadFromXMLNode: Failure: Unable to find "
                       "expected element.\n";
                return -1;
            }

            if ((xml->getNodeType() == EXN_ELEMENT) &&
                (!strcmp("accountEntry", xml->getNodeName()))) {
                String instrumentDefinitionID = xml->getAttributeValue(
                    "instrumentDefinitionID"); // Instrument Definition ID of
                                               // this account.
                String accountID = xml->getAttributeValue(
                    "accountID"); // Account ID for this account.

                if (!instrumentDefinitionID.Exists() || !accountID.Exists()) {
                    otErr << "Error loading accountEntry: Either the "
                             "instrumentDefinitionID ("
                          << instrumentDefinitionID << "), or the accountID ("
                          << accountID << ") was EMPTY.\n";
                    return -1;
                }

                mapAcctIDs_.insert(std::make_pair(instrumentDefinitionID.Get(),
                                                  accountID.Get()));
            }
            else {
                otErr << "Expected accountEntry element in accountList.\n";
                return -1;
            }
        }
    }

    if (!Contract::SkipAfterLoadingField(xml)) // </accountList>
    {
        otOut << "*** AccountList::ReadFromXMLNode: Bad data? Expected "
                 "EXN_ELEMENT_END here, but "
                 "didn't get it. Returning false.\n";
        return -1;
    }

    return 1;
}

void AccountList::Release_AcctList()
{
    mapAcctIDs_.clear();
    mapWeakAccts_.clear();
}

void AccountList::Release()
{
    Release_AcctList();
}

std::shared_ptr<Account> AccountList::GetOrRegisterAccount(
    Nym& serverNym, const Identifier& accountOwnerId,
    const Identifier& instrumentDefinitionID, const Identifier& notaryID,
    // this will be set to true if the acct is created here.
    // Otherwise set to false;
    bool& wasAcctCreated, int64_t stashTransNum)
{
    std::shared_ptr<Account> account;
    wasAcctCreated = false;

    if (Account::stash == acctType_) {
        if (stashTransNum <= 0) {
            otErr << "AccountList::GetOrRegisterAccount: Failed attempt to "
                     "create "
                     "stash account without cron item #.\n";
            return account;
        }
    }

    // First, we'll see if there's already an account ID available for the
    // requested instrument definition ID.
    std::string instrumentDefinitionIDString =
        String(instrumentDefinitionID).Get();

    String acctTypeString;
    TranslateAccountTypeToString(acctType_, acctTypeString);

    auto acctIDsIt = mapAcctIDs_.find(instrumentDefinitionIDString);
    // Account ID *IS* already there for this instrument definition
    if (mapAcctIDs_.end() != acctIDsIt) {
        // grab account ID
        std::string accountIdString = acctIDsIt->second;
        auto weakIt = mapWeakAccts_.find(accountIdString);

        // FOUND the weak ptr to the account! Maybe it's already loaded
        if (mapWeakAccts_.end() != weakIt) {
            try {
                std::shared_ptr<Account> weakAccount(weakIt->second);

                // If success, then we have a shared pointer. But it's worrying
                // (TODO) because this should have
                // gone out of scope and been destroyed by whoever ELSE was
                // using it. The fact that it's still here...
                // well I'm glad not to double-load it, but I wonder why it's
                // still here? And we aren't walking on anyone's
                // toes, right? If this were multi-threaded, then I'd explicitly
                // lock a mutex here, honestly. But since things
                // happen one at a time on OT, I'll settle for a warning for
                // now. I'm assuming that if the account's loaded
                // already somewhere, it's just a pointer sitting there, and
                // we're not walking on each other's toes.
                if (weakAccount) {
                    otOut << "AccountList::GetOrRegisterAccount: Warning: "
                             "account (" << accountIdString
                          << ") was already in memory so I gave you a "
                             "pointer to the existing one. (But who else has a "
                             "copy of it?) \n";
                    return weakAccount;
                }
            }
            catch (...) {
            }

            // Though the weak pointer was there, the resource must have since
            // been destroyed, because I cannot lock a new shared ptr onto it.
            // Therefore remove it from the map, and RE-LOAD IT.
            mapWeakAccts_.erase(weakIt);
        }

        // DIDN'T find the acct pointer, even though we had the ID.
        // (Or it was there, but we couldn't lock a shared_ptr onto it, so we
        // erased it...)
        // So let's load it now. After all, the Account ID *does* exist...
        String acctIDString(accountIdString.c_str());
        Identifier accountID(acctIDString);

        // The Account ID exists, but we don't have the pointer to a loaded
        // account for it. So, let's load it.
        Account* loadedAccount =
            Account::LoadExistingAccount(accountID, notaryID);

        if (!loadedAccount) {
            otErr << "Failed trying to load " << acctTypeString
                  << " account with account ID: " << acctIDString << '\n';
        }
        else if (!loadedAccount->VerifySignature(serverNym)) {
            otErr << "Failed verifying server's signature on " << acctTypeString
                  << " account with account ID: " << acctIDString << '\n';
        }
        else if (!loadedAccount->VerifyOwnerByID(accountOwnerId)) {
            String strOwnerID(accountOwnerId);
            otErr << "Failed verifying owner ID (" << strOwnerID << ") on "
                  << acctTypeString << " account ID: " << acctIDString << '\n';
        }
        else {
            otLog3 << "Successfully loaded " << acctTypeString
                   << " account ID: " << acctIDString
                   << " Instrument Definition ID: "
                   << instrumentDefinitionIDString << "\n";

            account = std::shared_ptr<Account>(loadedAccount);
            // save a weak pointer to the acct, so we'll never load it twice,
            // but we'll also know if it's been deleted.
            mapWeakAccts_[acctIDString.Get()] = std::weak_ptr<Account>(account);
        }
        return account;
    }

    // Not found. There's no account ID yet for that instrument definition ID.
    // That means
    // we can create it.
    Message message;
    accountOwnerId.GetString(message.m_strNymID);
    instrumentDefinitionID.GetString(message.m_strInstrumentDefinitionID);
    notaryID.GetString(message.m_strNotaryID);

    Account* createdAccount = Account::GenerateNewAccount(
        accountOwnerId, notaryID, serverNym, message, acctType_, stashTransNum);

    if (!createdAccount) {
        otErr << " AccountList::GetOrRegisterAccount: Failed trying to generate"
              << acctTypeString << " account with instrument definition ID: "
              << instrumentDefinitionIDString << "\n";
    }
    else {
        String acctIDString;
        createdAccount->GetIdentifier(acctIDString);

        otOut << "Successfully created " << acctTypeString
              << " account ID: " << acctIDString
              << " Instrument Definition ID: " << instrumentDefinitionIDString
              << "\n";

        account = std::shared_ptr<Account>(createdAccount);

        // save a weak pointer to the acct, so we'll never load it twice,
        // but we'll also know if it's been deleted.
        mapWeakAccts_[acctIDString.Get()] = std::weak_ptr<Account>(account);
        // Save the new acct ID in a map, keyed by instrument definition ID.
        mapAcctIDs_[message.m_strInstrumentDefinitionID.Get()] =
            acctIDString.Get();

        wasAcctCreated = true;
    }

    return account;
}

} // namespace opentxs
