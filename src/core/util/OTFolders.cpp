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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/OTPaths.hpp"

#ifdef _WIN32
#include <direct.h>
#include <shlobj.h>
#endif

#ifndef _WIN32
#include <libgen.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef TARGET_OS_MAC
#include <mach-o/dyld.h>
#include <limits.h>
#endif

#define DEFAULT_ACCOUNT "accounts"
#define DEFAULT_CERT "certs"
#define DEFAULT_COMMON "common"
#define DEFAULT_CONTRACT "contracts"
#define DEFAULT_CRON "cron"
#define DEFAULT_INBOX "inbox"
#define DEFAULT_MARKET "markets"
#define DEFAULT_MINT "mints"
#define DEFAULT_NYM "nyms"
#define DEFAULT_NYMBOX "nymbox"
#define DEFAULT_OUTBOX "outbox"
#define DEFAULT_PAYMENTINBOX "paymentInbox"
#define DEFAULT_PURSE "purse"
#define DEFAULT_RECEIPT "receipts"
#define DEFAULT_RECORDBOX "recordBox"
#define DEFAULT_EXPIREDBOX "expiredBox"
#define DEFAULT_SCRIPT "scripts"
#define DEFAULT_SMARTCONTRACTS "smartcontracts"
#define DEFAULT_SPENT "spent"
#define DEFAULT_USERACCT "useraccounts"

#define KEY_ACCOUNT "account"
#define KEY_CERT "cert"
#define KEY_COMMON "common"
#define KEY_CONTRACT "contract"
#define KEY_CRON "cron"
#define KEY_INBOX "inbox"
#define KEY_MARKET "market"
#define KEY_MINT "mint"
#define KEY_NYM "nym"
#define KEY_NYMBOX "nymbox"
#define KEY_OUTBOX "outbox"
#define KEY_PAYMENTINBOX "paymentinbox"
#define KEY_PURSE "purse"
#define KEY_RECEIPT "receipt"
#define KEY_RECORDBOX "recordbox"
#define KEY_EXPIREDBOX "expiredbox"
#define KEY_SCRIPT "script"
#define KEY_SMARTCONTRACTS "smartcontracts"
#define KEY_SPENT "spent"
#define KEY_USERACCT "useracct"

