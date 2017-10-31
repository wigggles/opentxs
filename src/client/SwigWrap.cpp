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

#include "opentxs/client/SwigWrap.hpp"

#include "opentxs/api/Activity.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Blockchain.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTME_too.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/ZMQ.hpp"
#include "opentxs/storage/Storage.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>

#ifndef OT_BOOL
#define OT_BOOL int32_t
#endif

#define DEFAULT_NODE_NAME "Stash Node Pro"

#define OT_METHOD "opentxs::SwigWrap::"

namespace opentxs
{
bool SwigWrap::networkFailure(const std::string& notaryID)
{
    return ConnectionState::ACTIVE != OT::App().ZMQ().Status(notaryID);
}

OTAPI_Exec* SwigWrap::Exec() { return &OT::App().API().Exec(); }

bool SwigWrap::AppInit(
    const std::uint64_t gcInterval,
    const std::string& storagePlugin,
    const std::string& archiveDirectory,
    const std::string& encryptedDirectory)
{
    OT::Factory(
        false,
        std::chrono::seconds(gcInterval),
        storagePlugin,
        archiveDirectory,
        encryptedDirectory);

    return true;
}

bool SwigWrap::AppRecover(
    const std::string& words,
    const std::string& passphrase,
    const std::uint64_t gcInterval,
    const std::string& storagePlugin,
    const std::string& archiveDirectory,
    const std::string& encryptedDirectory)
{
    OT::Factory(
        true,
        words,
        passphrase,
        false,
        std::chrono::seconds(gcInterval),
        storagePlugin,
        archiveDirectory,
        encryptedDirectory);

    return true;
}

bool SwigWrap::AppCleanup()
{
    OT::Cleanup();

    return true;
}

// SetAppBinaryFolder
// OPTIONAL. Used in Android and Qt.
//
// Certain platforms use this to override the Prefix folder.
// Basically /usr/local is the prefix folder by default, meaning
// /usr/local/lib/opentxs will be the location of the scripts. But
// if you override AppBinary folder to, say, "res/raw"
// (Android does something like that) then even though the prefix remains
// as /usr/local, the scripts folder will be res/raw
void SwigWrap::SetAppBinaryFolder(const std::string& strFolder)
{
    OTAPI_Exec::SetAppBinaryFolder(strFolder.c_str());
    ;
}

// SetHomeFolder
// OPTIONAL. Used in Android.
//
// The AppDataFolder, such as /Users/au/.ot, is constructed from the home
// folder, such as /Users/au.
//
// Normally the home folder is auto-detected, but certain platforms, such as
// Android, require us to explicitly set this folder from the Java code. Then
// the AppDataFolder is constructed from it. (It's the only way it can be done.)
//
// In Android, you would SetAppBinaryFolder to the path to
// "/data/app/packagename/res/raw",
// and you would SetHomeFolder to "/data/data/[app package]/files/"
void SwigWrap::SetHomeFolder(const std::string& strFolder)
{
    OTAPI_Exec::SetHomeFolder(strFolder.c_str());
}

OT_API* SwigWrap::OTAPI() { return &OT::App().API().OTAPI(); }

int64_t SwigWrap::StringToLong(const std::string& strNumber)
{
    return Exec()->StringToLong(strNumber);
}

std::string SwigWrap::LongToString(const int64_t& lNumber)
{
    return Exec()->LongToString(lNumber);
}

uint64_t SwigWrap::StringToUlong(const std::string& strNumber)
{
    return Exec()->StringToUlong(strNumber);
}

std::string SwigWrap::UlongToString(const uint64_t& lNumber)
{
    return Exec()->UlongToString(lNumber);
}

bool SwigWrap::CheckSetConfigSection(
    const std::string& strSection,
    const std::string& strComment)
{
    return Exec()->CheckSetConfigSection(strSection, strComment);
}

std::string SwigWrap::GetConfig_str(
    const std::string& strSection,
    const std::string& strKey)
{
    return Exec()->GetConfig_str(strSection, strKey);
}

int64_t SwigWrap::GetConfig_long(
    const std::string& strSection,
    const std::string& strKey)
{
    return Exec()->GetConfig_long(strSection, strKey);
}

bool SwigWrap::GetConfig_bool(
    const std::string& strSection,
    const std::string& strKey)
{
    return Exec()->GetConfig_bool(strSection, strKey);
}

bool SwigWrap::SetConfig_str(
    const std::string& strSection,
    const std::string& strKey,
    const std::string& strValue)
{
    return Exec()->SetConfig_str(strSection, strKey, strValue);
}

bool SwigWrap::SetConfig_long(
    const std::string& strSection,
    const std::string& strKey,
    const int64_t& lValue)
{
    return Exec()->SetConfig_long(strSection, strKey, lValue);
}

bool SwigWrap::SetConfig_bool(
    const std::string& strSection,
    const std::string& strKey,
    const bool bValue)
{
    return Exec()->SetConfig_bool(strSection, strKey, bValue);
}

void SwigWrap::Output(const int32_t& nLogLevel, const std::string& strOutput)
{
    return Exec()->Output(nLogLevel, strOutput);
}

bool SwigWrap::SetWallet(const std::string& strWalletFilename)
{
    return Exec()->SetWallet(strWalletFilename);
}

bool SwigWrap::WalletExists() { return Exec()->WalletExists(); }

bool SwigWrap::LoadWallet() { return Exec()->LoadWallet(); }

bool SwigWrap::SwitchWallet() { return Exec()->LoadWallet(); }

int32_t SwigWrap::GetMemlogSize() { return Exec()->GetMemlogSize(); }

std::string SwigWrap::GetMemlogAtIndex(const int32_t& nIndex)
{
    return Exec()->GetMemlogAtIndex(nIndex);
}

std::string SwigWrap::PeekMemlogFront() { return Exec()->PeekMemlogFront(); }

std::string SwigWrap::PeekMemlogBack() { return Exec()->PeekMemlogBack(); }

bool SwigWrap::PopMemlogFront() { return Exec()->PopMemlogFront(); }

bool SwigWrap::PopMemlogBack() { return Exec()->PopMemlogBack(); }

std::string SwigWrap::NumList_Add(
    const std::string& strNumList,
    const std::string& strNumbers)
{
    return Exec()->NumList_Add(strNumList, strNumbers);
}

std::string SwigWrap::NumList_Remove(
    const std::string& strNumList,
    const std::string& strNumbers)
{
    return Exec()->NumList_Remove(strNumList, strNumbers);
}

bool SwigWrap::NumList_VerifyQuery(
    const std::string& strNumList,
    const std::string& strNumbers)
{
    return Exec()->NumList_VerifyQuery(strNumList, strNumbers);
}

bool SwigWrap::NumList_VerifyAll(
    const std::string& strNumList,
    const std::string& strNumbers)
{
    return Exec()->NumList_VerifyAll(strNumList, strNumbers);
}

int32_t SwigWrap::NumList_Count(const std::string& strNumList)
{
    return Exec()->NumList_Count(strNumList);
}

bool SwigWrap::IsValidID(const std::string& strPurportedID)
{
    return Exec()->IsValidID(strPurportedID);
}

std::string SwigWrap::CreateNymLegacy(
    const int32_t& nKeySize,
    const std::string& NYM_ID_SOURCE)
{
    return Exec()->CreateNymLegacy(nKeySize, NYM_ID_SOURCE);
}

std::string SwigWrap::CreateIndividualNym(
    const std::string& name,
    const std::string& seed,
    const std::uint32_t index)
{
    return Exec()->CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, name, seed, index);
}

std::string SwigWrap::CreateOrganizationNym(
    const std::string& name,
    const std::string& seed,
    const std::uint32_t index)
{
    return Exec()->CreateNymHD(
        proto::CITEMTYPE_ORGANIZATION, name, seed, index);
}

std::string SwigWrap::CreateBusinessNym(
    const std::string& name,
    const std::string& seed,
    const std::uint32_t index)
{
    return Exec()->CreateNymHD(proto::CITEMTYPE_BUSINESS, name, seed, index);
}

std::string SwigWrap::GetNym_ActiveCronItemIDs(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID)
{
    return Exec()->GetNym_ActiveCronItemIDs(NYM_ID, NOTARY_ID);
}
std::string SwigWrap::GetActiveCronItem(
    const std::string& NOTARY_ID,
    int64_t lTransNum)
{
    return Exec()->GetActiveCronItem(NOTARY_ID, lTransNum);
}

std::string SwigWrap::GetNym_SourceForID(const std::string& NYM_ID)
{
    return Exec()->GetNym_SourceForID(NYM_ID);
}

std::string SwigWrap::GetNym_Description(const std::string& NYM_ID)
{
    return Exec()->GetNym_Description(NYM_ID);
}

int32_t SwigWrap::GetNym_MasterCredentialCount(const std::string& NYM_ID)
{
    return Exec()->GetNym_MasterCredentialCount(NYM_ID);
}

