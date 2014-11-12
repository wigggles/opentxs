/************************************************************
 *
 *  AccountList.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/AccountList.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/OTStorage.hpp>

#include "Helpers.hpp"

#include <irrxml/irrXML.hpp>

#include <cstring>
#include <string>
#include <utility>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

using namespace irr;
using namespace io;

namespace opentxs
{

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

void AccountList::Serialize(String& append) const
{
    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    append.Concatenate("<accountList type=\"%s\" count=\"%" PRI_SIZE "\" >\n\n",
                       acctType.Get(), mapAcctIDs_.size());

    for (auto& it : mapAcctIDs_) {
        std::string assetTypeId = it.first;
        std::string accountId = it.second;
        OT_ASSERT((assetTypeId.size() > 0) && (accountId.size() > 0));

        append.Concatenate(
            "<accountEntry assetTypeID=\"%s\" accountID=\"%s\" />\n\n",
            assetTypeId.c_str(), accountId.c_str());
    }

    append.Concatenate("</accountList>\n\n");
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
            if (!OTContract::SkipToElement(xml)) {
                otOut
                    << "AccountList::ReadFromXMLNode: Failure: Unable to find "
                       "expected element.\n";
                return -1;
            }

            if ((xml->getNodeType() == EXN_ELEMENT) &&
                (!strcmp("accountEntry", xml->getNodeName()))) {
                String assetTypeID = xml->getAttributeValue(
                    "assetTypeID"); // Asset Type ID of this account.
                String accountID = xml->getAttributeValue(
                    "accountID"); // Account ID for this account.

                if (!assetTypeID.Exists() || !accountID.Exists()) {
                    otErr << "Error loading accountEntry: Either the "
                             "assetTypeID (" << assetTypeID
                          << "), or the accountID (" << accountID
                          << ") was EMPTY.\n";
                    return -1;
                }

                mapAcctIDs_.insert(
                    std::make_pair(assetTypeID.Get(), accountID.Get()));
            }
            else {
                otErr << "Expected accountEntry element in accountList.\n";
                return -1;
            }
        }
    }

    if (!OTContract::SkipAfterLoadingField(xml)) // </accountList>
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

std::shared_ptr<Account> AccountList::GetOrCreateAccount(
    OTPseudonym& serverNym, const OTIdentifier& accountOwnerId,
    const OTIdentifier& assetTypeId, const OTIdentifier& serverId,
    // this will be set to true if the acct is created here.
    // Otherwise set to false;
    bool& wasAcctCreated, int64_t stashTransNum)
{
    std::shared_ptr<Account> account;
    wasAcctCreated = false;

    if (Account::stash == acctType_) {
        if (stashTransNum <= 0) {
            otErr
                << "AccountList::GetOrCreateAccount: Failed attempt to create "
                   "stash account without cron item #.\n";
            return account;
        }
    }

    // First, we'll see if there's already an account ID available for the
    // requested asset type ID.
    std::string assetTypeIdString = String(assetTypeId).Get();

    String acctTypeString;
    TranslateAccountTypeToString(acctType_, acctTypeString);

    auto acctIDsIt = mapAcctIDs_.find(assetTypeIdString);
    // Account ID *IS* already there for this asset type
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
                    otOut
                        << "AccountList::GetOrCreateAccount: Warning: account ("
                        << accountIdString
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
        OTIdentifier accountID(acctIDString);

        // The Account ID exists, but we don't have the pointer to a loaded
        // account for it. So, let's load it.
        Account* loadedAccount =
            Account::LoadExistingAccount(accountID, serverId);

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
                   << " Asset Type ID: " << assetTypeIdString << "\n";

            account = std::shared_ptr<Account>(loadedAccount);
            // save a weak pointer to the acct, so we'll never load it twice,
            // but we'll also know if it's been deleted.
            mapWeakAccts_[acctIDString.Get()] = std::weak_ptr<Account>(account);
        }
        return account;
    }

    // Not found. There's no account ID yet for that asset type ID. That means
    // we can create it.
    Message message;
    accountOwnerId.GetString(message.m_strNymID);
    assetTypeId.GetString(message.m_strAssetID);
    serverId.GetString(message.m_strServerID);

    Account* createdAccount = Account::GenerateNewAccount(
        accountOwnerId, serverId, serverNym, message, acctType_, stashTransNum);

    if (!createdAccount) {
        otErr << " AccountList::GetOrCreateAccount: Failed trying to generate"
              << acctTypeString
              << " account with asset type ID: " << assetTypeIdString << "\n";
    }
    else {
        String acctIDString;
        createdAccount->GetIdentifier(acctIDString);

        otOut << "Successfully created " << acctTypeString
              << " account ID: " << acctIDString
              << " Asset Type ID: " << assetTypeIdString << "\n";

        account = std::shared_ptr<Account>(createdAccount);

        // save a weak pointer to the acct, so we'll never load it twice,
        // but we'll also know if it's been deleted.
        mapWeakAccts_[acctIDString.Get()] = std::weak_ptr<Account>(account);
        // Save the new acct ID in a map, keyed by asset type ID.
        mapAcctIDs_[message.m_strAssetID.Get()] = acctIDString.Get();

        wasAcctCreated = true;
    }

    return account;
}

} // namespace opentxs
