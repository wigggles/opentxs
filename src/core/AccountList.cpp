// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/AccountList.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Helpers.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>
#include <sys/types.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

using namespace irr;
using namespace io;

#define OT_METHOD "opentxs::AccountList::"

namespace opentxs
{
AccountList::AccountList(const api::Core& core)
    : api_(core)
    , acctType_(Account::voucher)
    , mapAcctIDs_{}
{
}

AccountList::AccountList(const api::Core& core, Account::AccountType acctType)
    : api_(core)
    , acctType_(acctType)
    , mapAcctIDs_{}
{
}

void AccountList::Serialize(Tag& parent) const
{
    String acctType;
    TranslateAccountTypeToString(acctType_, acctType);

    std::uint32_t sizeMapAcctIDs = mapAcctIDs_.size();

    TagPtr pTag(new Tag("accountList"));

    pTag->add_attribute("type", acctType.Get());
    pTag->add_attribute("count", formatUint(sizeMapAcctIDs));

    for (auto& it : mapAcctIDs_) {
        std::string instrumentDefinitionID = it.first;
        std::string accountId = it.second;
        OT_ASSERT(
            (instrumentDefinitionID.size() > 0) && (accountId.size() > 0));

        TagPtr pTagEntry(new Tag("accountEntry"));

        pTagEntry->add_attribute(
            "instrumentDefinitionID", instrumentDefinitionID);
        pTagEntry->add_attribute("accountID", accountId);

        pTag->add_tag(pTagEntry);
    }

    parent.add_tag(pTag);
}

std::int32_t AccountList::ReadFromXMLNode(
    irr::io::IrrXMLReader*& xml,
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
    std::int32_t count = acctCount.Exists() ? atoi(acctCount.Get()) : 0;
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
                    "instrumentDefinitionID");  // Instrument Definition ID of
                                                // this account.
                String accountID = xml->getAttributeValue(
                    "accountID");  // Account ID for this account.

                if (!instrumentDefinitionID.Exists() || !accountID.Exists()) {
                    otErr << "Error loading accountEntry: Either the "
                             "instrumentDefinitionID ("
                          << instrumentDefinitionID << "), or the accountID ("
                          << accountID << ") was EMPTY.\n";
                    return -1;
                }

                mapAcctIDs_.insert(std::make_pair(
                    instrumentDefinitionID.Get(), accountID.Get()));
            } else {
                otErr << "Expected accountEntry element in accountList.\n";
                return -1;
            }
        }
    }

    if (!Contract::SkipAfterLoadingField(xml))  // </accountList>
    {
        otOut << "*** AccountList::ReadFromXMLNode: Bad data? Expected "
                 "EXN_ELEMENT_END here, but "
                 "didn't get it. Returning false.\n";
        return -1;
    }

    return 1;
}

void AccountList::Release_AcctList() { mapAcctIDs_.clear(); }

void AccountList::Release() { Release_AcctList(); }

ExclusiveAccount AccountList::GetOrRegisterAccount(
    const Nym& serverNym,
    const Identifier& accountOwnerId,
    const Identifier& instrumentDefinitionID,
    const Identifier& notaryID,
    bool& wasAcctCreated,
    std::int64_t stashTransNum)
{
    ExclusiveAccount account{};
    wasAcctCreated = false;

    if (Account::stash == acctType_) {
        if (1 > stashTransNum) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed attempt to "
                     "create stash account without cron item #.";

            return {};
        }
    }

    // First, we'll see if there's already an account ID available for the
    // requested instrument definition ID.
    String acctTypeString;
    TranslateAccountTypeToString(acctType_, acctTypeString);
    auto acctIDsIt = mapAcctIDs_.find(instrumentDefinitionID.str());

    // Account ID *IS* already there for this instrument definition
    if (mapAcctIDs_.end() != acctIDsIt) {
        const auto& accountID = acctIDsIt->second;
        account = api_.Wallet().mutable_Account(Identifier::Factory(accountID));

        if (account) {

            LogDebug(OT_METHOD)(__FUNCTION__)(": Successfully loaded ")(
                acctTypeString)(" account ID: ")(accountID)("Unit Type ID:: ")(
                instrumentDefinitionID.str())
                .Flush();

            return account;
        }
    }

    // Not found. There's no account ID yet for that instrument definition ID.
    // That means we can create it.
    account = api_.Wallet().CreateAccount(
        accountOwnerId,
        notaryID,
        instrumentDefinitionID,
        serverNym,
        acctType_,
        stashTransNum);

    if (false == bool(account)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed trying to generate"
              << acctTypeString << " account with instrument definition ID: "
              << instrumentDefinitionID.str() << std::endl;
    } else {
        String acctIDString;
        account.get().GetIdentifier(acctIDString);

        otOut << "Successfully created " << acctTypeString
              << " account ID: " << acctIDString
              << " Instrument Definition ID: " << instrumentDefinitionID.str()
              << std::endl;
        mapAcctIDs_[instrumentDefinitionID.str()] = acctIDString.Get();

        wasAcctCreated = true;
    }

    return account;
}

AccountList::~AccountList() { Release_AcctList(); }
}  // namespace opentxs