std::string SwigWrap::GetNym_MasterCredentialID(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_MasterCredentialID(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_MasterCredentialContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID)
{
    return Exec()->GetNym_MasterCredentialContents(NYM_ID, CREDENTIAL_ID);
}

int32_t SwigWrap::GetNym_RevokedCredCount(const std::string& NYM_ID)
{
    return Exec()->GetNym_RevokedCredCount(NYM_ID);
}

std::string SwigWrap::GetNym_RevokedCredID(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_RevokedCredID(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_RevokedCredContents(
    const std::string& NYM_ID,
    const std::string& CREDENTIAL_ID)
{
    return Exec()->GetNym_RevokedCredContents(NYM_ID, CREDENTIAL_ID);
}

int32_t SwigWrap::GetNym_ChildCredentialCount(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID)
{
    return Exec()->GetNym_ChildCredentialCount(NYM_ID, MASTER_CRED_ID);
}

std::string SwigWrap::GetNym_ChildCredentialID(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_ChildCredentialID(NYM_ID, MASTER_CRED_ID, nIndex);
}

std::string SwigWrap::GetNym_ChildCredentialContents(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID)
{
    return Exec()->GetNym_ChildCredentialContents(
        NYM_ID, MASTER_CRED_ID, SUB_CRED_ID);
}

std::string SwigWrap::NymIDFromPaymentCode(const std::string& paymentCode)
{
    return Exec()->NymIDFromPaymentCode(paymentCode);
}

bool SwigWrap::RevokeChildCredential(
    const std::string& NYM_ID,
    const std::string& MASTER_CRED_ID,
    const std::string& SUB_CRED_ID)
{
    return Exec()->RevokeChildCredential(NYM_ID, MASTER_CRED_ID, SUB_CRED_ID);
}

std::string SwigWrap::GetSignerNymID(const std::string& str_Contract)
{
    return Exec()->GetSignerNymID(str_Contract);
}

std::string SwigWrap::CalculateContractID(const std::string& str_Contract)
{
    return Exec()->CalculateContractID(str_Contract);
}

std::string SwigWrap::CreateCurrencyContract(
    const std::string& NYM_ID,
    const std::string& shortname,
    const std::string& terms,
    const std::string& name,
    const std::string& symbol,
    const std::string& tla,
    const uint32_t power,
    const std::string& fraction)
{
    return Exec()->CreateCurrencyContract(
        NYM_ID, shortname, terms, name, symbol, tla, power, fraction);
}

std::string SwigWrap::GetServer_Contract(const std::string& NOTARY_ID)
{
    return Exec()->GetServer_Contract(NOTARY_ID);
}

int32_t SwigWrap::GetCurrencyDecimalPower(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->GetCurrencyDecimalPower(INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::GetCurrencyTLA(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->GetCurrencyTLA(INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::GetCurrencySymbol(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->GetCurrencySymbol(INSTRUMENT_DEFINITION_ID);
}

int64_t SwigWrap::StringToAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT)
{
    return Exec()->StringToAmountLocale(
        INSTRUMENT_DEFINITION_ID, str_input, THOUSANDS_SEP, DECIMAL_POINT);
}

std::string SwigWrap::FormatAmountLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT)
{
    return Exec()->FormatAmountLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, THOUSANDS_SEP, DECIMAL_POINT);
}

std::string SwigWrap::FormatAmountWithoutSymbolLocale(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT,
    const std::string& THOUSANDS_SEP,
    const std::string& DECIMAL_POINT)
{
    return Exec()->FormatAmountWithoutSymbolLocale(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT, THOUSANDS_SEP, DECIMAL_POINT);
}

int64_t SwigWrap::StringToAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& str_input)
{
    return Exec()->StringToAmount(INSTRUMENT_DEFINITION_ID, str_input);
}

std::string SwigWrap::FormatAmount(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT)
{
    return Exec()->FormatAmount(INSTRUMENT_DEFINITION_ID, THE_AMOUNT);
}

std::string SwigWrap::FormatAmountWithoutSymbol(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const int64_t& THE_AMOUNT)
{
    return Exec()->FormatAmountWithoutSymbol(
        INSTRUMENT_DEFINITION_ID, THE_AMOUNT);
}

std::string SwigWrap::GetAssetType_Contract(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->GetAssetType_Contract(INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::AddServerContract(const std::string& strContract)
{
    return Exec()->AddServerContract(strContract);
}

std::string SwigWrap::AddUnitDefinition(const std::string& strContract)
{
    return Exec()->AddUnitDefinition(strContract);
}

int32_t SwigWrap::GetNymCount(void) { return Exec()->GetNymCount(); }

int32_t SwigWrap::GetServerCount(void) { return Exec()->GetServerCount(); }

int32_t SwigWrap::GetAssetTypeCount(void)
{
    return Exec()->GetAssetTypeCount();
}

int32_t SwigWrap::GetAccountCount(void) { return Exec()->GetAccountCount(); }

bool SwigWrap::Wallet_CanRemoveServer(const std::string& NOTARY_ID)
{
    return Exec()->Wallet_CanRemoveServer(NOTARY_ID);
}

bool SwigWrap::Wallet_RemoveServer(const std::string& NOTARY_ID)
{
    return Exec()->Wallet_RemoveServer(NOTARY_ID);
}

bool SwigWrap::Wallet_CanRemoveAssetType(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->Wallet_CanRemoveAssetType(INSTRUMENT_DEFINITION_ID);
}

bool SwigWrap::Wallet_RemoveAssetType(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->Wallet_RemoveAssetType(INSTRUMENT_DEFINITION_ID);
}

bool SwigWrap::Wallet_CanRemoveNym(const std::string& NYM_ID)
{
    return Exec()->Wallet_CanRemoveNym(NYM_ID);
}

bool SwigWrap::Wallet_RemoveNym(const std::string& NYM_ID)
{
    return Exec()->Wallet_RemoveNym(NYM_ID);
}

bool SwigWrap::Wallet_CanRemoveAccount(const std::string& ACCOUNT_ID)
{
    return Exec()->Wallet_CanRemoveAccount(ACCOUNT_ID);
}

bool SwigWrap::DoesBoxReceiptExist(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const int32_t& nBoxType,
    const int64_t& TRANSACTION_NUMBER)
{
    return Exec()->DoesBoxReceiptExist(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, nBoxType, TRANSACTION_NUMBER);
}

int32_t SwigWrap::getBoxReceipt(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const int32_t& nBoxType,
    const int64_t& TRANSACTION_NUMBER)
{
    return Exec()->getBoxReceipt(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, nBoxType, TRANSACTION_NUMBER);
}

int32_t SwigWrap::deleteAssetAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->deleteAssetAccount(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::Wallet_ExportNym(const std::string& NYM_ID)
{
    return Exec()->Wallet_ExportNym(NYM_ID);
}

std::string SwigWrap::Wallet_ImportNym(const std::string& FILE_CONTENTS)
{
    return Exec()->Wallet_ImportNym(FILE_CONTENTS);
}

bool SwigWrap::Wallet_ChangePassphrase()
{
    return Exec()->Wallet_ChangePassphrase();
}

bool SwigWrap::Wallet_CheckPassword()
{
    auto key = OT::App().Crypto().mutable_DefaultKey();

    if (false == key.It().IsGenerated()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No master key." << std::endl;

        return false;
    }

    const std::string message{};
    OTPassword null;
    key.It().Reset();

    return key.It().GetMasterPassword(key.It(), null, message.c_str(), false);
}

std::string SwigWrap::Wallet_GetNymIDFromPartial(const std::string& PARTIAL_ID)
{
    return Exec()->Wallet_GetNymIDFromPartial(PARTIAL_ID);
}

std::string SwigWrap::Wallet_GetNotaryIDFromPartial(
    const std::string& PARTIAL_ID)
{
    return Exec()->Wallet_GetNotaryIDFromPartial(PARTIAL_ID);
}

std::string SwigWrap::Wallet_GetInstrumentDefinitionIDFromPartial(
    const std::string& PARTIAL_ID)
{
    return Exec()->Wallet_GetInstrumentDefinitionIDFromPartial(PARTIAL_ID);
}

std::string SwigWrap::Wallet_GetAccountIDFromPartial(
    const std::string& PARTIAL_ID)
{
    return Exec()->Wallet_GetAccountIDFromPartial(PARTIAL_ID);
}

std::string SwigWrap::GetNym_ID(const int32_t& nIndex)
{
    return Exec()->GetNym_ID(nIndex);
}

std::string SwigWrap::GetNym_Name(const std::string& NYM_ID)
{
    return Exec()->GetNym_Name(NYM_ID);
}

bool SwigWrap::IsNym_RegisteredAtServer(
    const std::string& NYM_ID,
    const std::string& NOTARY_ID)
{
    return Exec()->IsNym_RegisteredAtServer(NYM_ID, NOTARY_ID);
}

std::string SwigWrap::GetNym_Stats(const std::string& NYM_ID)
{
    return Exec()->GetNym_Stats(NYM_ID);
}

std::string SwigWrap::GetNym_NymboxHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetNym_NymboxHash(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::GetNym_RecentHash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetNym_RecentHash(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::GetNym_InboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetNym_InboxHash(ACCOUNT_ID, NYM_ID);
}

std::string SwigWrap::GetNym_OutboxHash(
    const std::string& ACCOUNT_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetNym_OutboxHash(ACCOUNT_ID, NYM_ID);
}

std::string SwigWrap::GetNym_MailCount(const std::string& NYM_ID)
{
    return comma(Exec()->GetNym_MailCount(NYM_ID));
}

std::string SwigWrap::GetNym_MailContentsByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_MailContentsByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_MailSenderIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_MailSenderIDByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_MailNotaryIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_MailNotaryIDByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_RemoveMailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->Nym_RemoveMailByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_VerifyMailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->Nym_VerifyMailByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_OutmailCount(const std::string& NYM_ID)
{
    return comma(Exec()->GetNym_OutmailCount(NYM_ID));
}

std::string SwigWrap::GetNym_OutmailContentsByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_OutmailContentsByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_OutmailRecipientIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_OutmailRecipientIDByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_OutmailNotaryIDByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->GetNym_OutmailNotaryIDByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_RemoveOutmailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->Nym_RemoveOutmailByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_VerifyOutmailByIndex(
    const std::string& NYM_ID,
    const std::string& nIndex)
{
    return Exec()->Nym_VerifyOutmailByIndex(NYM_ID, nIndex);
}

int32_t SwigWrap::GetNym_OutpaymentsCount(const std::string& NYM_ID)
{
    return Exec()->GetNym_OutpaymentsCount(NYM_ID);
}

std::string SwigWrap::GetNym_OutpaymentsContentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_OutpaymentsContentsByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_OutpaymentsRecipientIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_OutpaymentsRecipientIDByIndex(NYM_ID, nIndex);
}

std::string SwigWrap::GetNym_OutpaymentsNotaryIDByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->GetNym_OutpaymentsNotaryIDByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_RemoveOutpaymentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->Nym_RemoveOutpaymentsByIndex(NYM_ID, nIndex);
}

bool SwigWrap::Nym_VerifyOutpaymentsByIndex(
    const std::string& NYM_ID,
    const int32_t& nIndex)
{
    return Exec()->Nym_VerifyOutpaymentsByIndex(NYM_ID, nIndex);
}

int64_t SwigWrap::Instrmnt_GetAmount(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetAmount(THE_INSTRUMENT);
}

int64_t SwigWrap::Instrmnt_GetTransNum(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetTransNum(THE_INSTRUMENT);
}

time64_t SwigWrap::Instrmnt_GetValidFrom(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetValidFrom(THE_INSTRUMENT);
}

time64_t SwigWrap::Instrmnt_GetValidTo(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetValidTo(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetType(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetType(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetMemo(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetMemo(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetNotaryID(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetNotaryID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetInstrumentDefinitionID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetInstrumentDefinitionID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetRemitterNymID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetRemitterNymID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetRemitterAcctID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetRemitterAcctID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetSenderNymID(const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetSenderNymID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetSenderAcctID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetSenderAcctID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetRecipientNymID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetRecipientNymID(THE_INSTRUMENT);
}

std::string SwigWrap::Instrmnt_GetRecipientAcctID(
    const std::string& THE_INSTRUMENT)
{
    return Exec()->Instrmnt_GetRecipientAcctID(THE_INSTRUMENT);
}

bool SwigWrap::SetNym_Alias(
    const std::string& targetNymID,
    const std::string& walletNymID,
    const std::string& name)
{
    return Exec()->SetNym_Alias(targetNymID, walletNymID, name);
}

bool SwigWrap::Rename_Nym(
    const std::string& nymID,
    const std::string& name,
    const std::uint32_t type,
    const bool primary)
{
    return Exec()->Rename_Nym(
        nymID, name, static_cast<proto::ContactItemType>(type), primary);
}

bool SwigWrap::SetServer_Name(
    const std::string& NOTARY_ID,
    const std::string& STR_NEW_NAME)
{
    return Exec()->SetServer_Name(NOTARY_ID, STR_NEW_NAME);
}

bool SwigWrap::SetAssetType_Name(
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& STR_NEW_NAME)
{
    return Exec()->SetAssetType_Name(INSTRUMENT_DEFINITION_ID, STR_NEW_NAME);
}

int32_t SwigWrap::GetNym_TransactionNumCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetNym_TransactionNumCount(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::GetServer_ID(const int32_t& nIndex)
{
    return Exec()->GetServer_ID(nIndex);
}

std::string SwigWrap::GetServer_Name(const std::string& THE_ID)
{
    return Exec()->GetServer_Name(THE_ID);
}

std::string SwigWrap::GetAssetType_ID(const int32_t& nIndex)
{
    return Exec()->GetAssetType_ID(nIndex);
}

std::string SwigWrap::GetAssetType_Name(const std::string& THE_ID)
{
    return Exec()->GetAssetType_Name(THE_ID);
}

std::string SwigWrap::GetAssetType_TLA(const std::string& THE_ID)
{
    return Exec()->GetAssetType_TLA(THE_ID);
}

std::string SwigWrap::GetAccountWallet_ID(const int32_t& nIndex)
{
    return Exec()->GetAccountWallet_ID(nIndex);
}

std::string SwigWrap::GetAccountWallet_Name(const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_Name(THE_ID);
}

std::string SwigWrap::GetAccountWallet_InboxHash(const std::string& ACCOUNT_ID)
{
    return Exec()->GetAccountWallet_InboxHash(ACCOUNT_ID);
}

std::string SwigWrap::GetAccountWallet_OutboxHash(const std::string& ACCOUNT_ID)
{
    return Exec()->GetAccountWallet_OutboxHash(ACCOUNT_ID);
}

time64_t SwigWrap::GetTime(void) { return Exec()->GetTime(); }

std::string SwigWrap::Encode(
    const std::string& strPlaintext,
    const bool& bLineBreaks)
{
    return Exec()->Encode(strPlaintext, bLineBreaks);
}

std::string SwigWrap::Decode(
    const std::string& strEncoded,
    const bool& bLineBreaks)
{
    return Exec()->Decode(strEncoded, bLineBreaks);
}

std::string SwigWrap::Encrypt(
    const std::string& RECIPIENT_NYM_ID,
    const std::string& strPlaintext)
{
    return Exec()->Encrypt(RECIPIENT_NYM_ID, strPlaintext);
}

std::string SwigWrap::Decrypt(
    const std::string& RECIPIENT_NYM_ID,
    const std::string& strCiphertext)
{
    return Exec()->Decrypt(RECIPIENT_NYM_ID, strCiphertext);
}

std::string SwigWrap::CreateSymmetricKey()
{
    return Exec()->CreateSymmetricKey();
}

std::string SwigWrap::SymmetricEncrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& PLAINTEXT)
{
    return Exec()->SymmetricEncrypt(SYMMETRIC_KEY, PLAINTEXT);
}

std::string SwigWrap::SymmetricDecrypt(
    const std::string& SYMMETRIC_KEY,
    const std::string& CIPHERTEXT_ENVELOPE)
{
    return Exec()->SymmetricDecrypt(SYMMETRIC_KEY, CIPHERTEXT_ENVELOPE);
}

std::string SwigWrap::SignContract(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT)
{
    return Exec()->SignContract(SIGNER_NYM_ID, THE_CONTRACT);
}

std::string SwigWrap::FlatSign(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_INPUT,
    const std::string& CONTRACT_TYPE)
{
    return Exec()->FlatSign(SIGNER_NYM_ID, THE_INPUT, CONTRACT_TYPE);
}

std::string SwigWrap::AddSignature(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT)
{
    return Exec()->AddSignature(SIGNER_NYM_ID, THE_CONTRACT);
}

bool SwigWrap::VerifySignature(
    const std::string& SIGNER_NYM_ID,
    const std::string& THE_CONTRACT)
{
    return Exec()->VerifySignature(SIGNER_NYM_ID, THE_CONTRACT);
}

std::string SwigWrap::VerifyAndRetrieveXMLContents(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_ID)
{
    return Exec()->VerifyAndRetrieveXMLContents(THE_CONTRACT, SIGNER_ID);
}

bool SwigWrap::VerifyAccountReceipt(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID)
{
    return Exec()->VerifyAccountReceipt(NOTARY_ID, NYM_ID, ACCT_ID);
}

bool SwigWrap::SetAccountWallet_Name(
    const std::string& ACCT_ID,
    const std::string& SIGNER_NYM_ID,
    const std::string& ACCT_NEW_NAME)
{
    return Exec()->SetAccountWallet_Name(ACCT_ID, SIGNER_NYM_ID, ACCT_NEW_NAME);
}

int64_t SwigWrap::GetAccountWallet_Balance(const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_Balance(THE_ID);
}

std::string SwigWrap::GetAccountWallet_Type(const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_Type(THE_ID);
}

std::string SwigWrap::GetAccountWallet_InstrumentDefinitionID(
    const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_InstrumentDefinitionID(THE_ID);
}

std::string SwigWrap::GetAccountWallet_NotaryID(const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_NotaryID(THE_ID);
}

std::string SwigWrap::GetAccountWallet_NymID(const std::string& THE_ID)
{
    return Exec()->GetAccountWallet_NymID(THE_ID);
}

std::string SwigWrap::WriteCheque(
    const std::string& NOTARY_ID,
    const int64_t& CHEQUE_AMOUNT,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    const std::string& SENDER_ACCT_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& CHEQUE_MEMO,
    const std::string& RECIPIENT_NYM_ID)
{
    return Exec()->WriteCheque(
        NOTARY_ID,
        CHEQUE_AMOUNT,
        VALID_FROM,
        VALID_TO,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        CHEQUE_MEMO,
        RECIPIENT_NYM_ID);
}

bool SwigWrap::DiscardCheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_CHEQUE)
{
    return Exec()->DiscardCheque(NOTARY_ID, NYM_ID, ACCT_ID, THE_CHEQUE);
}

std::string SwigWrap::ProposePaymentPlan(
    const std::string& NOTARY_ID,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    const std::string& SENDER_ACCT_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& PLAN_CONSIDERATION,
    const std::string& RECIPIENT_ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const int64_t& INITIAL_PAYMENT_AMOUNT,
    const time64_t& INITIAL_PAYMENT_DELAY,
    const int64_t& PAYMENT_PLAN_AMOUNT,
    const time64_t& PAYMENT_PLAN_DELAY,
    const time64_t& PAYMENT_PLAN_PERIOD,
    const time64_t& PAYMENT_PLAN_LENGTH,
    const int32_t& PAYMENT_PLAN_MAX_PAYMENTS)
{
    return Exec()->ProposePaymentPlan(
        NOTARY_ID,
        VALID_FROM,
        VALID_TO,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        PLAN_CONSIDERATION,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID,
        INITIAL_PAYMENT_AMOUNT,
        INITIAL_PAYMENT_DELAY,
        PAYMENT_PLAN_AMOUNT,
        PAYMENT_PLAN_DELAY,
        PAYMENT_PLAN_PERIOD,
        PAYMENT_PLAN_LENGTH,
        PAYMENT_PLAN_MAX_PAYMENTS);
}

std::string SwigWrap::EasyProposePlan(
    const std::string& NOTARY_ID,
    const std::string& DATE_RANGE,
    const std::string& SENDER_ACCT_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& PLAN_CONSIDERATION,
    const std::string& RECIPIENT_ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& INITIAL_PAYMENT,
    const std::string& PAYMENT_PLAN,
    const std::string& PLAN_EXPIRY)
{
    return Exec()->EasyProposePlan(
        NOTARY_ID,
        DATE_RANGE,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        PLAN_CONSIDERATION,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID,
        INITIAL_PAYMENT,
        PAYMENT_PLAN,
        PLAN_EXPIRY);
}

std::string SwigWrap::ConfirmPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& SENDER_NYM_ID,
    const std::string& SENDER_ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& PAYMENT_PLAN)
{
    return Exec()->ConfirmPaymentPlan(
        NOTARY_ID,
        SENDER_NYM_ID,
        SENDER_ACCT_ID,
        RECIPIENT_NYM_ID,
        PAYMENT_PLAN);
}

std::string SwigWrap::Create_SmartContract(
    const std::string& SIGNER_NYM_ID,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO,
    bool SPECIFY_ASSETS,
    bool SPECIFY_PARTIES)
{
    return Exec()->Create_SmartContract(
        SIGNER_NYM_ID, VALID_FROM, VALID_TO, SPECIFY_ASSETS, SPECIFY_PARTIES);
}

std::string SwigWrap::SmartContract_SetDates(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const time64_t& VALID_FROM,
    const time64_t& VALID_TO)
{
    return Exec()->SmartContract_SetDates(
        THE_CONTRACT, SIGNER_NYM_ID, VALID_FROM, VALID_TO);
}

bool SwigWrap::Smart_ArePartiesSpecified(const std::string& THE_CONTRACT)
{
    return Exec()->Smart_ArePartiesSpecified(THE_CONTRACT);
}

bool SwigWrap::Smart_AreAssetTypesSpecified(const std::string& THE_CONTRACT)
{
    return Exec()->Smart_AreAssetTypesSpecified(THE_CONTRACT);
}

std::string SwigWrap::SmartContract_AddBylaw(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME)
{
    return Exec()->SmartContract_AddBylaw(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME);
}

std::string SwigWrap::SmartContract_AddClause(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& CLAUSE_NAME,
    const std::string& SOURCE_CODE)
{
    return Exec()->SmartContract_AddClause(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, CLAUSE_NAME, SOURCE_CODE);
}

std::string SwigWrap::SmartContract_AddVariable(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& VAR_NAME,
    const std::string& VAR_ACCESS,
    const std::string& VAR_TYPE,
    const std::string& VAR_VALUE)
{
    return Exec()->SmartContract_AddVariable(
        THE_CONTRACT,
        SIGNER_NYM_ID,
        BYLAW_NAME,
        VAR_NAME,
        VAR_ACCESS,
        VAR_TYPE,
        VAR_VALUE);
}

std::string SwigWrap::SmartContract_AddCallback(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& CALLBACK_NAME,
    const std::string& CLAUSE_NAME)
{
    return Exec()->SmartContract_AddCallback(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, CALLBACK_NAME, CLAUSE_NAME);
}

std::string SwigWrap::SmartContract_AddHook(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME,
    const std::string& CLAUSE_NAME)
{
    return Exec()->SmartContract_AddHook(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, HOOK_NAME, CLAUSE_NAME);
}

std::string SwigWrap::SmartContract_AddParty(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NYM_ID,
    const std::string& PARTY_NAME,
    const std::string& AGENT_NAME)
{
    return Exec()->SmartContract_AddParty(
        THE_CONTRACT, SIGNER_NYM_ID, PARTY_NYM_ID, PARTY_NAME, AGENT_NAME);
}

std::string SwigWrap::SmartContract_AddAccount(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->SmartContract_AddAccount(
        THE_CONTRACT,
        SIGNER_NYM_ID,
        PARTY_NAME,
        ACCT_NAME,
        INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::SmartContract_RemoveBylaw(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME)
{
    return Exec()->SmartContract_RemoveBylaw(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME);
}

std::string SwigWrap::SmartContract_UpdateClause(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& CLAUSE_NAME,
    const std::string& SOURCE_CODE)
{
    return Exec()->SmartContract_UpdateClause(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, CLAUSE_NAME, SOURCE_CODE);
}

std::string SwigWrap::SmartContract_RemoveClause(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& CLAUSE_NAME)
{
    return Exec()->SmartContract_RemoveClause(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, CLAUSE_NAME);
}

std::string SwigWrap::SmartContract_RemoveVariable(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& VAR_NAME)
{
    return Exec()->SmartContract_RemoveVariable(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, VAR_NAME);
}

std::string SwigWrap::SmartContract_RemoveCallback(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& CALLBACK_NAME)
{
    return Exec()->SmartContract_RemoveCallback(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, CALLBACK_NAME);
}

std::string SwigWrap::SmartContract_RemoveHook(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME,
    const std::string& CLAUSE_NAME)
{
    return Exec()->SmartContract_RemoveHook(
        THE_CONTRACT, SIGNER_NYM_ID, BYLAW_NAME, HOOK_NAME, CLAUSE_NAME);
}

std::string SwigWrap::SmartContract_RemoveParty(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NAME)
{
    return Exec()->SmartContract_RemoveParty(
        THE_CONTRACT, SIGNER_NYM_ID, PARTY_NAME);
}

std::string SwigWrap::SmartContract_RemoveAccount(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME)
{
    return Exec()->SmartContract_RemoveAccount(
        THE_CONTRACT, SIGNER_NYM_ID, PARTY_NAME, ACCT_NAME);
}

int32_t SwigWrap::SmartContract_CountNumsNeeded(
    const std::string& THE_CONTRACT,
    const std::string& AGENT_NAME)
{
    return Exec()->SmartContract_CountNumsNeeded(THE_CONTRACT, AGENT_NAME);
}

std::string SwigWrap::SmartContract_ConfirmAccount(
    const std::string& THE_CONTRACT,
    const std::string& SIGNER_NYM_ID,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME,
    const std::string& AGENT_NAME,
    const std::string& ACCT_ID)
{
    return Exec()->SmartContract_ConfirmAccount(
        THE_CONTRACT,
        SIGNER_NYM_ID,
        PARTY_NAME,
        ACCT_NAME,
        AGENT_NAME,
        ACCT_ID);
}

std::string SwigWrap::SmartContract_ConfirmParty(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& NYM_ID,
    const std::string& NOTARY_ID)
{
    return Exec()->SmartContract_ConfirmParty(
        THE_CONTRACT, PARTY_NAME, NYM_ID, NOTARY_ID);
}

bool SwigWrap::Smart_AreAllPartiesConfirmed(const std::string& THE_CONTRACT)
{
    return Exec()->Smart_AreAllPartiesConfirmed(THE_CONTRACT);
}

bool SwigWrap::Smart_IsPartyConfirmed(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME)
{
    return Exec()->Smart_IsPartyConfirmed(THE_CONTRACT, PARTY_NAME);
}

int32_t SwigWrap::Smart_GetPartyCount(const std::string& THE_CONTRACT)
{
    return Exec()->Smart_GetPartyCount(THE_CONTRACT);
}

int32_t SwigWrap::Smart_GetBylawCount(const std::string& THE_CONTRACT)
{
    return Exec()->Smart_GetBylawCount(THE_CONTRACT);
}

std::string SwigWrap::Smart_GetPartyByIndex(
    const std::string& THE_CONTRACT,
    const int32_t& nIndex)
{
    return Exec()->Smart_GetPartyByIndex(THE_CONTRACT, nIndex);
}

std::string SwigWrap::Smart_GetBylawByIndex(
    const std::string& THE_CONTRACT,
    const int32_t& nIndex)
{
    return Exec()->Smart_GetBylawByIndex(THE_CONTRACT, nIndex);
}

std::string SwigWrap::Bylaw_GetLanguage(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME)
{
    return Exec()->Bylaw_GetLanguage(THE_CONTRACT, BYLAW_NAME);
}

int32_t SwigWrap::Bylaw_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME)
{
    return Exec()->Bylaw_GetClauseCount(THE_CONTRACT, BYLAW_NAME);
}

int32_t SwigWrap::Bylaw_GetVariableCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME)
{
    return Exec()->Bylaw_GetVariableCount(THE_CONTRACT, BYLAW_NAME);
}

int32_t SwigWrap::Bylaw_GetHookCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME)
{
    return Exec()->Bylaw_GetHookCount(THE_CONTRACT, BYLAW_NAME);
}

int32_t SwigWrap::Bylaw_GetCallbackCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME)
{
    return Exec()->Bylaw_GetCallbackCount(THE_CONTRACT, BYLAW_NAME);
}

std::string SwigWrap::Clause_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex)
{
    return Exec()->Clause_GetNameByIndex(THE_CONTRACT, BYLAW_NAME, nIndex);
}

std::string SwigWrap::Clause_GetContents(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& CLAUSE_NAME)
{
    return Exec()->Clause_GetContents(THE_CONTRACT, BYLAW_NAME, CLAUSE_NAME);
}

std::string SwigWrap::Variable_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex)
{
    return Exec()->Variable_GetNameByIndex(THE_CONTRACT, BYLAW_NAME, nIndex);
}

std::string SwigWrap::Variable_GetType(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME)
{
    return Exec()->Variable_GetType(THE_CONTRACT, BYLAW_NAME, VARIABLE_NAME);
}

std::string SwigWrap::Variable_GetAccess(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME)
{
    return Exec()->Variable_GetAccess(THE_CONTRACT, BYLAW_NAME, VARIABLE_NAME);
}

std::string SwigWrap::Variable_GetContents(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& VARIABLE_NAME)
{
    return Exec()->Variable_GetContents(
        THE_CONTRACT, BYLAW_NAME, VARIABLE_NAME);
}

std::string SwigWrap::Hook_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex)
{
    return Exec()->Hook_GetNameByIndex(THE_CONTRACT, BYLAW_NAME, nIndex);
}

int32_t SwigWrap::Hook_GetClauseCount(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME)
{
    return Exec()->Hook_GetClauseCount(THE_CONTRACT, BYLAW_NAME, HOOK_NAME);
}

std::string SwigWrap::Hook_GetClauseAtIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& HOOK_NAME,
    const int32_t& nIndex)
{
    return Exec()->Hook_GetClauseAtIndex(
        THE_CONTRACT, BYLAW_NAME, HOOK_NAME, nIndex);
}

std::string SwigWrap::Callback_GetNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const int32_t& nIndex)
{
    return Exec()->Callback_GetNameByIndex(THE_CONTRACT, BYLAW_NAME, nIndex);
}

std::string SwigWrap::Callback_GetClause(
    const std::string& THE_CONTRACT,
    const std::string& BYLAW_NAME,
    const std::string& CALLBACK_NAME)
{
    return Exec()->Callback_GetClause(THE_CONTRACT, BYLAW_NAME, CALLBACK_NAME);
}

int32_t SwigWrap::Party_GetAcctCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME)
{
    return Exec()->Party_GetAcctCount(THE_CONTRACT, PARTY_NAME);
}

int32_t SwigWrap::Party_GetAgentCount(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME)
{
    return Exec()->Party_GetAgentCount(THE_CONTRACT, PARTY_NAME);
}

std::string SwigWrap::Party_GetID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME)
{
    return Exec()->Party_GetID(THE_CONTRACT, PARTY_NAME);
}

std::string SwigWrap::Party_GetAcctNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const int32_t& nIndex)
{
    return Exec()->Party_GetAcctNameByIndex(THE_CONTRACT, PARTY_NAME, nIndex);
}