namespace opentxs
{

String OTFolders::s_strAccount("");
String OTFolders::s_strCert("");
String OTFolders::s_strCommon("");
String OTFolders::s_strContract("");
String OTFolders::s_strCron("");
String OTFolders::s_strInbox("");
String OTFolders::s_strMarket("");
String OTFolders::s_strMint("");
String OTFolders::s_strNym("");
String OTFolders::s_strNymbox("");
String OTFolders::s_strOutbox("");
String OTFolders::s_strPaymentInbox("");
String OTFolders::s_strPurse("");
String OTFolders::s_strReceipt("");
String OTFolders::s_strRecordBox("");
String OTFolders::s_strExpiredBox("");
String OTFolders::s_strScript("");
String OTFolders::s_strSmartContracts("");
String OTFolders::s_strSpent("");
String OTFolders::s_strUserAcct("");

bool OTFolders::GetSetAll()
{
    Settings config(OTPaths::GlobalConfigFile());

    config.Reset();

    if (!config.Load()) return false;

    if (!GetSetFolderName(config, KEY_ACCOUNT, DEFAULT_ACCOUNT, s_strAccount))
        return false;
    if (!GetSetFolderName(config, KEY_CERT, DEFAULT_CERT, s_strCert))
        return false;
    if (!GetSetFolderName(config, KEY_COMMON, DEFAULT_COMMON, s_strCommon))
        return false;
    if (!GetSetFolderName(config, KEY_CONTRACT, DEFAULT_CONTRACT,
                          s_strContract))
        return false;
    if (!GetSetFolderName(config, KEY_CRON, DEFAULT_CRON, s_strCron))
        return false;
    if (!GetSetFolderName(config, KEY_INBOX, DEFAULT_INBOX, s_strInbox))
        return false;
    if (!GetSetFolderName(config, KEY_MARKET, DEFAULT_MARKET, s_strMarket))
        return false;
    if (!GetSetFolderName(config, KEY_MINT, DEFAULT_MINT, s_strMint))
        return false;
    if (!GetSetFolderName(config, KEY_NYM, DEFAULT_NYM, s_strNym)) return false;
    if (!GetSetFolderName(config, KEY_NYMBOX, DEFAULT_NYMBOX, s_strNymbox))
        return false;
    if (!GetSetFolderName(config, KEY_OUTBOX, DEFAULT_OUTBOX, s_strOutbox))
        return false;
    if (!GetSetFolderName(config, KEY_PAYMENTINBOX, DEFAULT_PAYMENTINBOX,
                          s_strPaymentInbox))
        return false;
    if (!GetSetFolderName(config, KEY_PURSE, DEFAULT_PURSE, s_strPurse))
        return false;
    if (!GetSetFolderName(config, KEY_RECEIPT, DEFAULT_RECEIPT, s_strReceipt))
        return false;
    if (!GetSetFolderName(config, KEY_RECORDBOX, DEFAULT_RECORDBOX,
                          s_strRecordBox))
        return false;
    if (!GetSetFolderName(config, KEY_EXPIREDBOX, DEFAULT_EXPIREDBOX,
                          s_strExpiredBox))
        return false;
    if (!GetSetFolderName(config, KEY_SCRIPT, DEFAULT_SCRIPT, s_strScript))
        return false;
    if (!GetSetFolderName(config, KEY_SMARTCONTRACTS, DEFAULT_SMARTCONTRACTS,
                          s_strSmartContracts))
        return false;
    if (!GetSetFolderName(config, KEY_SPENT, DEFAULT_SPENT, s_strSpent))
        return false;
    if (!GetSetFolderName(config, KEY_USERACCT, DEFAULT_USERACCT,
                          s_strUserAcct))
        return false;

    if (!config.Save()) return false;

    config.Reset();

    return true;
}

const String& OTFolders::Account()
{
    return GetFolder(s_strAccount);
}
const String& OTFolders::Cert()
{
    return GetFolder(s_strCert);
}
const String& OTFolders::Common()
{
    return GetFolder(s_strCommon);
}
const String& OTFolders::Contract()
{
    return GetFolder(s_strContract);
}
const String& OTFolders::Cron()
{
    return GetFolder(s_strCron);
}
const String& OTFolders::Inbox()
{
    return GetFolder(s_strInbox);
}
const String& OTFolders::Market()
{
    return GetFolder(s_strMarket);
}
const String& OTFolders::Mint()
{
    return GetFolder(s_strMint);
}
const String& OTFolders::Nym()
{
    return GetFolder(s_strNym);
}
const String& OTFolders::Nymbox()
{
    return GetFolder(s_strNymbox);
}
const String& OTFolders::Outbox()
{
    return GetFolder(s_strOutbox);
}
const String& OTFolders::PaymentInbox()
{
    return GetFolder(s_strPaymentInbox);
}
const String& OTFolders::Purse()
{
    return GetFolder(s_strPurse);
}
const String& OTFolders::Receipt()
{
    return GetFolder(s_strReceipt);
}
const String& OTFolders::RecordBox()
{
    return GetFolder(s_strRecordBox);
}
const String& OTFolders::ExpiredBox()
{
    return GetFolder(s_strExpiredBox);
}
const String& OTFolders::Script()
{
    return GetFolder(s_strScript);
}
const String& OTFolders::SmartContracts()
{
    return GetFolder(s_strSmartContracts);
}
const String& OTFolders::Spent()
{
    return GetFolder(s_strSpent);
}
const String& OTFolders::UserAcct()
{
    return GetFolder(s_strUserAcct);
}

} // namespace opentxs
