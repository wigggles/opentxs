// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/util/OTFolders.hpp"

#include "opentxs/core/String.hpp"

#include "opentxs/core/util/OTPaths.hpp"

#include "Internal.hpp"

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
#define KEY_SPENT "spent"
#define KEY_USERACCT "useracct"

namespace opentxs
{

OTString OTFolders::s_strAccount(String::Factory());
OTString OTFolders::s_strCert(String::Factory());
OTString OTFolders::s_strCommon(String::Factory());
OTString OTFolders::s_strContract(String::Factory());
OTString OTFolders::s_strCron(String::Factory());
OTString OTFolders::s_strInbox(String::Factory());
OTString OTFolders::s_strMarket(String::Factory());
OTString OTFolders::s_strMint(String::Factory());
OTString OTFolders::s_strNym(String::Factory());
OTString OTFolders::s_strNymbox(String::Factory());
OTString OTFolders::s_strOutbox(String::Factory());
OTString OTFolders::s_strPaymentInbox(String::Factory());
OTString OTFolders::s_strPurse(String::Factory());
OTString OTFolders::s_strReceipt(String::Factory());
OTString OTFolders::s_strRecordBox(String::Factory());
OTString OTFolders::s_strExpiredBox(String::Factory());
OTString OTFolders::s_strSpent(String::Factory());
OTString OTFolders::s_strUserAcct(String::Factory());

bool OTFolders::GetSetAll()
{
    std::unique_ptr<api::Settings> config{
        Factory::Settings(OTPaths::GlobalConfigFile())};
    config->Reset();

    if (!config->Load()) return false;

    if (!GetSetFolderName(*config, KEY_ACCOUNT, DEFAULT_ACCOUNT, s_strAccount))
        return false;
    if (!GetSetFolderName(*config, KEY_CERT, DEFAULT_CERT, s_strCert))
        return false;
    if (!GetSetFolderName(*config, KEY_COMMON, DEFAULT_COMMON, s_strCommon))
        return false;
    if (!GetSetFolderName(
            *config, KEY_CONTRACT, DEFAULT_CONTRACT, s_strContract))
        return false;
    if (!GetSetFolderName(*config, KEY_CRON, DEFAULT_CRON, s_strCron))
        return false;
    if (!GetSetFolderName(*config, KEY_INBOX, DEFAULT_INBOX, s_strInbox))
        return false;
    if (!GetSetFolderName(*config, KEY_MARKET, DEFAULT_MARKET, s_strMarket))
        return false;
    if (!GetSetFolderName(*config, KEY_MINT, DEFAULT_MINT, s_strMint))
        return false;
    if (!GetSetFolderName(*config, KEY_NYM, DEFAULT_NYM, s_strNym))
        return false;
    if (!GetSetFolderName(*config, KEY_NYMBOX, DEFAULT_NYMBOX, s_strNymbox))
        return false;
    if (!GetSetFolderName(*config, KEY_OUTBOX, DEFAULT_OUTBOX, s_strOutbox))
        return false;
    if (!GetSetFolderName(
            *config, KEY_PAYMENTINBOX, DEFAULT_PAYMENTINBOX, s_strPaymentInbox))
        return false;
    if (!GetSetFolderName(*config, KEY_PURSE, DEFAULT_PURSE, s_strPurse))
        return false;
    if (!GetSetFolderName(*config, KEY_RECEIPT, DEFAULT_RECEIPT, s_strReceipt))
        return false;
    if (!GetSetFolderName(
            *config, KEY_RECORDBOX, DEFAULT_RECORDBOX, s_strRecordBox))
        return false;
    if (!GetSetFolderName(
            *config, KEY_EXPIREDBOX, DEFAULT_EXPIREDBOX, s_strExpiredBox))
        return false;
    if (!GetSetFolderName(*config, KEY_SPENT, DEFAULT_SPENT, s_strSpent))
        return false;
    if (!GetSetFolderName(
            *config, KEY_USERACCT, DEFAULT_USERACCT, s_strUserAcct))
        return false;

    if (!config->Save()) return false;

    config->Reset();

    return true;
}

const String& OTFolders::Account() { return GetFolder(s_strAccount); }
const String& OTFolders::Cert() { return GetFolder(s_strCert); }
const String& OTFolders::Common() { return GetFolder(s_strCommon); }
const String& OTFolders::Contract() { return GetFolder(s_strContract); }
const String& OTFolders::Cron() { return GetFolder(s_strCron); }
const String& OTFolders::Inbox() { return GetFolder(s_strInbox); }
const String& OTFolders::Market() { return GetFolder(s_strMarket); }
const String& OTFolders::Mint() { return GetFolder(s_strMint); }
const String& OTFolders::Nym() { return GetFolder(s_strNym); }
const String& OTFolders::Nymbox() { return GetFolder(s_strNymbox); }
const String& OTFolders::Outbox() { return GetFolder(s_strOutbox); }
const String& OTFolders::PaymentInbox() { return GetFolder(s_strPaymentInbox); }
const String& OTFolders::Purse() { return GetFolder(s_strPurse); }
const String& OTFolders::Receipt() { return GetFolder(s_strReceipt); }
const String& OTFolders::RecordBox() { return GetFolder(s_strRecordBox); }
const String& OTFolders::ExpiredBox() { return GetFolder(s_strExpiredBox); }
const String& OTFolders::Spent() { return GetFolder(s_strSpent); }
const String& OTFolders::UserAcct() { return GetFolder(s_strUserAcct); }

}  // namespace opentxs