std::string SwigWrap::Party_GetAcctID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME)
{
    return Exec()->Party_GetAcctID(THE_CONTRACT, PARTY_NAME, ACCT_NAME);
}

std::string SwigWrap::Party_GetAcctInstrumentDefinitionID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME)
{
    return Exec()->Party_GetAcctInstrumentDefinitionID(
        THE_CONTRACT, PARTY_NAME, ACCT_NAME);
}

std::string SwigWrap::Party_GetAcctAgentName(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& ACCT_NAME)
{
    return Exec()->Party_GetAcctAgentName(THE_CONTRACT, PARTY_NAME, ACCT_NAME);
}

std::string SwigWrap::Party_GetAgentNameByIndex(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const int32_t& nIndex)
{
    return Exec()->Party_GetAgentNameByIndex(THE_CONTRACT, PARTY_NAME, nIndex);
}

std::string SwigWrap::Party_GetAgentID(
    const std::string& THE_CONTRACT,
    const std::string& PARTY_NAME,
    const std::string& AGENT_NAME)
{
    return Exec()->Party_GetAgentID(THE_CONTRACT, PARTY_NAME, AGENT_NAME);
}

int32_t SwigWrap::activateSmartContract(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_SMART_CONTRACT)
{
    return Exec()->activateSmartContract(NOTARY_ID, NYM_ID, THE_SMART_CONTRACT);
}

int32_t SwigWrap::triggerClause(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& TRANSACTION_NUMBER,
    const std::string& CLAUSE_NAME,
    const std::string& STR_PARAM)
{
    return Exec()->triggerClause(
        NOTARY_ID, NYM_ID, TRANSACTION_NUMBER, CLAUSE_NAME, STR_PARAM);
}

bool SwigWrap::Msg_HarvestTransactionNumbers(
    const std::string& THE_MESSAGE,
    const std::string& NYM_ID,
    const bool& bHarvestingForRetry,
    const bool& bReplyWasSuccess,
    const bool& bReplyWasFailure,
    const bool& bTransactionWasSuccess,
    const bool& bTransactionWasFailure)
{
    return Exec()->Msg_HarvestTransactionNumbers(
        THE_MESSAGE,
        NYM_ID,
        bHarvestingForRetry,
        bReplyWasSuccess,
        bReplyWasFailure,
        bTransactionWasSuccess,
        bTransactionWasFailure);
}

std::string SwigWrap::LoadPubkey_Encryption(const std::string& NYM_ID)
{
    return Exec()->LoadPubkey_Encryption(NYM_ID);
}

std::string SwigWrap::LoadPubkey_Signing(const std::string& NYM_ID)
{
    return Exec()->LoadPubkey_Signing(NYM_ID);
}

std::string SwigWrap::LoadUserPubkey_Encryption(const std::string& NYM_ID)
{
    return Exec()->LoadUserPubkey_Encryption(NYM_ID);
}

std::string SwigWrap::LoadUserPubkey_Signing(const std::string& NYM_ID)
{
    return Exec()->LoadUserPubkey_Signing(NYM_ID);
}

bool SwigWrap::VerifyUserPrivateKey(const std::string& NYM_ID)
{
    return Exec()->VerifyUserPrivateKey(NYM_ID);
}

bool SwigWrap::Mint_IsStillGood(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->Mint_IsStillGood(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::LoadMint(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->LoadMint(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::LoadServerContract(const std::string& NOTARY_ID)
{
    return Exec()->LoadServerContract(NOTARY_ID);
}

std::string SwigWrap::LoadAssetAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadAssetAccount(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::Nymbox_GetReplyNotice(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& REQUEST_NUMBER)
{
    return Exec()->Nymbox_GetReplyNotice(NOTARY_ID, NYM_ID, REQUEST_NUMBER);
}

bool SwigWrap::HaveAlreadySeenReply(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int64_t& REQUEST_NUMBER)
{
    return Exec()->HaveAlreadySeenReply(NOTARY_ID, NYM_ID, REQUEST_NUMBER);
}

std::string SwigWrap::LoadNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadNymbox(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::LoadNymboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadNymboxNoVerify(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::LoadInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadInbox(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadInboxNoVerify(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadOutbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadOutbox(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadOutboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadOutboxNoVerify(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadPaymentInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadPaymentInbox(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::LoadPaymentInboxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadPaymentInboxNoVerify(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::LoadRecordBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadRecordBox(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadRecordBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->LoadRecordBoxNoVerify(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::LoadExpiredBox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadExpiredBox(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::LoadExpiredBoxNoVerify(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadExpiredBoxNoVerify(NOTARY_ID, NYM_ID);
}

bool SwigWrap::RecordPayment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const bool& bIsInbox,
    const int32_t& nIndex,
    const bool& bSaveCopy)
{
    return Exec()->RecordPayment(
        NOTARY_ID, NYM_ID, bIsInbox, nIndex, bSaveCopy);
}

bool SwigWrap::ClearRecord(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const int32_t& nIndex,
    const bool& bClearAll)
{
    return Exec()->ClearRecord(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, nIndex, bClearAll);
}

bool SwigWrap::ClearExpired(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const int32_t& nIndex,
    const bool& bClearAll)
{
    return Exec()->ClearExpired(NOTARY_ID, NYM_ID, nIndex, bClearAll);
}

int32_t SwigWrap::Ledger_GetCount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER)
{
    return Exec()->Ledger_GetCount(NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER);
}

std::string SwigWrap::Ledger_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID)
{
    return Exec()->Ledger_CreateResponse(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

std::string SwigWrap::Ledger_GetTransactionByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex)
{
    return Exec()->Ledger_GetTransactionByIndex(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, nIndex);
}

std::string SwigWrap::Ledger_GetTransactionByID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int64_t& TRANSACTION_NUMBER)
{
    return Exec()->Ledger_GetTransactionByID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, TRANSACTION_NUMBER);
}

std::string SwigWrap::Ledger_GetInstrument(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex)
{
    return Exec()->Ledger_GetInstrument(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, nIndex);
}

std::string SwigWrap::Ledger_GetInstrumentByReceiptID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int64_t& lReceiptId)
{
    return Exec()->Ledger_GetInstrumentByReceiptID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, lReceiptId);
}

int64_t SwigWrap::Ledger_GetTransactionIDByIndex(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const int32_t& nIndex)
{
    return Exec()->Ledger_GetTransactionIDByIndex(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, nIndex);
}

std::string SwigWrap::Ledger_GetTransactionNums(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER)
{
    return Exec()->Ledger_GetTransactionNums(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER);
}

std::string SwigWrap::Ledger_AddTransaction(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Ledger_AddTransaction(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_CreateResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER,
    const std::string& THE_TRANSACTION,
    const bool& BOOL_DO_I_ACCEPT)
{
    return Exec()->Transaction_CreateResponse(
        NOTARY_ID,
        NYM_ID,
        ACCOUNT_ID,
        THE_LEDGER,
        THE_TRANSACTION,
        BOOL_DO_I_ACCEPT);
}

std::string SwigWrap::Ledger_FinalizeResponse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_LEDGER)
{
    return Exec()->Ledger_FinalizeResponse(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_LEDGER);
}

std::string SwigWrap::Transaction_GetVoucher(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetVoucher(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_GetSenderNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetSenderNymID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_GetRecipientNymID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetRecipientNymID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_GetSenderAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetSenderAcctID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_GetRecipientAcctID(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetRecipientAcctID(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Pending_GetNote(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Pending_GetNote(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

int64_t SwigWrap::Transaction_GetAmount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetAmount(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

int64_t SwigWrap::Transaction_GetDisplayReferenceToNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetDisplayReferenceToNum(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

std::string SwigWrap::Transaction_GetType(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetType(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

int64_t SwigWrap::ReplyNotice_GetRequestNum(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->ReplyNotice_GetRequestNum(
        NOTARY_ID, NYM_ID, THE_TRANSACTION);
}

time64_t SwigWrap::Transaction_GetDateSigned(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetDateSigned(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

OT_BOOL SwigWrap::Transaction_GetSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

OT_BOOL SwigWrap::Transaction_IsCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_IsCanceled(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

OT_BOOL SwigWrap::Transaction_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_TRANSACTION)
{
    return Exec()->Transaction_GetBalanceAgreementSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_TRANSACTION);
}

OT_BOOL SwigWrap::Message_GetBalanceAgreementSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetBalanceAgreementSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_MESSAGE);
}

bool SwigWrap::SavePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE)
{
    return Exec()->SavePurse(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID, THE_PURSE);
}

std::string SwigWrap::LoadPurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID)
{
    return Exec()->LoadPurse(NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID);
}

int64_t SwigWrap::Purse_GetTotalValue(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_GetTotalValue(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_PURSE);
}

int32_t SwigWrap::Purse_Count(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_Count(NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_PURSE);
}

bool SwigWrap::Purse_HasPassword(
    const std::string& NOTARY_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_HasPassword(NOTARY_ID, THE_PURSE);
}

std::string SwigWrap::CreatePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_ID,
    const std::string& SIGNER_ID)
{
    return Exec()->CreatePurse(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, OWNER_ID, SIGNER_ID);
}

std::string SwigWrap::CreatePurse_Passphrase(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID)
{
    return Exec()->CreatePurse_Passphrase(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, SIGNER_ID);
}

std::string SwigWrap::Purse_Peek(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_Peek(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, OWNER_ID, THE_PURSE);
}

std::string SwigWrap::Purse_Pop(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& OWNER_OR_SIGNER_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_Pop(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, OWNER_OR_SIGNER_ID, THE_PURSE);
}

std::string SwigWrap::Purse_Empty(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Purse_Empty(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, SIGNER_ID, THE_PURSE);
}

std::string SwigWrap::Purse_Push(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& SIGNER_ID,
    const std::string& OWNER_ID,
    const std::string& THE_PURSE,
    const std::string& THE_TOKEN)
{
    return Exec()->Purse_Push(
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        SIGNER_ID,
        OWNER_ID,
        THE_PURSE,
        THE_TOKEN);
}

bool SwigWrap::Wallet_ImportPurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE)
{
    return Exec()->Wallet_ImportPurse(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID, THE_PURSE);
}

int32_t SwigWrap::exchangePurse(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& NYM_ID,
    const std::string& THE_PURSE)
{
    return Exec()->exchangePurse(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, NYM_ID, THE_PURSE);
}

std::string SwigWrap::Token_ChangeOwner(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN,
    const std::string& SIGNER_NYM_ID,
    const std::string& OLD_OWNER,
    const std::string& NEW_OWNER)
{
    return Exec()->Token_ChangeOwner(
        NOTARY_ID,
        INSTRUMENT_DEFINITION_ID,
        THE_TOKEN,
        SIGNER_NYM_ID,
        OLD_OWNER,
        NEW_OWNER);
}

std::string SwigWrap::Token_GetID(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetID(NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_TOKEN);
}

int64_t SwigWrap::Token_GetDenomination(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetDenomination(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_TOKEN);
}

int32_t SwigWrap::Token_GetSeries(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetSeries(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_TOKEN);
}

time64_t SwigWrap::Token_GetValidFrom(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetValidFrom(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_TOKEN);
}

time64_t SwigWrap::Token_GetValidTo(
    const std::string& NOTARY_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetValidTo(
        NOTARY_ID, INSTRUMENT_DEFINITION_ID, THE_TOKEN);
}

std::string SwigWrap::Token_GetInstrumentDefinitionID(
    const std::string& THE_TOKEN)
{
    return Exec()->Token_GetInstrumentDefinitionID(THE_TOKEN);
}

std::string SwigWrap::Token_GetNotaryID(const std::string& THE_TOKEN)
{
    return Exec()->Token_GetNotaryID(THE_TOKEN);
}

bool SwigWrap::IsBasketCurrency(const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->IsBasketCurrency(INSTRUMENT_DEFINITION_ID);
}

int32_t SwigWrap::Basket_GetMemberCount(
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->Basket_GetMemberCount(INSTRUMENT_DEFINITION_ID);
}

std::string SwigWrap::Basket_GetMemberType(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const int32_t& nIndex)
{
    return Exec()->Basket_GetMemberType(
        BASKET_INSTRUMENT_DEFINITION_ID, nIndex);
}

int64_t SwigWrap::Basket_GetMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID)
{
    return Exec()->Basket_GetMinimumTransferAmount(
        BASKET_INSTRUMENT_DEFINITION_ID);
}

int64_t SwigWrap::Basket_GetMemberMinimumTransferAmount(
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const int32_t& nIndex)
{
    return Exec()->Basket_GetMemberMinimumTransferAmount(
        BASKET_INSTRUMENT_DEFINITION_ID, nIndex);
}

int32_t SwigWrap::registerContractNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT)
{
    return Exec()->registerContractNym(NOTARY_ID, NYM_ID, CONTRACT);
}

int32_t SwigWrap::registerContractServer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT)
{
    return Exec()->registerContractServer(NOTARY_ID, NYM_ID, CONTRACT);
}

int32_t SwigWrap::registerContractUnit(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT)
{
    return Exec()->registerContractUnit(NOTARY_ID, NYM_ID, CONTRACT);
}

int32_t SwigWrap::registerNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->registerNym(NOTARY_ID, NYM_ID);
}

int32_t SwigWrap::unregisterNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->unregisterNym(NOTARY_ID, NYM_ID);
}

int64_t SwigWrap::Message_GetUsageCredits(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetUsageCredits(THE_MESSAGE);
}

int32_t SwigWrap::usageCredits(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_CHECK,
    const int64_t& ADJUSTMENT)
{
    return Exec()->usageCredits(NOTARY_ID, NYM_ID, NYM_ID_CHECK, ADJUSTMENT);
}

int32_t SwigWrap::checkNym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_CHECK)
{
    return Exec()->checkNym(NOTARY_ID, NYM_ID, NYM_ID_CHECK);
}

int32_t SwigWrap::sendNymMessage(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_RECIPIENT,
    const std::string& THE_MESSAGE)
{
    return Exec()->sendNymMessage(
        NOTARY_ID, NYM_ID, NYM_ID_RECIPIENT, THE_MESSAGE);
}

std::string SwigWrap::notifyBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& recipientNymID,
    const std::string& unitID,
    const std::string& txid)
{
    return Exec()->notifyBailment(
        serverID, senderNymID, recipientNymID, unitID, txid);
}

std::string SwigWrap::initiateBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& unitID)
{
    return Exec()->initiateBailment(serverID, senderNymID, unitID);
}

std::string SwigWrap::initiateOutBailment(
    const std::string& serverID,
    const std::string& senderNymID,
    const std::string& unitID,
    const std::uint64_t& amount,
    const std::string& terms)
{
    return Exec()->initiateOutBailment(
        serverID, senderNymID, unitID, amount, terms);
}

std::string SwigWrap::requestConnection(
    const std::string& senderNymID,
    const std::string& recipientNymID,
    const std::string& serverID,
    const std::uint64_t& type)
{
    return Exec()->requestConnection(
        senderNymID, recipientNymID, serverID, type);
}

std::string SwigWrap::storeSecret(
    const std::string& senderNymID,
    const std::string& recipientNymID,
    const std::string& serverID,
    const std::uint64_t& type,
    const std::string& primary,
    const std::string& secondary)
{
    return Exec()->storeSecret(
        senderNymID, recipientNymID, serverID, type, primary, secondary);
}

std::string SwigWrap::acknowledgeBailment(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& serverID,
    const std::string& terms)
{
    return Exec()->acknowledgeBailment(senderNymID, requestID, serverID, terms);
}

std::string SwigWrap::acknowledgeNotice(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& serverID,
    const bool ack)
{
    return Exec()->acknowledgeNotice(senderNymID, requestID, serverID, ack);
}

std::string SwigWrap::acknowledgeOutBailment(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& serverID,
    const std::string& terms)
{
    return Exec()->acknowledgeOutBailment(
        senderNymID, requestID, serverID, terms);
}

std::string SwigWrap::acknowledge_connection(
    const std::string& senderNymID,
    const std::string& requestID,
    const std::string& serverID,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key)
{
    return Exec()->acknowledgeConnection(
        senderNymID, requestID, serverID, ack, url, login, password, key);
}

int32_t SwigWrap::initiatePeerRequest(
    const std::string& sender,
    const std::string& recipient,
    const std::string& server,
    const std::string& request)
{
    return Exec()->initiatePeerRequest(sender, recipient, server, request);
}

int32_t SwigWrap::initiatePeerReply(
    const std::string& sender,
    const std::string& recipient,
    const std::string& server,
    const std::string& request,
    const std::string& reply)
{
    return Exec()->initiatePeerReply(sender, recipient, server, request, reply);
}

int32_t SwigWrap::completePeerReply(
    const std::string& nymID,
    const std::string& replyID)
{
    return Exec()->completePeerReply(nymID, replyID);
}

int32_t SwigWrap::completePeerRequest(
    const std::string& nymID,
    const std::string& requestID)
{
    return Exec()->completePeerRequest(nymID, requestID);
}

std::string SwigWrap::comma(const std::list<std::string>& list)
{
    std::ostringstream stream;

    for (const auto& item : list) {
        stream << item;
        stream << ",";
    }

    std::string output = stream.str();

    if (0 < output.size()) {
        output.erase(output.size() - 1, 1);
    }

    return output;
}

std::string SwigWrap::comma(const ObjectList& list)
{
    std::ostringstream stream;

    for (const auto& it : list) {
        const auto& item = it.first;
        stream << item;
        stream << ",";
    }

    std::string output = stream.str();

    if (0 < output.size()) {
        output.erase(output.size() - 1, 1);
    }

    return output;
}

std::string SwigWrap::comma(const std::set<Identifier>& list)
{
    std::ostringstream stream;

    for (const auto& item : list) {
        stream << String(item).Get();
        stream << ",";
    }

    std::string output = stream.str();

    if (0 < output.size()) {
        output.erase(output.size() - 1, 1);
    }

    return output;
}

std::string SwigWrap::getSentRequests(const std::string& nymID)
{
    return comma(Exec()->getSentRequests(nymID));
}

std::string SwigWrap::getIncomingRequests(const std::string& nymID)
{
    return comma(Exec()->getIncomingRequests(nymID));
}

std::string SwigWrap::getFinishedRequests(const std::string& nymID)
{
    return comma(Exec()->getFinishedRequests(nymID));
}

std::string SwigWrap::getProcessedRequests(const std::string& nymID)
{
    return comma(Exec()->getProcessedRequests(nymID));
}

std::string SwigWrap::getSentReplies(const std::string& nymID)
{
    return comma(Exec()->getSentReplies(nymID));
}

std::string SwigWrap::getIncomingReplies(const std::string& nymID)
{
    return comma(Exec()->getIncomingReplies(nymID));
}

std::string SwigWrap::getFinishedReplies(const std::string& nymID)
{
    return comma(Exec()->getFinishedReplies(nymID));
}

std::string SwigWrap::getProcessedReplies(const std::string& nymID)
{
    return comma(Exec()->getProcessedReplies(nymID));
}

std::string SwigWrap::getRequest(
    const std::string& nymID,
    const std::string& requestID,
    const std::uint64_t box)
{
    return Exec()->getRequest(nymID, requestID, static_cast<StorageBox>(box));
}

std::string SwigWrap::getReply(
    const std::string& nymID,
    const std::string& replyID,
    const std::uint64_t box)
{
    return Exec()->getReply(nymID, replyID, static_cast<StorageBox>(box));
}

std::string SwigWrap::getRequest_Base64(
    const std::string& nymID,
    const std::string& requestID)
{
    return Exec()->getRequest_Base64(nymID, requestID);
}

std::string SwigWrap::getReply_Base64(
    const std::string& nymID,
    const std::string& replyID)
{
    return Exec()->getReply_Base64(nymID, replyID);
}

int32_t SwigWrap::sendNymInstrument(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& NYM_ID_RECIPIENT,
    const std::string& THE_INSTRUMENT,
    const std::string& INSTRUMENT_FOR_SENDER)
{
    return Exec()->sendNymInstrument(
        NOTARY_ID,
        NYM_ID,
        NYM_ID_RECIPIENT,
        THE_INSTRUMENT,
        INSTRUMENT_FOR_SENDER);
}

int32_t SwigWrap::registerInstrumentDefinition(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_CONTRACT)
{
    return Exec()->registerInstrumentDefinition(
        NOTARY_ID, NYM_ID, THE_CONTRACT);
}

int32_t SwigWrap::getInstrumentDefinition(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->getInstrumentDefinition(
        NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
}

int32_t SwigWrap::getMint(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->getMint(NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
}

int32_t SwigWrap::registerAccount(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID)
{
    return Exec()->registerAccount(NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
}

int32_t SwigWrap::getAccountData(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID)
{
    return Exec()->getAccountData(NOTARY_ID, NYM_ID, ACCT_ID);
}

std::string SwigWrap::GenerateBasketCreation(
    const std::string& nymID,
    const std::string& shortname,
    const std::string& name,
    const std::string& symbol,
    const std::string& terms,
    const uint64_t weight)
{
    return Exec()->GenerateBasketCreation(
        nymID, shortname, name, symbol, terms, weight);
}

std::string SwigWrap::AddBasketCreationItem(
    const std::string& basketTemplate,
    const std::string& currencyID,
    const uint64_t& weight)
{
    return Exec()->AddBasketCreationItem(basketTemplate, currencyID, weight);
}

int32_t SwigWrap::issueBasket(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_BASKET)
{
    return Exec()->issueBasket(NOTARY_ID, NYM_ID, THE_BASKET);
}

std::string SwigWrap::GenerateBasketExchange(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::string& BASKET_ASSET_ACCT_ID,
    const int32_t& TRANSFER_MULTIPLE)
{
    return Exec()->GenerateBasketExchange(
        NOTARY_ID,
        NYM_ID,
        BASKET_INSTRUMENT_DEFINITION_ID,
        BASKET_ASSET_ACCT_ID,
        TRANSFER_MULTIPLE);
}

std::string SwigWrap::AddBasketExchangeItem(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_BASKET,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& ASSET_ACCT_ID)
{
    return Exec()->AddBasketExchangeItem(
        NOTARY_ID, NYM_ID, THE_BASKET, INSTRUMENT_DEFINITION_ID, ASSET_ACCT_ID);
}

int32_t SwigWrap::exchangeBasket(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::string& THE_BASKET,
    const bool& BOOL_EXCHANGE_IN_OR_OUT)
{
    return Exec()->exchangeBasket(
        NOTARY_ID,
        NYM_ID,
        BASKET_INSTRUMENT_DEFINITION_ID,
        THE_BASKET,
        BOOL_EXCHANGE_IN_OR_OUT);
}

int32_t SwigWrap::getTransactionNumbers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->getTransactionNumbers(NOTARY_ID, NYM_ID);
}

int32_t SwigWrap::notarizeWithdrawal(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const int64_t& AMOUNT)
{
    return Exec()->notarizeWithdrawal(NOTARY_ID, NYM_ID, ACCT_ID, AMOUNT);
}

int32_t SwigWrap::notarizeDeposit(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_PURSE)
{
    return Exec()->notarizeDeposit(NOTARY_ID, NYM_ID, ACCT_ID, THE_PURSE);
}

int32_t SwigWrap::notarizeTransfer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_FROM,
    const std::string& ACCT_TO,
    const int64_t& AMOUNT,
    const std::string& NOTE)
{
    return Exec()->notarizeTransfer(
        NOTARY_ID, NYM_ID, ACCT_FROM, ACCT_TO, AMOUNT, NOTE);
}

int32_t SwigWrap::getNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->getNymbox(NOTARY_ID, NYM_ID);
}

int32_t SwigWrap::processInbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& ACCT_LEDGER)
{
    return Exec()->processInbox(NOTARY_ID, NYM_ID, ACCT_ID, ACCT_LEDGER);
}

int32_t SwigWrap::processNymbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->processNymbox(NOTARY_ID, NYM_ID);
}

int32_t SwigWrap::withdrawVoucher(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& CHEQUE_MEMO,
    const int64_t& AMOUNT)
{
    return Exec()->withdrawVoucher(
        NOTARY_ID, NYM_ID, ACCT_ID, RECIPIENT_NYM_ID, CHEQUE_MEMO, AMOUNT);
}

int32_t SwigWrap::payDividend(
    const std::string& NOTARY_ID,
    const std::string& ISSUER_NYM_ID,
    const std::string& DIVIDEND_FROM_ACCT_ID,
    const std::string& SHARES_INSTRUMENT_DEFINITION_ID,
    const std::string& DIVIDEND_MEMO,
    const int64_t& AMOUNT_PER_SHARE)
{
    return Exec()->payDividend(
        NOTARY_ID,
        ISSUER_NYM_ID,
        DIVIDEND_FROM_ACCT_ID,
        SHARES_INSTRUMENT_DEFINITION_ID,
        DIVIDEND_MEMO,
        AMOUNT_PER_SHARE);
}

int32_t SwigWrap::depositCheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& THE_CHEQUE)
{
    return Exec()->depositCheque(NOTARY_ID, NYM_ID, ACCT_ID, THE_CHEQUE);
}

int32_t SwigWrap::depositPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_PAYMENT_PLAN)
{
    return Exec()->depositPaymentPlan(NOTARY_ID, NYM_ID, THE_PAYMENT_PLAN);
}

int32_t SwigWrap::killMarketOffer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ASSET_ACCT_ID,
    const int64_t& TRANSACTION_NUMBER)
{
    return Exec()->killMarketOffer(
        NOTARY_ID, NYM_ID, ASSET_ACCT_ID, TRANSACTION_NUMBER);
}

int32_t SwigWrap::killPaymentPlan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& FROM_ACCT_ID,
    const int64_t& TRANSACTION_NUMBER)
{
    return Exec()->killPaymentPlan(
        NOTARY_ID, NYM_ID, FROM_ACCT_ID, TRANSACTION_NUMBER);
}

int32_t SwigWrap::requestAdmin(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& PASSWORD)
{
    return Exec()->requestAdmin(NOTARY_ID, NYM_ID, PASSWORD);
}

int32_t SwigWrap::serverAddClaim(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& SECTION,
    const std::string& TYPE,
    const std::string& VALUE,
    const bool PRIMARY)
{
    return Exec()->serverAddClaim(
        NOTARY_ID, NYM_ID, SECTION, TYPE, VALUE, PRIMARY);
}

int32_t SwigWrap::issueMarketOffer(
    const std::string& ASSET_ACCT_ID,
    const std::string& CURRENCY_ACCT_ID,
    const int64_t& MARKET_SCALE,
    const int64_t& MINIMUM_INCREMENT,
    const int64_t& TOTAL_ASSETS_ON_OFFER,
    const int64_t& PRICE_LIMIT,
    const bool& bBuyingOrSelling,
    const time64_t& LIFESPAN_IN_SECONDS,
    const std::string& STOP_SIGN,
    const int64_t& ACTIVATION_PRICE)
{
    return Exec()->issueMarketOffer(
        ASSET_ACCT_ID,
        CURRENCY_ACCT_ID,
        MARKET_SCALE,
        MINIMUM_INCREMENT,
        TOTAL_ASSETS_ON_OFFER,
        PRICE_LIMIT,
        bBuyingOrSelling,
        LIFESPAN_IN_SECONDS,
        STOP_SIGN,
        ACTIVATION_PRICE);
}

int32_t SwigWrap::getMarketList(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->getMarketList(NOTARY_ID, NYM_ID);
}

int32_t SwigWrap::getMarketOffers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID,
    const int64_t& MAX_DEPTH)
{
    return Exec()->getMarketOffers(NOTARY_ID, NYM_ID, MARKET_ID, MAX_DEPTH);
}

int32_t SwigWrap::getMarketRecentTrades(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID)
{
    return Exec()->getMarketRecentTrades(NOTARY_ID, NYM_ID, MARKET_ID);
}

int32_t SwigWrap::getNymMarketOffers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->getNymMarketOffers(NOTARY_ID, NYM_ID);
}

std::string SwigWrap::PopMessageBuffer(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->PopMessageBuffer(REQUEST_NUMBER, NOTARY_ID, NYM_ID);
}

void SwigWrap::FlushMessageBuffer(void) { return Exec()->FlushMessageBuffer(); }

std::string SwigWrap::GetSentMessage(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->GetSentMessage(REQUEST_NUMBER, NOTARY_ID, NYM_ID);
}

bool SwigWrap::RemoveSentMessage(
    const int64_t& REQUEST_NUMBER,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID)
{
    return Exec()->RemoveSentMessage(REQUEST_NUMBER, NOTARY_ID, NYM_ID);
}

void SwigWrap::FlushSentMessages(
    const bool& bHarvestingForRetry,
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_NYMBOX)
{
    return Exec()->FlushSentMessages(
        bHarvestingForRetry, NOTARY_ID, NYM_ID, THE_NYMBOX);
}

void SwigWrap::Sleep(const int64_t& MILLISECONDS)
{
    Log::Sleep(std::chrono::milliseconds(MILLISECONDS));
}

bool SwigWrap::ResyncNymWithServer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_MESSAGE)
{
    return Exec()->ResyncNymWithServer(NOTARY_ID, NYM_ID, THE_MESSAGE);
}

int32_t SwigWrap::queryInstrumentDefinitions(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ENCODED_MAP)
{
    return Exec()->queryInstrumentDefinitions(NOTARY_ID, NYM_ID, ENCODED_MAP);
}

std::string SwigWrap::Message_GetPayload(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetPayload(THE_MESSAGE);
}

std::string SwigWrap::Message_GetCommand(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetCommand(THE_MESSAGE);
}

std::string SwigWrap::Message_GetLedger(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetLedger(THE_MESSAGE);
}

std::string SwigWrap::Message_GetNewInstrumentDefinitionID(
    const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetNewInstrumentDefinitionID(THE_MESSAGE);
}

std::string SwigWrap::Message_GetNewIssuerAcctID(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetNewIssuerAcctID(THE_MESSAGE);
}

std::string SwigWrap::Message_GetNewAcctID(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetNewAcctID(THE_MESSAGE);
}

std::string SwigWrap::Message_GetNymboxHash(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetNymboxHash(THE_MESSAGE);
}

OT_BOOL SwigWrap::Message_GetSuccess(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetSuccess(THE_MESSAGE);
}

int32_t SwigWrap::Message_GetDepth(const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetDepth(THE_MESSAGE);
}

OT_BOOL SwigWrap::Message_IsTransactionCanceled(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE)
{
    return Exec()->Message_IsTransactionCanceled(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_MESSAGE);
}

OT_BOOL SwigWrap::Message_GetTransactionSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& THE_MESSAGE)
{
    return Exec()->Message_GetTransactionSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, THE_MESSAGE);
}

std::string SwigWrap::GetContactData(const std::string nymID)
{
    return Exec()->GetContactData(nymID);
}

std::string SwigWrap::GetContactData_Base64(const std::string nymID)
{
    return Exec()->GetContactData_Base64(nymID);
}

std::string SwigWrap::DumpContactData(const std::string nymID)
{
    return Exec()->DumpContactData(nymID);
}

bool SwigWrap::SetContactData(const std::string nymID, const std::string data)
{
    return Exec()->SetContactData(nymID, data);
}

bool SwigWrap::SetContactData_Base64(
    const std::string nymID,
    const std::string data)
{
    return Exec()->SetContactData_Base64(nymID, data);
}

bool SwigWrap::SetClaim(
    const std::string nymID,
    const std::uint32_t section,
    const std::string claim)
{
    return Exec()->SetClaim(nymID, section, claim);
}

bool SwigWrap::SetClaim_Base64(
    const std::string nymID,
    const std::uint32_t section,
    const std::string claim)
{
    return Exec()->SetClaim_Base64(nymID, section, claim);
}

bool SwigWrap::AddClaim(
    const std::string nymID,
    const std::uint32_t section,
    const std::uint32_t type,
    const std::string value,
    const bool active,
    const bool primary)
{
    return Exec()->AddClaim(nymID, section, type, value, active, primary);
}

bool SwigWrap::DeleteClaim(const std::string nymID, const std::string claimID)
{
    return Exec()->DeleteClaim(nymID, claimID);
}

std::string SwigWrap::GetVerificationSet(const std::string nymID)
{
    return Exec()->GetVerificationSet(nymID);
}

std::string SwigWrap::GetVerificationSet_Base64(const std::string nymID)
{
    return Exec()->GetVerificationSet_Base64(nymID);
}

std::string SwigWrap::SetVerification(
    const std::string onNym,
    const std::string claimantNymID,
    const std::string claimID,
    const std::uint8_t polarity,
    const std::int64_t start,
    const std::int64_t end)
{
    bool notUsed = false;

    return Exec()->SetVerification(
        notUsed,
        onNym,
        claimantNymID,
        claimID,
        static_cast<ClaimPolarity>(polarity),
        start,
        end);
}

std::string SwigWrap::SetVerification_Base64(
    const std::string onNym,
    const std::string claimantNymID,
    const std::string claimID,
    const std::uint8_t polarity,
    const std::int64_t start,
    const std::int64_t end)
{
    bool notUsed = false;

    return Exec()->SetVerification_Base64(
        notUsed,
        onNym,
        claimantNymID,
        claimID,
        static_cast<ClaimPolarity>(polarity),
        start,
        end);
}

std::string SwigWrap::GetContactAttributeName(
    const std::uint32_t type,
    std::string lang)
{
    return Exec()->ContactAttributeName(
        static_cast<proto::ContactItemAttribute>(type), lang);
}

std::string SwigWrap::GetContactSections(const std::uint32_t version)
{
    const auto data = Exec()->ContactSectionList(version);
    NumList list;

    for (const auto& it : data) {
        list.Add(it);
    }

    String output;
    list.Output(output);

    return output.Get();
}

std::string SwigWrap::GetContactSectionName(
    const std::uint32_t section,
    std::string lang)
{
    return Exec()->ContactSectionName(
        static_cast<proto::ContactSectionName>(section), lang);
}

std::string SwigWrap::GetContactSectionTypes(
    const std::uint32_t section,
    const std::uint32_t version)
{
    const auto data = Exec()->ContactSectionTypeList(
        static_cast<proto::ContactSectionName>(section), version);
    NumList list;

    for (const auto& it : data) {
        list.Add(it);
    }

    String output;
    list.Output(output);

    return output.Get();
}

std::string SwigWrap::GetContactTypeName(
    const std::uint32_t type,
    std::string lang)
{
    return Exec()->ContactTypeName(
        static_cast<proto::ContactItemType>(type), lang);
}

std::uint32_t SwigWrap::GetReciprocalRelationship(
    const std::uint32_t relationship)
{
    return Exec()->ReciprocalRelationship(
        static_cast<proto::ContactItemType>(relationship));
}

NymData SwigWrap::Wallet_GetNym(const std::string& nymID)
{
    return OT::App().Contract().mutable_Nym(Identifier(nymID));
}

std::string SwigWrap::Wallet_GetSeed() { return Exec()->Wallet_GetSeed(); }

std::string SwigWrap::Wallet_GetPassphrase()
{
    return Exec()->Wallet_GetPassphrase();
}

std::string SwigWrap::Wallet_GetWords() { return Exec()->Wallet_GetWords(); }

std::string SwigWrap::Wallet_ImportSeed(
    const std::string& words,
    const std::string& passphrase)
{
    return Exec()->Wallet_ImportSeed(words, passphrase);
}

void SwigWrap::SetZMQKeepAlive(const std::uint64_t seconds)
{
    Exec()->SetZMQKeepAlive(seconds);
}

bool SwigWrap::CheckConnection(const std::string& server)
{
    return Exec()->CheckConnection(server);
}

bool SwigWrap::ChangeConnectionType(
    const std::string& server,
    const std::uint32_t type)
{
    Identifier serverID(server);

    if (serverID.empty()) {

        return false;
    }

    auto& connection = OT::App().ZMQ().Server(server);

    return connection.ChangeAddressType(static_cast<proto::AddressType>(type));
}

bool SwigWrap::ClearProxy(const std::string& server)
{
    Identifier serverID(server);

    if (serverID.empty()) {

        return false;
    }

    auto& connection = OT::App().ZMQ().Server(server);

    return connection.ClearProxy();
}

std::string SwigWrap::AddChildEd25519Credential(
    const std::string& nymID,
    const std::string& masterID)
{
    return Exec()->AddChildEd25519Credential(
        Identifier(nymID), Identifier(masterID));
}

std::string SwigWrap::AddChildSecp256k1Credential(
    const std::string& nymID,
    const std::string& masterID)
{
    return Exec()->AddChildSecp256k1Credential(
        Identifier(nymID), Identifier(masterID));
}

std::string SwigWrap::AddChildRSACredential(
    const std::string& nymID,
    const std::string& masterID,
    const std::uint32_t keysize)
{
    return Exec()->AddChildRSACredential(
        Identifier(nymID), Identifier(masterID), keysize);
}

//-----------------------------------------------------------------------------

void SwigWrap::Activity_Preload(
    const std::string& nymID,
    const std::uint32_t& items)
{
    OT::App().Activity().PreloadActivity(Identifier(nymID), items);
}

bool SwigWrap::Activity_Mark_Read(
    const std::string& nymID,
    const std::string& threadID,
    const std::string& itemID)
{
    return OT::App().Activity().MarkRead(
        Identifier(nymID), Identifier(threadID), Identifier(itemID));
}

bool SwigWrap::Activity_Mark_Unread(
    const std::string& nymID,
    const std::string& threadID,
    const std::string& itemID)
{
    return OT::App().Activity().MarkUnread(
        Identifier(nymID), Identifier(threadID), Identifier(itemID));
}

std::string SwigWrap::Activity_Thread_base64(
    const std::string& nymId,
    const std::string& threadId)
{
    std::string output{};
    const auto thread =
        OT::App().Activity().Thread(Identifier(nymId), Identifier(threadId));

    if (thread) {

        return OT::App().Crypto().Encode().DataEncode(
            proto::ProtoAsData(*thread));
    }

    return output;
}

std::string SwigWrap::Activity_Threads(
    const std::string& nymID,
    const bool unreadOnly)
{
    return comma(Exec()->GetNym_MailThreads(nymID, unreadOnly));
}

std::uint64_t SwigWrap::Activity_Unread_Count(const std::string& nymID)
{
    return OT::App().Activity().UnreadCount(Identifier(nymID));
}

void SwigWrap::Thread_Preload(
    const std::string& nymID,
    const std::string& threadID,
    const std::uint32_t start,
    const std::uint32_t items)
{
    OT::App().Activity().PreloadThread(
        Identifier(nymID), Identifier(threadID), start, items);
}

//-----------------------------------------------------------------------------

std::string SwigWrap::Blockchain_Account(
    const std::string& nymID,
    const std::string& accountID)
{
    const auto output = OT::App().Blockchain().Account(
        Identifier(nymID), Identifier(accountID));

    if (false == bool(output)) {

        return {};
    }

    return proto::ProtoAsString(*output);
}

std::string SwigWrap::Blockchain_Account_base64(
    const std::string& nymID,
    const std::string& accountID)
{
    const auto account = Blockchain_Account(nymID, accountID);

    if (account.empty()) {

        return {};
    }

    return OT::App().Crypto().Encode().DataEncode(account);
}

std::string SwigWrap::Blockchain_Account_List(
    const std::string& nymID,
    const std::uint32_t chain)
{
    const Identifier nym(nymID);
    const auto type = static_cast<proto::ContactItemType>(chain);
    otInfo << OT_METHOD << __FUNCTION__ << ": Loading account list for "
           << proto::TranslateItemType(type) << std::endl;
    const auto output = OT::App().Blockchain().AccountList(nym, type);

    return comma(output);
}

std::string SwigWrap::Blockchain_Allocate_Address(
    const std::string& nymID,
    const std::string& accountID,
    const std::string& label,
    const bool internal)
{
    const auto output = OT::App().Blockchain().AllocateAddress(
        Identifier(nymID), Identifier(accountID), label, internal);

    if (false == bool(output)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to allocate address"
              << std::endl;

        return {};
    }

    return proto::ProtoAsString(*output);
}

std::string SwigWrap::Blockchain_Allocate_Address_base64(
    const std::string& nymID,
    const std::string& accountID,
    const std::string& label,
    const bool internal)
{
    const auto address =
        Blockchain_Allocate_Address(nymID, accountID, label, internal);

    if (address.empty()) {

        return {};
    }

    return OT::App().Crypto().Encode().DataEncode(address);
}

bool SwigWrap::Blockchain_Assign_Address(
    const std::string& nymID,
    const std::string& accountID,
    const std::uint32_t index,
    const std::string& contact,
    const bool internal)
{
    return OT::App().Blockchain().AssignAddress(
        Identifier(nymID),
        Identifier(accountID),
        index,
        Identifier(contact),
        internal);
}

std::string SwigWrap::Blockchain_Load_Address(
    const std::string& nymID,
    const std::string& accountID,
    const std::uint32_t index,
    const bool internal)
{
    const auto output = OT::App().Blockchain().LoadAddress(
        Identifier(nymID), Identifier(accountID), index, internal);

    if (false == bool(output)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load address"
              << std::endl;

        return {};
    }

    return proto::ProtoAsString(*output);
}

std::string SwigWrap::Blockchain_Load_Address_base64(
    const std::string& nymID,
    const std::string& accountID,
    const std::uint32_t index,
    const bool internal)
{
    const auto address =
        Blockchain_Load_Address(nymID, accountID, index, internal);

    if (address.empty()) {

        return {};
    }

    return OT::App().Crypto().Encode().DataEncode(address);
}

std::string SwigWrap::Blockchain_New_Bip44_Account(
    const std::string& nymID,
    const std::uint32_t chain)
{
    return String(OT::App().Blockchain().NewAccount(
                      Identifier(nymID),
                      BlockchainAccountType::BIP44,
                      static_cast<proto::ContactItemType>(chain)))
        .Get();
}

std::string SwigWrap::Blockchain_New_Bip32_Account(
    const std::string& nymID,
    const std::uint32_t chain)
{
    return String(OT::App().Blockchain().NewAccount(
                      Identifier(nymID),
                      BlockchainAccountType::BIP32,
                      static_cast<proto::ContactItemType>(chain)))
        .Get();
}

bool SwigWrap::Blockchain_Store_Incoming(
    const std::string& nymID,
    const std::string& accountID,
    const std::uint32_t index,
    const bool internal,
    const std::string& transaction)
{
    const auto input =
        proto::TextToProto<proto::BlockchainTransaction>(transaction);
    const auto valid = proto::Validate(input, VERBOSE);

    if (false == valid) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid transaction."
              << std::endl;

        return false;
    }

    return OT::App().Blockchain().StoreIncoming(
        Identifier(nymID), Identifier(accountID), index, internal, input);
}

bool SwigWrap::Blockchain_Store_Incoming_base64(
    const std::string& nymID,
    const std::string& accountID,
    const std::uint32_t index,
    const bool internal,
    const std::string& transaction)
{
    const auto input = OT::App().Crypto().Encode().DataDecode(transaction);

    return Blockchain_Store_Incoming(nymID, accountID, index, internal, input);
}

bool SwigWrap::Blockchain_Store_Outgoing(
    const std::string& nymID,
    const std::string& accountID,
    const std::string& recipientContactID,
    const std::string& transaction)
{
    const auto input =
        proto::TextToProto<proto::BlockchainTransaction>(transaction);
    const auto valid = proto::Validate(input, VERBOSE);

    if (false == valid) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid transaction."
              << std::endl;

        return false;
    }

    return OT::App().Blockchain().StoreOutgoing(
        Identifier(nymID),
        Identifier(accountID),
        Identifier(recipientContactID),
        input);
}

bool SwigWrap::Blockchain_Store_Outgoing_base64(
    const std::string& nymID,
    const std::string& accountID,
    const std::string& recipientContactID,
    const std::string& transaction)
{
    const auto input = OT::App().Crypto().Encode().DataDecode(transaction);

    return Blockchain_Store_Outgoing(
        nymID, accountID, recipientContactID, input);
}

std::string SwigWrap::Blockchain_Transaction(const std::string& txid)
{
    const auto output = OT::App().Blockchain().Transaction(Identifier(txid));

    if (false == bool(output)) {

        return {};
    }

    return proto::ProtoAsString(*output);
}

std::string SwigWrap::Blockchain_Transaction_base64(const std::string& txid)
{
    const auto transaction = Blockchain_Transaction(txid);

    if (transaction.empty()) {

        return {};
    }

    return OT::App().Crypto().Encode().DataEncode(transaction);
}

//-----------------------------------------------------------------------------

std::string SwigWrap::Add_Contact(
    const std::string label,
    const std::string& nymID,
    const std::string& paymentCode)
{
    const bool noLabel = label.empty();
    const bool noNym = nymID.empty();
    const bool noPaymentCode = paymentCode.empty();

    if (noLabel && noNym && noPaymentCode) {

        return {};
    }

    Identifier nym(nymID);
    PaymentCode code(paymentCode);

    if (nym.empty() && code.VerifyInternally()) {
        nym = code.ID();
    }

    auto output = OT::App().Contact().NewContact(label, nym, code);

    if (false == bool(output)) {

        return {};
    }

    return String(output->ID()).Get();
}

std::string SwigWrap::Blockchain_Address_To_Contact(
    const std::string& address,
    const std::uint32_t chain,
    const std::string& label)
{
    const proto::ContactItemType type =
        static_cast<proto::ContactItemType>(chain);
    const auto existing =
        OT::App().Contact().BlockchainAddressToContact(address, type);

    if (false == existing.empty()) {

        return String(existing).Get();
    }

    const auto contact =
        OT::App().Contact().NewContactFromAddress(address, label, type);

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to create new contact."
              << std::endl;

        return {};
    }

    return String(contact->ID()).Get();
}

bool SwigWrap::Contact_Add_Blockchain_Address(
    const std::string& contactID,
    const std::string& address,
    const std::uint32_t chain)
{
    auto contact = OT::App().Contact().mutable_Contact(Identifier(contactID));

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Contact does not exist."
              << std::endl;

        return false;
    }

    return contact->It().AddBlockchainAddress(
        address, static_cast<proto::ContactItemType>(chain));
}

std::string SwigWrap::Contact_List()
{
    return comma(OT::App().Contact().ContactList());
}

bool SwigWrap::Contact_Merge(
    const std::string& parent,
    const std::string& child)
{
    auto contact =
        OT::App().Contact().Merge(Identifier(parent), Identifier(child));

    return bool(contact);
}

std::string SwigWrap::Contact_Name(const std::string& id)
{
    auto contact = OT::App().Contact().Contact(Identifier(id));

    if (contact) {

        return contact->Label();
    }

    return {};
}

std::string SwigWrap::Contact_PaymentCode(
    const std::string& id,
    const std::uint32_t currency)
{
    auto contact = OT::App().Contact().Contact(Identifier(id));

    if (contact) {

        return contact->PaymentCode(
            static_cast<proto::ContactItemType>(currency));
    }

    return {};
}

std::string SwigWrap::Contact_to_Nym(const std::string& contactID)
{
    const auto contact = OT::App().Contact().Contact(Identifier(contactID));

    if (false == bool(contact)) {

        return {};
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {

        return {};
    }

    return String(*nyms.begin()).Get();
}

bool SwigWrap::Have_Contact(const std::string& id)
{
    auto contact = OT::App().Contact().Contact(Identifier(id));

    return bool(contact);
}

bool SwigWrap::Rename_Contact(const std::string& id, const std::string& name)
{
    auto contact = OT::App().Contact().mutable_Contact(Identifier(id));

    if (contact) {
        contact->It().SetLabel(name);

        return true;
    }

    return false;
}

std::string SwigWrap::Nym_to_Contact(const std::string& nymID)
{
    return String(OT::App().Contact().ContactID(Identifier(nymID))).Get();
}

//-----------------------------------------------------------------------------

std::uint8_t SwigWrap::Can_Message(
    const std::string& senderNymID,
    const std::string& recipientContactID)
{
    return static_cast<std::uint8_t>(
        OT::App().API().OTME_TOO().CanMessage(senderNymID, recipientContactID));
}

std::string SwigWrap::Find_Nym(const std::string& nymID)
{
    return String(OT::App().API().OTME_TOO().FindNym(nymID, "")).Get();
}

std::string SwigWrap::Find_Nym_Hint(
    const std::string& nymID,
    const std::string& serverID)
{
    return String(OT::App().API().OTME_TOO().FindNym(nymID, serverID)).Get();
}

std::string SwigWrap::Find_Server(const std::string& serverID)
{
    return String(OT::App().API().OTME_TOO().FindServer(serverID)).Get();
}

std::string SwigWrap::Get_Introduction_Server()
{
    return String(OT::App().API().OTME_TOO().GetIntroductionServer()).Get();
}

std::string SwigWrap::Import_Nym(const std::string& armored)
{
    return OT::App().API().OTME_TOO().ImportNym(armored);
}

std::string SwigWrap::Message_Contact(
    const std::string& senderNymID,
    const std::string& contactID,
    const std::string& message)
{
    const auto output = OT::App().API().OTME_TOO().MessageContact(
        senderNymID, contactID, message);

    return String(output).Get();
}

bool SwigWrap::Node_Request_Connection(
    const std::string& nym,
    const std::string& node,
    const std::int64_t type)
{
    return OT::App().API().OTME_TOO().RequestConnection(nym, node, type);
}

bool SwigWrap::Pair_Complete(const std::string& identifier)
{
    return OT::App().API().OTME_TOO().PairingComplete(identifier);
}

bool SwigWrap::Pair_Node(
    const std::string& myNym,
    const std::string& bridgeNym,
    const std::string& password)
{
    return OT::App().API().OTME_TOO().PairNode(myNym, bridgeNym, password);
}

bool SwigWrap::Pair_ShouldRename(const std::string& identifier)
{
    auto& me_too = OT::App().API().OTME_TOO();

    if (!me_too.PairingComplete(identifier)) {
        if (me_too.PairingSuccessful(identifier)) {
            if (!me_too.NodeRenamed(identifier)) {
                const std::string notaryID = me_too.GetPairedServer(identifier);
                const std::string name = GetServer_Name(notaryID);
                const bool renamed = name != DEFAULT_NODE_NAME;

                return !renamed;
            }
        }
    }

    return false;
}

bool SwigWrap::Pair_Started(const std::string& identifier)
{
    return OT::App().API().OTME_TOO().PairingStarted(identifier);
}

std::string SwigWrap::Pair_Status(const std::string& identifier)
{
    return OT::App().API().OTME_TOO().PairingStatus(identifier);
}

bool SwigWrap::Pair_Success(const std::string& identifier)
{
    return OT::App().API().OTME_TOO().PairingSuccessful(identifier);
}

std::uint64_t SwigWrap::Paired_Node_Count()
{
    return OT::App().API().OTME_TOO().PairedNodeCount();
}

std::string SwigWrap::Paired_Server(const std::string& identifier)
{
    return OT::App().API().OTME_TOO().GetPairedServer(identifier);
}

std::uint64_t SwigWrap::Refresh_Counter()
{
    return OT::App().API().OTME_TOO().RefreshCount();
}

bool SwigWrap::Register_Nym_Public(
    const std::string& nym,
    const std::string& server)
{
    return OT::App().API().OTME_TOO().RegisterNym(nym, server, true);
}

std::string SwigWrap::Register_Nym_Public_async(
    const std::string& nym,
    const std::string& server)
{
    const auto taskID =
        OT::App().API().OTME_TOO().RegisterNym_async(nym, server, true);

    return String(taskID).Get();
}

std::string SwigWrap::Set_Introduction_Server(const std::string& contract)
{
    return OT::App().API().OTME_TOO().SetIntroductionServer(contract);
}

std::uint8_t SwigWrap::Task_Status(const std::string& id)
{
    return static_cast<std::uint8_t>(
        OT::App().API().OTME_TOO().Status(Identifier(id)));
}

void SwigWrap::Trigger_Refresh(const std::string& wallet)
{
    OT::App().API().OTME_TOO().Refresh(wallet);
}

void SwigWrap::Update_Pairing(const std::string& wallet)
{
    OT::App().API().OTME_TOO().UpdatePairing(wallet);
}
}  // namespace opentxs
