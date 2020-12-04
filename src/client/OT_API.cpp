// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "opentxs/client/OT_API.hpp"  // IWYU pragma: associated

#include <cinttypes>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <ratio>
#include <string>
#include <tuple>

#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/client/OTClient.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/basket/Basket.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/BasketItem.pb.h"
#include "opentxs/protobuf/BasketParams.pb.h"
#include "opentxs/protobuf/ContractEnums.pb.h"
#include "opentxs/protobuf/UnitDefinition.pb.h"

#define CLIENT_MASTER_KEY_TIMEOUT_DEFAULT 300

#define OT_METHOD "opentxs::OT_API::"

// The #defines for the latency values can be found in ServerConnection.cpp.
namespace opentxs
{
namespace
{
auto VerifyBalanceReceipt(
    const api::internal::Core& api,
    const otx::context::Server& context,
    const identifier::Server& NOTARY_ID,
    const Identifier& accountID,
    const PasswordPrompt& reason) -> bool
{
    const auto& SERVER_NYM = context.RemoteNym();
    // Load the last successful BALANCE STATEMENT...

    auto tranOut{
        api.Factory().Transaction(SERVER_NYM.ID(), accountID, NOTARY_ID)};

    OT_ASSERT(false != bool(tranOut));

    auto strFilename = String::Factory();
    strFilename->Format("%s.success", accountID.str().c_str());
    const char* szFolder1name = api.Legacy().Receipt();  // receipts
    const auto sNotaryID{NOTARY_ID.str()};
    const char* szFolder2name = sNotaryID.c_str();  // receipts/NOTARY_ID
    const char* szFilename =
        strFilename->Get();  // receipts/NOTARY_ID/accountID.success

    if (!OTDB::Exists(
            api,
            context.LegacyDataFolder(),
            szFolder1name,
            szFolder2name,
            szFilename,
            "")) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Receipt file doesn't exist: ")(
            context.LegacyDataFolder())(PathSeparator())(szFolder1name)(
            PathSeparator())(szFolder2name)(PathSeparator())(szFilename)
            .Flush();
        return false;
    }

    std::string strFileContents(OTDB::QueryPlainString(
        api,
        context.LegacyDataFolder(),
        szFolder1name,
        szFolder2name,
        szFilename,
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (strFileContents.length() < 2) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading file: ")(
            szFolder1name)(PathSeparator())(szFolder2name)(PathSeparator())(
            szFilename)(".")
            .Flush();
        return false;
    }

    auto strTransaction = String::Factory(strFileContents.c_str());

    if (!tranOut->LoadContractFromString(strTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to load balance "
                                           "statement: ")(szFolder1name)(
            PathSeparator())(szFolder2name)(PathSeparator())(szFilename)(".")
            .Flush();
        return false;
    }

    std::unique_ptr<OTTransaction> transaction = nullptr;

    if (tranOut->IsAbbreviated())  // should never happen
    {
        std::int64_t lBoxType = 0;

        if (tranOut->Contains("nymboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::nymbox);
        else if (tranOut->Contains("inboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::inbox);
        else if (tranOut->Contains("outboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::outbox);
        else if (tranOut->Contains("paymentInboxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::paymentInbox);
        else if (tranOut->Contains("recordBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::recordBox);
        else if (tranOut->Contains("expiredBoxRecord"))
            lBoxType = static_cast<std::int64_t>(ledgerType::expiredBox);
        else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading from "
                "abbreviated transaction: "
                "unknown ledger type. (Probably message but who knows).")
                .Flush();
            return false;
        }

        transaction = LoadBoxReceipt(api, *tranOut, lBoxType);
        if (false == bool(transaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading from "
                                               "abbreviated transaction: "
                                               "failed loading box receipt.")
                .Flush();
            return false;
        }
    } else
        transaction.reset(tranOut.release());

    if (!transaction->VerifySignature(SERVER_NYM)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to verify "
            "SERVER_NYM signature on balance statement: ")(szFolder1name)(
            PathSeparator())(szFolder2name)(PathSeparator())(szFilename)(".")
            .Flush();
        return false;
    }

    // At this point, transaction is successfully loaded and verified,
    // containing the last balance receipt.

    return transaction->VerifyBalanceReceipt(context, reason);
}
}  // namespace

OT_API::OT_API(
    const api::internal::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::client::Workflow& workflow,
    const api::network::ZMQ& zmq,
    const ContextLockCallback& lockCallback)
    : api_(api)
    , activity_(activity)
    , contacts_(contacts)
    , workflow_(workflow)
    , zmq_(zmq)
    , m_strDataPath(String::Factory())
    , m_strConfigFilename(String::Factory())
    , m_strConfigFilePath(String::Factory())
    , m_pClient(nullptr)
    , lock_callback_(lockCallback)
{
    // WARNING: do not access api_.Wallet() during construction

    if (!Init()) {
        Cleanup();
        OT_FAIL;
    }

    OT_ASSERT(m_pClient);
}

void OT_API::AddHashesToTransaction(
    OTTransaction& transaction,
    const otx::context::Base& context,
    const Account& account,
    const PasswordPrompt& reason) const
{
    auto accountHash{api_.Factory().Identifier()};
    account.ConsensusHash(context, accountHash, reason);
    transaction.SetAccountHash(accountHash);

    auto accountid{api_.Factory().Identifier()};
    account.GetIdentifier(accountid);

    auto nymfile = context.Nymfile(reason);

    auto inboxHash{api_.Factory().Identifier()};
    nymfile->GetInboxHash(accountid->str(), inboxHash);
    transaction.SetInboxHash(inboxHash);

    auto outboxHash{api_.Factory().Identifier()};
    nymfile->GetOutboxHash(accountid->str(), outboxHash);
    transaction.SetOutboxHash(outboxHash);
}

void OT_API::AddHashesToTransaction(
    OTTransaction& transaction,
    const otx::context::Base& context,
    const Identifier& accountid,
    const PasswordPrompt& reason) const
{
    auto account = api_.Wallet().Account(accountid);
    AddHashesToTransaction(transaction, context, account.get(), reason);
}

// Call this once per INSTANCE of OT_API.
//
// Theoretically, someday OTAPI can be the "OT_CTX" and we will be able to
// instantiate
// multiple instances of it within a single application.
//
// So you use OT_API::InitOTAPI to initialize the entire application, and then
// you use
// OT_API::Init() to initialize THIS "OT_CTX" (the OT_API object.)
//
// If not initialized yet, but then this function is successful, it will return
// true.
// If ALREADY initialized, this function still returns true.
// If initialization fails, it will return false, but you can just call it
// again.
//
auto OT_API::Init() -> bool
{
    // WARNING: do not access api_.Wallet() during construction

    if (!LoadConfigFile()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Unable to Load Config File!")
            .Flush();

        return false;
    }

    return true;
}

auto OT_API::Cleanup() -> bool { return true; }

// Load the configuration file.
//
auto OT_API::LoadConfigFile() -> bool
{
    Lock lock(lock_);

    // PID -- Make sure we're not running two copies of OT on the same data
    // simultaneously here.
    //
    // we need to get the loacation of where the pid file should be.
    // then we pass it to the OpenPid function.
    auto strDataPath = String::Factory(api_.DataFolder().c_str());

    if (!api_.Legacy().ConfirmCreateFolder(strDataPath)) { return false; }

    // This way, everywhere else I can use the default storage context (for now)
    // and it will work everywhere I put it. (Because it's now set up...)
    m_bDefaultStore = OTDB::InitDefaultStorage(
        OTDB_DEFAULT_STORAGE,
        OTDB_DEFAULT_PACKER);  // We only need to do this once now.

    if (m_bDefaultStore) {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Success invoking OTDB::InitDefaultStorage")
            .Flush();

        m_pClient.reset(new OTClient(api_));
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed invoking OTDB::InitDefaultStorage.")
            .Flush();

        return false;
    }

    // LATENCY
    {
        const char* szComment =
            ";; LATENCY:\n\n"
            ";; - linger is the number of milliseconds OT will try to send any "
            "outgoing messages queued in a socket that's being closed.\n"
            ";; - send_timeout is the number of milliseconds OT will wait "
            "while sending a message, before it gives up.\n"
            ";; - recv_timeout is the number of milliseconds OT will wait "
            "while receiving a reply, before it gives up.\n";

        bool b_SectionExist = false;
        api_.Config().CheckSetSection(
            String::Factory("latency"),
            String::Factory(szComment),
            b_SectionExist);
    }

    // SECURITY (beginnings of..)

    // Master Key Timeout
    {
        const char* szComment =
            "; master_key_timeout is how long the master key will be in "
            "memory until a thread wipes it out.\n"
            "; 0   : This means you have to type your password EVERY time OT "
            "uses a "
            "private key. (Even multiple times in a single function.)\n"
            "; 300 : This means you only have to type it once per 5 minutes.\n"
            "; -1  : This means you only type it once PER RUN (popular for "
            "servers.)\n";

        bool bIsNewKey = false;
        std::int64_t lValue = 0;
        api_.Config().CheckSet_long(
            String::Factory("security"),
            String::Factory("master_key_timeout"),
            CLIENT_MASTER_KEY_TIMEOUT_DEFAULT,
            lValue,
            bIsNewKey,
            String::Factory(szComment));
        api_.SetMasterKeyTimeout(std::chrono::seconds(lValue));
    }

    // Done Loading... Lets save any changes...
    if (!api_.Config().Save()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error! Unable to save updated Config!!!")
            .Flush();
        OT_FAIL;
    }

    return true;
}

auto OT_API::IsNym_RegisteredAtServer(
    const identifier::Nym& NYM_ID,
    const identifier::Server& NOTARY_ID) const -> bool
{
    if (NYM_ID.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": NYM_ID is empty!").Flush();
        OT_FAIL;
    }

    auto context = api_.Wallet().ServerContext(NYM_ID, NOTARY_ID);

    if (context) { return (0 != context->Request()); }

    return false;
}

/** TIME (in seconds, as std::int64_t)

 This will return the current time in seconds, as a std::int64_t std::int32_t.

 Todo:  consider making this available on the server side as well,
 so the smart contracts can see what time it is.
 */
auto OT_API::GetTime() const -> Time { return Clock::now(); }

/// === Verify Account Receipt ===
/// Returns bool. Verifies any asset account (intermediary files) against its
/// own last signed receipt.
/// Obviously this will fail for any new account that hasn't done any
/// transactions yet (and thus has no receipts.)
///
///
auto OT_API::VerifyAccountReceipt(
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const Identifier& ACCOUNT_ID) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto context = api_.Wallet().ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(NYM_ID)(
            " is not registered on ")(NOTARY_ID)(".")
            .Flush();

        return false;
    }

    return VerifyBalanceReceipt(api_, *context, NOTARY_ID, ACCOUNT_ID, reason);
}

auto OT_API::Create_SmartContract(
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    Time VALID_FROM,                       // Default (0 or nullptr) == NOW
    Time VALID_TO,  // Default (0 or nullptr) == no expiry / cancel anytime
    bool SPECIFY_ASSETS,   // This means asset type IDs must be provided for
                           // every named account.
    bool SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                           // party.
    String& strOutput) const -> bool
{
#ifndef OPENTXS_CHAINCONFIG_SMARTCONTRACT
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed, Smart Contracts are compiler disabled.")
            .Flush();
    return false;
#endif

    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract = api_.Factory().SmartContract();
    OT_ASSERT_MSG(
        false != bool(contract),
        "OT_API::Create_SmartContract: ASSERT "
        "while trying to instantiate blank smart "
        "contract.\n");
    if (!contract->SetDateRange(VALID_FROM, VALID_TO)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to set date "
                                           "range.")
            .Flush();
        return false;
    }

    contract->specifyParties(SPECIFY_PARTIES);
    contract->specifyAssetTypes(SPECIFY_ASSETS);

    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_SetDates(
    const String& THE_CONTRACT,  // The contract, about to have the dates
                                 // changed on it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    Time VALID_FROM,                       // Default (0 or nullptr) == NOW
    Time VALID_TO,  // Default (0 or nullptr) == no expiry / cancel anytime.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)

    auto contract{api_.Factory().CronItem(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    if (!contract->SetDateRange(VALID_FROM, VALID_TO)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to set date "
                                           "range.")
            .Flush();
        return false;
    }
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_AddParty(
    const String& THE_CONTRACT,  // The contract, about to have the party added
                                 // to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& PARTY_NYM_ID,  // Optional. Some smart contracts require the
                                 // party's Nym to be specified in advance.
    const String& PARTY_NAME,    // The Party's NAME as referenced in the smart
                                 // contract. (And the scripts...)
    const String& AGENT_NAME,    // An AGENT will be added by default for this
                                 // party. Need Agent NAME.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract(api_.Factory().Scriptable(THE_CONTRACT));
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get()),
        str_agent_name(AGENT_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr != party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party already exists.")
            .Flush();
        return false;
    }

    // New param PARTY_NYM_ID must either be present, or empty, based on
    // arePartiesSpecified().
    //
    const char* szPartyNymID = nullptr;

    if (contract->arePartiesSpecified()) {
        if (!PARTY_NYM_ID.Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: Party Nym ID is empty, even though this "
                "contract is configured to require a Party's NymID to "
                "appear on the contract.")
                .Flush();
            return false;
        }

        szPartyNymID = PARTY_NYM_ID.Get();
    } else {
        if (PARTY_NYM_ID.Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: Party Nym ID was provided but erroneously so, "
                "since "
                "this contract is NOT configured to require a Party's "
                "NymID when first adding the party.")
                .Flush();
            return false;
        }
    }

    party = new OTParty(
        api_.Wallet(),
        api_.DataFolder(),
        str_party_name.c_str(),
        true /*bIsOwnerNym*/,
        szPartyNymID,
        str_agent_name.c_str(),
        true);  // bCreateAgent=false by default.
    OT_ASSERT(nullptr != party);

    if (!contract->AddParty(*party))  // takes ownership.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed while trying to add party: ")(PARTY_NAME)(".")
            .Flush();
        delete party;
        party = nullptr;
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveParty(
    const String& THE_CONTRACT,  // The contract, about to have the party
                                 // removed
                                 // from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) return false;
    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    if (contract->RemoveParty(str_party_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_AddAccount(
    const String& THE_CONTRACT,  // The contract, about to have the account
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    const String& ACCT_NAME,   // The Account's name as referenced in the smart
                               // contract
    const String& INSTRUMENT_DEFINITION_ID,  // Instrument Definition ID for the
                                             // Account.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party "
                                           "doesn't exist.")
            .Flush();
        return false;
    }
    const std::string str_name(ACCT_NAME.Get()),
        str_instrument_definition_id(INSTRUMENT_DEFINITION_ID.Get());

    if (nullptr != party->GetAccount(str_name)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed adding: "
            "account is already there with that name (")(str_name)(
            ") on "
            "party: ")(str_party_name)(".")
            .Flush();
        return false;
    }
    // -----------------------------------
    // Need to explicitly check contract->areAssetTypesSpecified() and then
    // mandate that the instrument definition ID must either be present, or not,
    // based on that.

    const char* szAssetTypeID = nullptr;

    if (contract->areAssetTypesSpecified()) {
        if (str_instrument_definition_id.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: Asset Type ID is empty, even though this "
                "contract is configured to require the Asset Types to "
                "appear on the contract.")
                .Flush();
            return false;
        }

        szAssetTypeID = str_instrument_definition_id.c_str();
    } else {
        if (!str_instrument_definition_id.empty()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: Asset Type ID was provided but erroneously "
                "so, since "
                "this contract is NOT configured to require the Asset "
                "Types when first adding the account.")
                .Flush();
            return false;
        }
    }

    const auto strAgentName = String::Factory(),
               strAcctName = String::Factory(str_name.c_str()),
               strAcctID = String::Factory();
    auto strInstrumentDefinitionID = String::Factory();

    if (nullptr != szAssetTypeID) strInstrumentDefinitionID->Set(szAssetTypeID);

    if (false == party->AddAccount(
                     strAgentName,
                     strAcctName,
                     strAcctID,
                     strInstrumentDefinitionID,
                     0)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to "
            "add account (")(str_name)(") to party: ")(str_party_name)(".")
            .Flush();
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveAccount(
    const String& THE_CONTRACT,  // The contract, about to have the account
                                 // removed from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& PARTY_NAME,  // The Party's NAME as referenced in the smart
                               // contract. (And the scripts...)
    const String& ACCT_NAME,   // The Account's name as referenced in the smart
                               // contract
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party doesn't exist.")
            .Flush();
        return false;
    }

    const std::string str_name(ACCT_NAME.Get());

    if (party->RemoveAccount(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_CountNumsNeeded(
    const String& THE_CONTRACT,  // The contract, about to have the bylaw added
                                 // to it.
    const String& AGENT_NAME) const -> std::int32_t  // An AGENT will be added
                                                     // by default for this
                                                     // party. Need Agent NAME.
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    std::int32_t nReturnValue = 0;
    const std::string str_agent_name(AGENT_NAME.Get());
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract.")
            .Flush();
        return nReturnValue;
    }
    // -- nReturnValue starts as 0.
    // -- If agent is authorizing agent for a party, nReturnValue++. (Opening
    // number.)
    // -- If agent is authorized agent for any of party's accts, nReturnValue++
    // for each. (Closing numbers.)
    //
    // (Then return the count.)

    nReturnValue = contract->GetCountTransNumsNeededForAgent(str_agent_name);
    return nReturnValue;
}

auto OT_API::SmartContract_ConfirmAccount(
    const String& THE_CONTRACT,
    const identifier::Nym& SIGNER_NYM_ID,
    const String& PARTY_NAME,
    const String& ACCT_NAME,
    const String& AGENT_NAME,
    const String& ACCT_ID,
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    const auto accountID = api_.Factory().Identifier(ACCT_ID);
    auto account = api_.Wallet().Account(accountID);

    if (false == bool(account)) return false;

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto pScriptable{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(pScriptable)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    auto* contract = dynamic_cast<OTSmartContract*>(pScriptable.get());
    if (nullptr == contract) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure casting to Smart Contract. "
            "Are you SURE it's a smart contract? Contents: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());
    OTParty* party = contract->GetParty(str_party_name);
    if (nullptr == party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party doesn't exist: ")(
            PARTY_NAME)(".")
            .Flush();
        return false;
    }
    // Make sure there's not already an account here with the same ID (Server
    // disallows.)
    //
    OTPartyAccount* pDupeAcct = party->GetAccountByID(accountID);
    if (nullptr != pDupeAcct)  // It's already there.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed, since a duplicate account ID (")(ACCT_ID)(
            ") was already found on this contract. (Server "
            "disallows, sorry).")
            .Flush();
        return false;
    }
    // Find the account template based on its name, to affix the acct ID to.
    //
    const std::string str_name(ACCT_NAME.Get());

    OTPartyAccount* partyAcct = party->GetAccount(str_name);
    if (nullptr == partyAcct)  // It's not already there. (Though it should
                               // be...)
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed: No account found on contract with name: ")(str_name)(".")
            .Flush();
        return false;
    }

    // the actual instrument definition ID

    const auto theExpectedInstrumentDefinitionID = api_.Factory().UnitID(
        partyAcct->GetInstrumentDefinitionID());  // The expected instrument
                                                  // definition ID,
                                                  // converting
                                                  // from a string.
    const auto& theActualInstrumentDefinitionID =
        account.get().GetInstrumentDefinitionID();  // the actual instrument
                                                    // definition
    // ID, already an identifier, from
    // the actual account.

    if (contract->areAssetTypesSpecified() &&
        (theExpectedInstrumentDefinitionID !=
         theActualInstrumentDefinitionID)) {
        const auto strInstrumentDefinitionID =
            String::Factory(theActualInstrumentDefinitionID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed, since the instrument definition ID of the account (")(
            strInstrumentDefinitionID)(") does not match what was expected (")(
            partyAcct->GetInstrumentDefinitionID())(
            ") according to this contract.")
            .Flush();
        return false;
    }

    // I'm leaving this here for now, since a party can only be a Nym for now
    // anyway (until I code entities.)
    // Therefore this account COULD ONLY be owned by that Nym anyway, and thus
    // will pass this test.
    // All the above functions aren't that stringent because they are about
    // designing the smart contract, not
    // executing it. But in THIS case, we are actually confirming the thing, and
    // adding our actual account #s
    // to it, signing it, etc, so I might as well save the person the hassle of
    // being rejected later because
    // he accidentally set it up with the wrong Nym.
    //
    if (!account.get().VerifyOwner(*nym)) {
        const auto strNymID = String::Factory(SIGNER_NYM_ID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed, since this nym (")(
            strNymID)(") isn't the owner of this account (")(str_name)(").")
            .Flush();
        return false;
    }
    // When the first ACCOUNT is confirmed, then at that moment, we know which
    // server
    // this smart contract is intended to execute on.
    //
    // If this is the first account being confirmed, then we will set the server
    // ID
    // for the smart contract based on the server ID of this account. Otherwise
    // if this
    // is not the first account being confirmed, then we will compare the server
    // ID
    // that's already on the smart contract, to the server ID for this account,
    // and make
    // sure they match. (Otherwise we will reject the confirmation.)
    //
    // Once the contract is activated, the server will verify all the parties
    // and accounts
    // anyway. So might as well save ourselves the hassle, if this doesn't match
    // up now.
    //
    if (contract->SetNotaryIDIfEmpty(account.get().GetPurportedNotaryID())) {
        // TODO security: possibly want to verify here that this really is the
        // FIRST account being confirmed in this smart contract, or at least the
        // first party. Right now we're just using the server ID being empty as
        // an easy way to find out, but technically a party could slip in a
        // "signed version" without setting the server ID, and it might slip by
        // here (though it would eventually fail some verification.) In the
        // std::int64_t term we'll do a more thorough check here, though.
    } else if (
        contract->GetNotaryID() != account.get().GetPurportedNotaryID()) {
        const auto strServer1 = String::Factory(contract->GetNotaryID()),
                   strServer2 =
                       String::Factory(account.get().GetPurportedNotaryID());
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The smart contract has a different server ID on it already (")(
            strServer1)(") than the one  that goes with this account (server ")(
            strServer2)(", for account ")(ACCT_ID)(").")
            .Flush();
        return false;
    }
    // BY THIS POINT, we know that the account is actually owned by the Nym,
    // and we know that it's got the proper instrument definition ID that was
    // expected
    // according to the smart contract. We also know that the smart contract
    // has the same server ID as the account being confirmed.
    //
    partyAcct->SetAcctID(String::Factory(ACCT_ID.Get()));
    partyAcct->SetAgentName(String::Factory(AGENT_NAME.Get()));
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::Smart_ArePartiesSpecified(const String& THE_CONTRACT) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }

    return contract->arePartiesSpecified();
}

auto OT_API::Smart_AreAssetTypesSpecified(const String& THE_CONTRACT) const
    -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }

    return contract->areAssetTypesSpecified();
}

auto OT_API::SmartContract_ConfirmParty(
    const String& THE_CONTRACT,  // The smart contract, about to be changed by
                                 // this function.
    const String& PARTY_NAME,    // Should already be on the contract. This way
                                 // we can find it.
    const identifier::Nym& NYM_ID,  // Nym ID for the party, the actual owner,
    const identifier::Server& NOTARY_ID,
    String& strOutput) const
    -> bool  // ===> AS WELL AS for the default AGENT of that
             // party.
             // (For now, until I code entities)
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto reason = api_.Factory().PasswordPrompt("Activating a smart contract");
    auto context =
        api_.Wallet().mutable_ServerContext(NYM_ID, NOTARY_ID, reason);
    auto nymfile = context.get().mutable_Nymfile(reason);
    auto nym = context.get().Nym();

    // By this point, nymfile is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)

    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart contract: ")(
            THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_party_name(PARTY_NAME.Get());

    OTParty* party = contract->GetParty(str_party_name);

    if (nullptr == party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party (")(
            str_party_name)(") doesn't exist, so how can you confirm it?")
            .Flush();
        return false;
    }

    if (contract->arePartiesSpecified()) {
        bool bSuccessID = false;
        const std::string partyNymID = party->GetNymID(&bSuccessID);

        if (bSuccessID && !partyNymID.empty()) {
            auto strPartyNymID = String::Factory(partyNymID);
            auto idParty = api_.Factory().NymID(strPartyNymID);

            if (idParty != NYM_ID) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Party (")(
                    str_party_name)(
                    ") has an expected NymID that doesn't match the "
                    "actual NymID.")
                    .Flush();
                return false;
            }
        }
    }

    auto* pNewParty = new OTParty(
        api_.Wallet(),
        api_.DataFolder(),
        party->GetPartyName(),
        *nym,  // party keeps an internal pointer to nym from here on.
        party->GetAuthorizingAgentName());  // Party name and agent name must
                                            // match, in order to replace /
                                            // activate this party.
    OT_ASSERT(nullptr != pNewParty);
    if (!party->CopyAcctsToConfirmingParty(*pNewParty)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed while trying to copy accounts, while "
            "confirming party: ")(PARTY_NAME)(".")
            .Flush();
        delete pNewParty;
        pNewParty = nullptr;
        return false;
    }

    if (!contract->ConfirmParty(*pNewParty, context.get(), reason)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed while trying to confirm party: ")(PARTY_NAME)(".")
            .Flush();
        delete pNewParty;
        pNewParty = nullptr;
        return false;
    }
    // ConfirmParty(), unlike most others, actually signs and saves the contract
    // already.
    // Therefore, all that's needed here is to grab it in string form...
    // (No need to sign again, it's just a waste of resource..)
    //    contract->ReleaseSignatures();
    //    contract->SignContract(*nymfile, reason);
    //    contract->SaveContract();

    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the party, we
    // have to track it until it's activated or until we cancel it.)
    //
    const auto strInstrument = String::Factory(*contract);
    auto pMessage = api_.Factory().Message();
    OT_ASSERT(false != bool(pMessage));

    const auto strNymID = String::Factory(NYM_ID);

    pMessage->m_strCommand = String::Factory("outpaymentsMessage");
    pMessage->m_strNymID = strNymID;
    //  pMessage->m_strNotaryID        = strNotaryID;
    pMessage->m_ascPayload->SetString(strInstrument);

    auto pNym = api_.Wallet().Nym(nymfile.get().ID());
    OT_ASSERT(nullptr != pNym)

    pMessage->SignContract(*pNym, reason);
    pMessage->SaveContract();

    std::shared_ptr<Message> message{pMessage.release()};
    nymfile.get().AddOutpayments(message);

    return true;
}

auto OT_API::SmartContract_AddBylaw(
    const String& THE_CONTRACT,  // The contract, about to have the bylaw added
                                 // to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    const char* BYLAW_LANGUAGE = "chai";  // todo hardcoding.
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);
    if (false == bool(nym)) return false;
    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart "
                                           "contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get()),
        str_language(BYLAW_LANGUAGE);

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr != pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Bylaw "
                                           "already exists: ")(BYLAW_NAME)(".")
            .Flush();
        return false;
    }
    pBylaw = new OTBylaw(str_bylaw_name.c_str(), str_language.c_str());
    OT_ASSERT(nullptr != pBylaw);

    if (!contract->AddBylaw(*pBylaw))  // takes ownership.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed while trying "
                                           "to add bylaw: ")(BYLAW_NAME)(".")
            .Flush();
        delete pBylaw;
        pBylaw = nullptr;
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveBylaw(
    const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                 // removed
                                 // from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart "
                                           "contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    if (contract->RemoveBylaw(str_bylaw_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_AddHook(
    const String& THE_CONTRACT,  // The contract, about to have the hook
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& HOOK_NAME,    // The Hook's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& CLAUSE_NAME,  // The actual clause that will be triggered by
                                // the hook. (You can call this multiple times,
                                // and have multiple clauses trigger on the
                                // same hook.)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart "
                                           "contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    const std::string str_name(HOOK_NAME.Get()), str_clause(CLAUSE_NAME.Get());

    if (!pBylaw->AddHook(str_name, str_clause)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to add "
                                           "hook (")(str_name)(", clause ")(
            str_clause)(") to bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveHook(
    const String& THE_CONTRACT,  // The contract, about to have the hook
                                 // removed from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& HOOK_NAME,    // The Hook's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& CLAUSE_NAME,  // The actual clause that will be triggered by
                                // the hook. (You can call this multiple times,
                                // and have multiple clauses trigger on the
                                // same hook.)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error loading smart "
                                           "contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }

    const std::string str_name(HOOK_NAME.Get()), str_clause(CLAUSE_NAME.Get());

    if (pBylaw->RemoveHook(str_name, str_clause)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_AddCallback(
    const String& THE_CONTRACT,  // The contract, about to have the callback
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& CALLBACK_NAME,  // The Callback's name as referenced in the
                                  // smart contract. (And the scripts...)
    const String& CLAUSE_NAME,    // The actual clause that will be triggered by
                                  // the callback. (Must exist.)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    const std::string str_name(CALLBACK_NAME.Get()),
        str_clause(CLAUSE_NAME.Get());

    if (nullptr != pBylaw->GetCallback(str_name)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: "
                                           "Callback (")(str_name)(
            ") already exists on bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    if (!pBylaw->AddCallback(str_name.c_str(), str_clause.c_str())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to "
                                           "add callback (")(str_name)(
            ", clause ")(str_clause)(") to bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveCallback(
    const String& THE_CONTRACT,  // The contract, about to have the callback
                                 // removed from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& CALLBACK_NAME,  // The Callback's name as referenced in the
                                  // smart contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    const std::string str_name(CALLBACK_NAME.Get());

    if (pBylaw->RemoveCallback(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_AddClause(
    const String& THE_CONTRACT,  // The contract, about to have the clause
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& SOURCE_CODE,  // The actual source code for the clause.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Bylaw "
                                           "doesn't exist: ")(str_bylaw_name)(
            ". Input contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get()), str_code(SOURCE_CODE.Get());

    if (nullptr != pBylaw->GetClause(str_name)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed adding: "
            "clause is already there with that name (")(str_name)(
            ") on "
            "bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    if (!pBylaw->AddClause(str_name.c_str(), str_code.c_str())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to "
            "add clause (")(str_name)(") to bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_UpdateClause(
    const String& THE_CONTRACT,  // The contract, about to have the clause
                                 // updated on it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    const String& SOURCE_CODE,  // The actual source code for the clause.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Bylaw "
                                           "doesn't exist: ")(str_bylaw_name)(
            ". Input contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get()), str_code(SOURCE_CODE.Get());

    if (pBylaw->UpdateClause(str_name, str_code)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_RemoveClause(
    const String& THE_CONTRACT,  // The contract, about to have the clause
                                 // removed from it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,   // Should already be on the contract. (This way
                                // we can find it.)
    const String& CLAUSE_NAME,  // The Clause's name as referenced in the smart
                                // contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: Bylaw "
                                           "doesn't exist: ")(str_bylaw_name)(
            " Input contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_name(CLAUSE_NAME.Get());

    if (pBylaw->RemoveClause(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

auto OT_API::SmartContract_AddVariable(
    const String& THE_CONTRACT,  // The contract, about to have the variable
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& VAR_NAME,    // The Variable's name as referenced in the smart
                               // contract. (And the scripts...)
    const String& VAR_ACCESS,  // "constant", "persistent", or "important".
    const String& VAR_TYPE,    // "string", "std::int64_t", or "bool"
    const String& VAR_VALUE,   // Contains a string. If type is :std::int64_t,
                               // atol()
    // will be used to convert value to a std::int64_t. If
    // type is bool, the strings "true" or "false"
    // are expected here in order to convert to a
    // bool.
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    const std::string str_name(VAR_NAME.Get()), str_access(VAR_ACCESS.Get()),
        str_type(VAR_TYPE.Get()), str_value(VAR_VALUE.Get());

    if (nullptr != pBylaw->GetVariable(str_name)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: "
                                           "Variable (")(str_name)(
            ") already exists on bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    OTVariable::OTVariable_Access theAccess = OTVariable::Var_Error_Access;

    if (str_access.compare("constant") == 0)
        theAccess = OTVariable::Var_Constant;
    else if (str_access.compare("persistent") == 0)
        theAccess = OTVariable::Var_Persistent;
    else if (str_access.compare("important") == 0)
        theAccess = OTVariable::Var_Important;
    OTVariable::OTVariable_Type theType = OTVariable::Var_Error_Type;

    if (str_type.compare("bool") == 0)
        theType = OTVariable::Var_Bool;
    else if (str_type.compare("integer") == 0)
        theType = OTVariable::Var_Integer;
    else if (str_type.compare("string") == 0)
        theType = OTVariable::Var_String;
    if ((OTVariable::Var_Error_Type == theType) ||
        (OTVariable::Var_Error_Access == theAccess)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed due to bad "
                                           "variable type or bad access type.")
            .Flush();
        return false;
    }
    bool bAdded = false;

    switch (theType) {
        case OTVariable::Var_Bool: {
            const bool bValue = (str_value.compare("true") == 0);
            bAdded = pBylaw->AddVariable(str_name, bValue, theAccess);
        } break;
        case OTVariable::Var_Integer: {
            const std::int32_t nValue = atoi(str_value.c_str());
            bAdded = pBylaw->AddVariable(str_name, nValue, theAccess);
        } break;
        case OTVariable::Var_String: {
            bAdded = pBylaw->AddVariable(str_name, str_value, theAccess);
        } break;
        default:
            // SHOULD NEVER HAPPEN (We already return above, if the variable
            // type
            // isn't correct.)
            OT_FAIL_MSG("Should never happen. You aren't seeing this.");
    }

    if (!bAdded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to "
            "add variable (")(str_name)(") to bylaw: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    // Success!
    //
    contract->ReleaseSignatures();
    contract->SignContract(*nym, reason);
    contract->SaveContract();
    strOutput.Release();
    contract->SaveContractRaw(strOutput);
    return true;
}

auto OT_API::SmartContract_RemoveVariable(
    const String& THE_CONTRACT,  // The contract, about to have the variable
                                 // added to it.
    const identifier::Nym& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
    const String& BYLAW_NAME,  // Should already be on the contract. (This way
                               // we can find it.)
    const String& VAR_NAME,    // The Variable's name as referenced in the smart
                               // contract. (And the scripts...)
    String& strOutput) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto nym = api_.Wallet().Nym(SIGNER_NYM_ID);

    if (false == bool(nym)) { return false; }

    // By this point, nym is a good pointer, and is on the wallet. (No need
    // to
    // cleanup.)
    auto contract{api_.Factory().Scriptable(THE_CONTRACT)};
    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error loading "
            "smart contract: ")(THE_CONTRACT)(".")
            .Flush();
        return false;
    }
    const std::string str_bylaw_name(BYLAW_NAME.Get());

    OTBylaw* pBylaw = contract->GetBylaw(str_bylaw_name);

    if (nullptr == pBylaw) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Bylaw "
            "doesn't exist: ")(str_bylaw_name)(".")
            .Flush();
        return false;
    }
    const std::string str_name(VAR_NAME.Get());

    if (pBylaw->RemoveVariable(str_name)) {
        // Success!
        //
        contract->ReleaseSignatures();
        contract->SignContract(*nym, reason);
        contract->SaveContract();
        strOutput.Release();
        contract->SaveContractRaw(strOutput);
        return true;
    }

    return false;
}

// WRITE CHEQUE
//
// Returns an OTCheque pointer, or nullptr.
// (Caller responsible to delete.)
auto OT_API::WriteCheque(
    const identifier::Server& NOTARY_ID,
    const std::int64_t& CHEQUE_AMOUNT,
    const Time& VALID_FROM,
    const Time& VALID_TO,
    const Identifier& SENDER_accountID,
    const identifier::Nym& SENDER_NYM_ID,
    const String& CHEQUE_MEMO,
    const identifier::Nym& pRECIPIENT_NYM_ID) const -> Cheque*
{
    rLock lock(lock_callback_({SENDER_NYM_ID.str(), NOTARY_ID.str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto context =
        api_.Wallet().mutable_ServerContext(SENDER_NYM_ID, NOTARY_ID, reason);
    auto nym = context.get().Nym();
    auto account = api_.Wallet().Account(SENDER_accountID);

    if (false == bool(account)) { return nullptr; }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)

    // To write a cheque, we need to burn one of our transaction numbers.
    // (Presumably the wallet is also storing a couple of these, since they are
    // needed to perform any transaction.)
    //
    // I don't have to contact the server to write a cheque -- as long as I
    // already have a transaction number I can use to write it with. (Otherwise
    // I'd have to ask the server to send me one first.)
    auto strNotaryID = String::Factory(NOTARY_ID);
    const auto number =
        context.get().NextTransactionNumber(MessageType::notarizeTransaction);

    if (false == number->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": User attempted to write a cheque, but had no "
            "transaction numbers.")
            .Flush();

        return nullptr;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        number->Value())(".")
        .Flush();

    // At this point, I know that number contains one I can use.
    auto pCheque{api_.Factory().Cheque(
        account.get().GetRealNotaryID(),
        account.get().GetInstrumentDefinitionID())};
    OT_ASSERT_MSG(
        false != bool(pCheque),
        "OT_API::WriteCheque: Error allocating memory in the OT API.");
    // At this point, I know that pCheque is a good pointer that I either
    // have to delete, or return to the caller.
    bool bIssueCheque = pCheque->IssueCheque(
        CHEQUE_AMOUNT,
        number->Value(),
        VALID_FROM,
        VALID_TO,
        SENDER_accountID,
        SENDER_NYM_ID,
        CHEQUE_MEMO,
        pRECIPIENT_NYM_ID);

    if (!bIssueCheque) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure calling OTCheque::IssueCheque().")
            .Flush();

        return nullptr;
    }

    pCheque->SignContract(*nym, reason);
    pCheque->SaveContract();
    auto workflow = workflow_.WriteCheque(*pCheque);

    if (workflow->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create workflow.")
            .Flush();

        return nullptr;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Started workflow ")(workflow)(
            ".")
            .Flush();
    }

    // Above this line, the transaction number will be recovered automatically
    number->SetSuccess(true);

    return pCheque.release();
}

// PROPOSE PAYMENT PLAN  (MERCHANT calls this function)
//
// Returns an OTPaymentPlan pointer, or nullptr.
// (Caller responsible to delete.)
//
// The process (finally) is:
//
// 1) Payment plan is written, and signed, by the recipient. (Merchant.)
// 2) He sends it to the sender, who signs it and submits it. (Payer /
// Customer.)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
//
//
// Payment Plan Delay, and Payment Plan Period, both default to 30 days (if you
// pass 0.)
// Payment Plan Length, and Payment Plan Max Payments, both default to 0, which
// means
// no maximum length and no maximum number of payments.
//
// WARNING: the OTPaymentPlan object being returned, contains 2 transaction
// numbers of the Recipient.
// If you delete that object without actually activating it, make sure you
// retrieve those transaction
// numbers first, and add them BACK to the Recipient's Nym!!
//
// Furthermore, recipient should keep a COPY of this proposal after making it,
// so that he can retrieve the transaction numbers from it, for the same reason.
auto OT_API::ProposePaymentPlan(
    const identifier::Server& NOTARY_ID,
    const Time& VALID_FROM,  // Default (0) == NOW (It will set it to the
                             // current time in seconds since Jan 1970)
    const Time& VALID_TO,    // Default (0) == no expiry / cancel anytime.
                             // Otherwise this is a LENGTH and is ADDED to
                             // VALID_FROM
    const Identifier& pSENDER_accountID,
    const identifier::Nym& SENDER_NYM_ID,
    const String& PLAN_CONSIDERATION,  // Like a memo.
    const Identifier& RECIPIENT_accountID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    // ----------------------------------------  // If it's above zero, the
    // initial
    const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // amount will be processed
                                                 // after
    const std::chrono::seconds INITIAL_PAYMENT_DELAY,  // delay (seconds from
                                                       // now.)
    // ----------------------------------------  // AND SEPARATELY FROM THIS...
    const std::int64_t& PAYMENT_PLAN_AMOUNT,  // The regular amount charged,
    const std::chrono::seconds PAYMENT_PLAN_DELAY,   // which begins occuring
                                                     // after delay
    const std::chrono::seconds PAYMENT_PLAN_PERIOD,  // (seconds from now) and
                                                     // happens
    // ----------------------------------------// every period, ad infinitum,
    const std::chrono::seconds PAYMENT_PLAN_LENGTH,  // until after the length
                                                     // (in seconds)
    const std::int32_t PAYMENT_PLAN_MAX_PAYMENTS     // expires, or after the
                                                     // maximum
) const -> OTPaymentPlan*  // number of payments. These last
{                          // two arguments are optional.
#ifndef OPENTXS_CHAINCONFIG_PAYMENTPLAN
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed, Payment Plans are compiler disabled.")
            .Flush();
    return nullptr;
#endif
    auto reason = api_.Factory().PasswordPrompt("Proposing a payment plan");
    auto context = api_.Wallet().mutable_ServerContext(
        RECIPIENT_NYM_ID, NOTARY_ID, reason);
    auto nymfile = context.get().mutable_Nymfile(reason);
    auto nym = context.get().Nym();
    auto account = api_.Wallet().Account(RECIPIENT_accountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return nullptr;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    std::unique_ptr<OTPaymentPlan> pPlan = nullptr;

    // We don't always know the sender's account ID at the time of the proposal.
    // (The sender is the payer aka customer, who selects his account at the
    // time of
    // confirmation, which is after the merchant has created the proposal (here)
    // and
    // sent it to him.)
    if (pSENDER_accountID.empty()) {
        pPlan.reset(
            api_.Factory()
                .PaymentPlan(
                    NOTARY_ID, account.get().GetInstrumentDefinitionID())
                .release());

        OT_ASSERT_MSG(
            false != bool(pPlan),
            "OT_API::ProposePaymentPlan: 1 Error allocating "
            "memory in the OT API for new "
            "OTPaymentPlan.\n");

        pPlan->setCustomerNymId(SENDER_NYM_ID);
        pPlan->SetRecipientNymID(RECIPIENT_NYM_ID);
        pPlan->SetRecipientAcctID(RECIPIENT_accountID);
    } else {
        pPlan.reset(api_.Factory()
                        .PaymentPlan(
                            NOTARY_ID,
                            account.get().GetInstrumentDefinitionID(),
                            pSENDER_accountID,
                            SENDER_NYM_ID,
                            RECIPIENT_accountID,
                            RECIPIENT_NYM_ID)
                        .release());

        OT_ASSERT_MSG(
            false != bool(pPlan),
            "OT_API::ProposePaymentPlan: 2 Error allocating "
            "memory in the OT API for new "
            "OTPaymentPlan.\n");
    }
    // At this point, I know that pPlan is a good pointer that I either
    // have to delete, or return to the caller. CLEANUP WARNING!
    bool bSuccessSetProposal = pPlan->SetProposal(
        context.get(), account.get(), PLAN_CONSIDERATION, VALID_FROM, VALID_TO);
    // WARNING!!!! SetProposal() burns TWO transaction numbers for RECIPIENT.
    // (*nymfile)
    // BELOW THIS POINT, if you have an error, then you must retrieve those
    // numbers from
    // the plan, and set them BACK on nymfile before you return!!!
    const auto strNotaryID = String::Factory(NOTARY_ID);

    if (!bSuccessSetProposal) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to set the proposal.")
            .Flush();
        pPlan->HarvestOpeningNumber(context.get());
        pPlan->HarvestClosingNumbers(context.get());
        return nullptr;
    }
    // the default, in case user chooses not to even have this payment.
    bool bSuccessSetInitialPayment = true;
    // the default, in case user chooses not to have a payment plan.
    bool bSuccessSetPaymentPlan = true;
    if ((INITIAL_PAYMENT_AMOUNT > 0) &&
        (INITIAL_PAYMENT_DELAY >= std::chrono::seconds{0})) {
        // The Initial payment delay is measured in seconds, starting from the
        // "Creation Date".
        bSuccessSetInitialPayment = pPlan->SetInitialPayment(
            INITIAL_PAYMENT_AMOUNT, INITIAL_PAYMENT_DELAY);
    }
    if (!bSuccessSetInitialPayment) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to set the initial payment.")
            .Flush();
        pPlan->HarvestOpeningNumber(context.get());
        pPlan->HarvestClosingNumbers(context.get());
        return nullptr;
    }
    //
    //                  " 6 minutes    ==      360 Seconds\n"
    //                  "10 minutes    ==      600 Seconds\n"
    //                  "1 hour        ==     3600 Seconds\n"
    //                  "1 day        ==    86400 Seconds\n"
    //                  "30 days        ==  2592000 Seconds\n"
    //                  "3 months        ==  7776000 Seconds\n"
    //                  "6 months        == 15552000 Seconds\n\n"
    //
    if (PAYMENT_PLAN_AMOUNT > 0)  // If there are regular payments.
    {
        // The payment plan delay is measured in seconds, starting from the
        // "Creation Date".
        std::chrono::seconds PAYMENT_DELAY = std::chrono::hours{24 * 30};

        if (PAYMENT_PLAN_DELAY > std::chrono::seconds{0})
            PAYMENT_DELAY = PAYMENT_PLAN_DELAY;
        // Defaults to 30 days, measured in seconds (if you pass 0.)
        std::chrono::seconds PAYMENT_PERIOD = std::chrono::hours{24 * 30};

        if (PAYMENT_PLAN_PERIOD > std::chrono::seconds{0})
            PAYMENT_PERIOD = PAYMENT_PLAN_PERIOD;
        // Defaults to 0 seconds (for no max length).
        std::chrono::seconds PLAN_LENGTH = std::chrono::seconds{0};

        if (PAYMENT_PLAN_LENGTH > std::chrono::seconds{0})
            PLAN_LENGTH = PAYMENT_PLAN_LENGTH;
        std::int32_t nMaxPayments =
            0;  // Defaults to 0 maximum payments (for no maximum).

        if (PAYMENT_PLAN_MAX_PAYMENTS > 0)
            nMaxPayments = PAYMENT_PLAN_MAX_PAYMENTS;
        bSuccessSetPaymentPlan = pPlan->SetPaymentPlan(
            PAYMENT_PLAN_AMOUNT,
            PAYMENT_DELAY,
            PAYMENT_PERIOD,
            PLAN_LENGTH,
            nMaxPayments);
    }
    if (!bSuccessSetPaymentPlan) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to set the payment plan.")
            .Flush();
        pPlan->HarvestOpeningNumber(context.get());
        pPlan->HarvestClosingNumbers(context.get());
        return nullptr;
    }
    pPlan->SignContract(*nym, reason);  // Here we have saved the MERCHANT's
    pPlan->SaveContract();  // VERSION. A copy of this will be attached to the
                            // CUSTOMER's version as well.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to propose the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const auto strInstrument = String::Factory(*pPlan);
    auto pMessage = api_.Factory().Message();
    OT_ASSERT(false != bool(pMessage));

    const auto strNymID = String::Factory(RECIPIENT_NYM_ID),
               strNymID2 = String::Factory(SENDER_NYM_ID);

    pMessage->m_strCommand = String::Factory("outpaymentsMessage");
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strNotaryID = strNotaryID;
    pMessage->m_ascPayload->SetString(strInstrument);

    pMessage->SignContract(*nym, reason);
    pMessage->SaveContract();

    std::shared_ptr<Message> message{pMessage.release()};
    nymfile.get().AddOutpayments(message);

    return pPlan.release();
}

// CONFIRM PAYMENT PLAN  (CUSTOMER)
//
// Returns an OTPaymentPlan pointer, or nullptr.
// (Caller responsible to delete.)
//
// The process (currently) is:
//
// 1) Payment plan is written, and signed, by the recipient. (Merchant.)
// 2) He sends it to the sender, who signs it and submits it. (Payer /
// Customer.)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
auto OT_API::ConfirmPaymentPlan(
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const Identifier& SENDER_accountID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    OTPaymentPlan& thePlan) const -> bool
{
#ifndef OPENTXS_CHAINCONFIG_PAYMENTPLAN
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed, Payment Plans are compiler disabled.")
            .Flush();
    return false;
#endif
    rLock lock(lock_callback_({SENDER_NYM_ID.str(), NOTARY_ID.str()}));
    auto reason = api_.Factory().PasswordPrompt("Activating a payment plan");
    auto context =
        api_.Wallet().mutable_ServerContext(SENDER_NYM_ID, NOTARY_ID, reason);
    auto nymfile = context.get().mutable_Nymfile(reason);
    auto nym = context.get().Nym();
    auto account = api_.Wallet().Account(SENDER_accountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return false;
    }

    // By this point, account is a good pointer, and is on the wallet. (No need
    // to cleanup.)
    auto pMerchantNym = api_.Wallet().Nym(RECIPIENT_NYM_ID);

    if (!pMerchantNym)  // We don't have this Nym in our storage already.
    {
        const auto strRecinymfileID = String::Factory(RECIPIENT_NYM_ID);
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: First you need to download the missing "
            "(Merchant) Nym's credentials: ")(strRecinymfileID)(".")
            .Flush();
        return false;
    }
    // pMerchantNym is also good, and has an angel. (No need to cleanup.)
    // --------------------------------------------------------
    // The "Creation Date" of the agreement is re-set here.
    //
    bool bConfirmed = thePlan.Confirm(
        context.get(), account.get(), RECIPIENT_NYM_ID, pMerchantNym.get());
    //
    // WARNING:  The call to "Confirm()" uses TWO transaction numbers from
    // nymfile!
    // If you don't end up actually USING this payment plan, then you need to
    // retrieve
    // those numbers and Add them back to nymfile again.
    // A nice client will store a copy of any payment plans until they are
    // closed out, or canceled,
    // or whatever.
    // ANY FAILURES BELOW THIS POINT need to be smart enough to retrieve those
    // numbers before returning.
    //
    const auto strNotaryID = String::Factory(NOTARY_ID);

    if (!bConfirmed) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed trying to confirm the agreement.")
            .Flush();
        thePlan.HarvestOpeningNumber(context.get());
        thePlan.HarvestClosingNumbers(context.get());
        return false;
    }
    thePlan.SignContract(*nym, reason);  // Here we have saved the CUSTOMER's
    thePlan.SaveContract();  // version, which contains a copy of the merchant's
                             // version.
    //
    // DROP A COPY into the Outpayments box...
    //
    // (Since we used a transaction number to confirm the plan, we
    // have to track it until it's deposited or until we cancel it.)
    //
    const auto strInstrument = String::Factory(thePlan);
    auto pMessage = api_.Factory().Message();
    OT_ASSERT(false != bool(pMessage));

    const auto strNymID = String::Factory(SENDER_NYM_ID),
               strNymID2 = String::Factory(RECIPIENT_NYM_ID);

    pMessage->m_strCommand = String::Factory("outpaymentsMessage");
    pMessage->m_strNymID = strNymID;
    pMessage->m_strNymID2 = strNymID2;
    pMessage->m_strNotaryID = strNotaryID;
    pMessage->m_ascPayload->SetString(strInstrument);

    pMessage->SignContract(*nym, reason);
    pMessage->SaveContract();

    std::shared_ptr<Message> message{pMessage.release()};
    nymfile.get().AddOutpayments(message);

    return true;
}

// LOAD NYMBOX
//
// Caller IS responsible to delete
auto OT_API::LoadNymbox(
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& NYM_ID) const -> std::unique_ptr<Ledger>
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto context = api_.Wallet().ServerContext(NYM_ID, NOTARY_ID);

    if (false == bool(context)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(NYM_ID)(
            " is not registered on ")(NOTARY_ID)(".")
            .Flush();

        return nullptr;
    }

    auto nym = context->Nym();
    auto pLedger{
        api_.Factory().Ledger(NYM_ID, NYM_ID, NOTARY_ID, ledgerType::nymbox)};

    OT_NEW_ASSERT_MSG(
        false != bool(pLedger), "Error allocating memory in the OT API.");

    if (pLedger->LoadNymbox() && pLedger->VerifyAccount(*nym)) {

        return pLedger;
    }

    return nullptr;
}

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given instrument definition is actually a
// basket currency.
//
auto OT_API::IsBasketCurrency(
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
    -> bool  // returns true or false.
{
    std::shared_ptr<proto::UnitDefinition> contract;

    bool loaded = api_.Storage().Load(
        BASKET_INSTRUMENT_DEFINITION_ID.str(), contract, true);

    if (!loaded) { return false; }

    return (proto::UNITTYPE_BASKET == contract->type());
}

// Get Basket Count (of member currency types.)
//
// Returns the number of instrument definitions that make up this basket.
// (Or zero.)
//
auto OT_API::GetBasketMemberCount(
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
    -> std::int32_t
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    api_.Storage().Load(
        BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) { return 0; }

    if (proto::UNITTYPE_BASKET != serialized->type()) { return 0; }

    return serialized->basket().item_size();
}

// Get Basket Member Asset Type
//
// Returns one of the instrument definitions that make up this basket,
// by index, and true.
// (Or false.)
//
auto OT_API::GetBasketMemberType(
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
    std::int32_t nIndex,
    identifier::UnitDefinition& theOutputMemberType) const -> bool
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    api_.Storage().Load(
        BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) { return false; }

    if (proto::UNITTYPE_BASKET != serialized->type()) { return false; }

    if ((nIndex >= serialized->basket().item_size()) || (nIndex < 0)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Index out of bounds: ")(nIndex)(
            ".")
            .Flush();

        return false;
    }

    theOutputMemberType.SetString(serialized->basket().item(nIndex).unit());

    return true;
}

// Get Basket Member Minimum Transfer Amount
//
// Returns the minimum transfer amount for one of the instrument definitions
// that
// makes up this basket, by index.
// (Or 0.)
//
auto OT_API::GetBasketMemberMinimumTransferAmount(
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
    std::int32_t nIndex) const -> std::int64_t
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    api_.Storage().Load(
        BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) { return 0; }

    if (proto::UNITTYPE_BASKET != serialized->type()) { return 0; }

    if ((nIndex >= serialized->basket().item_size()) || (nIndex < 0)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Index out of bounds: ")(nIndex)(
            ".")
            .Flush();
        return 0;
    }

    return serialized->basket().item(nIndex).weight();
}

// Get Basket Minimum Transfer Amount
//
// Returns the minimum transfer amount for the basket.
// (Or 0.)
//
auto OT_API::GetBasketMinimumTransferAmount(
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID) const
    -> std::int64_t
{
    std::shared_ptr<proto::UnitDefinition> serialized;
    api_.Storage().Load(
        BASKET_INSTRUMENT_DEFINITION_ID.str(), serialized, true);

    if (!serialized) { return 0; }

    if (proto::UNITTYPE_BASKET != serialized->type()) { return 0; }

    return serialized->basket().weight();
}

// ADD BASKET CREATION ITEM
//
// Used for creating a request to generate a new basket currency.
auto OT_API::AddBasketCreationItem(
    proto::UnitDefinition& basketTemplate,
    const String& currencyID,
    const std::uint64_t weight) const -> bool
{
    auto item = basketTemplate.mutable_basket()->add_item();

    if (nullptr == item) { return false; }

    item->set_version(1);
    item->set_weight(weight);
    item->set_unit(currencyID.Get());

    return true;
}

// Create a basket account, which is like an issuer account, but based on a
// basket of other instrument definitions. This way, users can trade with
// what is apparently a single currency, when in fact the issuence is
// delegated and distributed across multiple issuers.
//
// The user signs and saves the contract, but once the server gets it, the
// server releases signatures and signs it, calculating the hash from the
// result, in order to form the ID.
//
// The result is the same as any other currency contract, but with the
// server's signature on it (and thus it must store the server's public
// key). The server handles all transactions in and out of the basket
// currency based upon the rules set up by the user.
//
// The user who created the currency has no more control over it. The server
// reserves the right to exchange out to the various users and close the
// basket.
auto OT_API::issueBasket(
    otx::context::Server& context,
    const proto::UnitDefinition& basket,
    const std::string& label) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::issueBasket,
        api_.Factory().Armored(api_.Factory().Data(basket)),
        api_.Factory().Identifier(),
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason,
        label);

    return output;
}

// GENERATE BASKET EXCHANGE REQUEST
//
// Creates a new request (for exchanging funds in/out of a Basket).
// (Each currency in this request will be added with
// subsequent calls to OT_API::GenerateBasketItem()).
//
// (Caller is responsible to delete.)
auto OT_API::GenerateBasketExchange(
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
    const Identifier& accountID,
    std::int32_t TRANSFER_MULTIPLE) const -> Basket*
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto context =
        api_.Wallet().mutable_ServerContext(NYM_ID, NOTARY_ID, reason);
    auto nym = context.get().Nym();

    try {
        auto contract =
            api_.Wallet().BasketContract(BASKET_INSTRUMENT_DEFINITION_ID);
        // By this point, contract is a good pointer, and is on the wallet. (No
        // need to cleanup.)
        auto account = api_.Wallet().Account(accountID);

        if (false == bool(account)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.")
                .Flush();

            return nullptr;
        }

        // By this point, account is a good pointer, and is on the wallet. (No
        // need to cleanup.)
        if (BASKET_INSTRUMENT_DEFINITION_ID !=
            account.get().GetInstrumentDefinitionID()) {
            const auto strAcctID = String::Factory(accountID),
                       strAcctTypeID =
                           String::Factory(BASKET_INSTRUMENT_DEFINITION_ID);
            LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong instrument "
                                               "definition ID "
                                               "on account ")(strAcctID)(
                " (expected type to be ")(strAcctTypeID)(").")
                .Flush();
            return nullptr;
        }
        // By this point, I know that everything checks out. Signature and
        // Account ID. account is good, and no need to clean it up.
        auto strNotaryID = String::Factory(NOTARY_ID);

        std::int32_t nTransferMultiple = 1;

        if (TRANSFER_MULTIPLE > 0) nTransferMultiple = TRANSFER_MULTIPLE;

        // Next load the Basket object out of that contract.
        std::unique_ptr<Basket> pRequestBasket = nullptr;

        // We need a transaction number just to send this thing. Plus, we need a
        // number for each sub-account to the basket, as well as the basket's
        // main account. That is: 1 + theBasket.Count() + 1
        const std::size_t currencies = contract->Currencies().size();

        if (context.get().AvailableNumbers() < (2 + currencies)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": You don't have "
                "enough transaction numbers to perform the "
                "exchange.")
                .Flush();
        } else {
            pRequestBasket.reset(api_.Factory()
                                     .Basket(currencies, contract->Weight())
                                     .release());
            OT_ASSERT_MSG(
                false != bool(pRequestBasket),
                "OT_API::GenerateBasketExchange: Error allocating "
                "memory in the OT API");

            pRequestBasket->SetTransferMultiple(
                nTransferMultiple);  // This stays in this function.

            // Make sure the server knows where to put my new basket currency
            // funds,
            // once the exchange is done.
            pRequestBasket->SetRequestAccountID(accountID);  // This stays too

            // Export the Basket object into a string, add it as
            // a payload on my request, and send to server.
            pRequestBasket->SignContract(*nym, reason);
            pRequestBasket->SaveContract();
        }  // *nymfile apparently has enough transaction numbers to exchange the
        // basket.

        return pRequestBasket.release();
    } catch (...) {

        return nullptr;
    }
}

// ADD BASKET EXCHANGE ITEM
auto OT_API::AddBasketExchangeItem(
    const identifier::Server& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    Basket& theBasket,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const Identifier& ASSET_ACCOUNT_ID) const -> bool
{
    rLock lock(lock_callback_({NYM_ID.str(), NOTARY_ID.str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto context =
        api_.Wallet().mutable_ServerContext(NYM_ID, NOTARY_ID, reason);
    auto nym = context.get().Nym();

    try {
        api_.Wallet().UnitDefinition(INSTRUMENT_DEFINITION_ID);
    } catch (...) {
        return false;
    }

    auto account = api_.Wallet().Account(ASSET_ACCOUNT_ID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return false;
    }
    if (false == bool(account)) { return false; }

    // By this point, account is a good pointer, and is on the wallet. (No
    // need to cleanup.)
    if (context.get().AvailableNumbers() < 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": You need at least one "
            "transaction number to add this exchange item.")
            .Flush();
        return false;
    }

    if (INSTRUMENT_DEFINITION_ID != account.get().GetInstrumentDefinitionID()) {
        const auto strInstrumentDefinitionID =
                       String::Factory(INSTRUMENT_DEFINITION_ID),
                   strAcctID = String::Factory(ASSET_ACCOUNT_ID);
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong instrument "
                                           "definition ID "
                                           "on account ")(strAcctID)(
            " (expected to find instrument definition ")(
            strInstrumentDefinitionID)(").")
            .Flush();
        return false;
    }
    // By this point, I know that everything checks out. Signature and
    // Account ID. account is good, and no need to clean it up.
    const auto strNotaryID = String::Factory(NOTARY_ID);
    const auto number =
        context.get().NextTransactionNumber(MessageType::notarizeTransaction);

    if (false == number->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed getting next "
                                           "transaction number.")
            .Flush();

        return false;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        number->Value())(".")
        .Flush();
    number->SetSuccess(true);
    theBasket.AddRequestSubContract(
        INSTRUMENT_DEFINITION_ID, ASSET_ACCOUNT_ID, number->Value());
    theBasket.ReleaseSignatures();
    theBasket.SignContract(*nym, reason);
    theBasket.SaveContract();

    return true;
}

// TODO!  Either stop getting the next transaction numbers above, and do it
// all in a big loop in the next function below, OR, provide a way to
// harvest those numbers back again in the event of error (ugh.)
//

// TODO: It was a mistake to only drop Nymbox notices for successful
// messages. If the client misses a failure message, the client will then
// fail to claw back his transaction numbers from that failure. For
// transactions, at least, failure notices should also be dropped.

// BUT perhaps we can avoid many failure notices, by nipping things in the
// bud. For example, if the NymboxHash is wrong, we can reject transactions
// before even the opening number is burned (similar to rejecting messages
// based on bad request
// #.)
// This "message level" rejection allows the client to claw-back or re-try,
// without
// the pain of having burned SOME numbers while others are still good (and
// then having to reconcile that mess to get back into sync.)

// Some notes on TRANSACTION NUMBERS:
/*
 1. You must BURN a number even to ATTEMPT a transaction.

 2. However, remember that OT has messages, and those messages contain
 transactions.
    A message might fail due to out-of-sync request number, meaning it was
 cut off before even having a chance to call the NotarizeTransaction code.

 3. If the message failed, that means the transaction has not yet even been
 tried, so
    the number on it is still good. (You can re-send the message.)

 4. But if the message succeeded, then the transaction itself requires 2
 steps: the balance agreement, and the transaction itself.

 5. If the balance agreement or transaction fails, then the primary
 transaction # has been burned. BUT there may be OTHER transaction numbers
 that are STILL good. The closing numbers, etc.

 6. The OTHER numbers are only burned if the transaction is a SUCCESS.


 Therefore:
 -- If the message fails, we can re-sync and re-try. All trans #s are still
 good.
 -- If the message succeeds, but balance agreement fails, or the transaction
 fails,
    then the primary transaction # is burned, but any additional numbers are
 still good.
 -- If the transaction itself succeeds, then ALL numbers are burned.

 OT Client, if the MESSAGE was a failure, MUST re-send it, or claw back the
 numbers, since
 they are all still good, even the primary #. (This is only if an EXPLICIT
 message failure
 is received. A null reply could mean it was SUCCESSFUL but we just didn't
 SEE that success because we never got the reply! In that case, we do NOT
 want to re-try, nor do we want to claw the numbers back.)

 Thus OT client, as long as the MESSAGE was a success, can operate based on
 the assumption that the primary transaction number IS burned, whether the
 transaction itself was successful or not.

 OT client also assumes the other numbers are burned as well, UNLESS IT
 RECEIVES A FAILURE REPLY. If an explicit transaction failure is received,
 then OT client can CLAW BACK all of the transaction numbers EXCEPT the
 primary one.

 OT MUST SEE THE FAILURE MESSAGES -- AT LEAST FOR TRANSACTIONS!
 What if:

 What if I try a TRANSACTION, and fail to receive the server reply?
 Potentialities:

 1. Message was a failure. (No numbers are burned.)
 2. Message was success but balance agreement was a failure.
    (The opening # was burned, but the others are still good.)
 3. Message and balance agreement were both success, but transaction itself
 failed.
    (The opening # was burned, but the others are still good.)
 4. Message, balance agreement, and transaction were all a success.
    (ALL transaction #s on the message are burned.)


 Currently:
 1. If I don't receive this failure message, then I resend, which would
 basically work since the numbers are all still good.
    BUT IN THE CASE OF EXCHANGE BASKET, IT FUCKING GRABS A NEW NUMBER EACH
 CALL! So when OTAPI_Func retries, it grabs a new one each time. (Instead of
 just using the ones that are still good.)

 2. If I miss this success reply, or treat the null as a failure, then I
 will end up re-trying a transaction that has no prayer of working, since
 its opening # is already burned. I'll also fail to claw back my numbers,
 which are mostly still good.

 3. If I miss this success reply, or treat the null as a failure, then I
 will end up re-trying a transaction that has no prayer of working, since
 its opening # is already burned. I'll also fail to claw back my numbers,
 which are mostly still good.

 4. If I miss this success reply, the numbers are all burned on both sides,
 but I don't KNOW that, and though I correctly believe the numbers are all
 burned, I also end up CONTINUING to re-try the message, which fails every
 time, since it was already a success and the numbers are all burned
 already.
 ----------------------------------
 Therefore, the new solutions...
 --------------------------------------------------
 */

// EXCHANGE (into or out of) BASKET (request to server.)
auto OT_API::exchangeBasket(
    otx::context::Server& context,
    const identifier::UnitDefinition& BASKET_INSTRUMENT_DEFINITION_ID,
    const String& BASKET_INFO,
    bool bExchangeInOrOut  // exchanging in == true, out == false.
) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt("Exchanging a basket currency");
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();

    try {
        api_.Wallet().BasketContract(BASKET_INSTRUMENT_DEFINITION_ID);
    } catch (...) {

        return output;
    }

    auto basket{api_.Factory().Basket()};

    OT_ASSERT(false != bool(basket));

    bool validBasket = (0 < BASKET_INFO.GetLength());
    validBasket &= basket->LoadContractFromString(BASKET_INFO);

    if (false == validBasket) { return output; }

    const auto& accountID(basket->GetRequestAccountID());
    auto account = api_.Wallet().Account(accountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return output;
    }

    std::unique_ptr<Ledger> inbox(account.get().LoadInbox(nym));

    if (false == bool(inbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading inbox for "
                                           "account ")(accountID)(".")
            .Flush();

        return output;
    }

    std::unique_ptr<Ledger> outbox(account.get().LoadOutbox(nym));

    if (false == bool(outbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading outbox for "
                                           "account ")(accountID)(".")
            .Flush();

        return output;
    }

    // We need a transaction number just to send this thing. Plus, we need a
    // number for each sub-account to the basket, as well as the basket's
    // main account. That is: 1 + theBasket.Count() + 1 Total of 2, since
    // theBasket.Count() worth of numbers were already added in the calls to
    // OT_API::AddBasketExchangeItem.

    if (context.AvailableNumbers() < 2) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": You don't have enough "
            "transaction numbers to perform the exchange.")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<OTManagedNumber> managed{};
    managed.emplace(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. "
            "Try requesting the server for a new one.")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        managedNumber->Value())(".")
        .Flush();
    transactionNum = managedNumber->Value();
    auto transaction{api_.Factory().Transaction(
        nymID,
        accountID,
        serverID,
        transactionType::exchangeBasket,
        originType::not_applicable,
        transactionNum)};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::exchangeBasket, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& closingNumber = *managed.rbegin();

    if (false == closingNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. "
            "Try requesting the server for a new one.")
            .Flush();

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        closingNumber->Value())(".")
        .Flush();
    basket->SetClosingNum(closingNumber->Value());
    basket->SetExchangingIn(bExchangeInOrOut);
    basket->ReleaseSignatures();
    basket->SignContract(nym, reason);
    basket->SaveContract();
    auto strBasketInfo = String::Factory();
    basket->SaveContractRaw(strBasketInfo);
    item->SetAttachment(strBasketInfo);
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        0, *transaction, context, account.get(), *outbox, reason));

    if (false == bool(balanceItem)) { return output; }

    std::shared_ptr<Item> pbalanceItem{balanceItem.release()};
    transaction->AddItem(pbalanceItem);
    AddHashesToTransaction(*transaction, context, account.get(), reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{api_.Factory().Ledger(nymID, accountID, serverID)};
    ledger->GenerateLedger(accountID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    account.Release();
    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        managed,
        context,
        *message,
        reason);

    return output;
}

auto OT_API::getTransactionNumbers(otx::context::Server& context) const
    -> std::unique_ptr<Message>
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    auto output{api_.Factory().Message()};
    auto requestNum = m_pClient->ProcessUserCommand(
        MessageType::getTransactionNumbers,
        context,
        *output,
        api_.Factory().NymID(),
        api_.Factory().Identifier(),
        reason);

    if (1 > requestNum) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error processing "
            "getTransactionNumber command. Return value: ")(requestNum)(".")
            .Flush();

        return {};
    }

    return output;
}

// Let's pretend you are paying a dollar dividend for Pepsi shares...
// Therefore accountID needs to be a dollar account, and
// SHARES_INSTRUMENT_DEFINITION_ID needs
// to be the Pepsi instrument definition ID. (NOT the dollar instrument
// definition ID...)
auto OT_API::payDividend(
    otx::context::Server& context,
    const Identifier& DIVIDEND_FROM_accountID,  // if dollars paid for pepsi
                                                // shares, then this is the
                                                // issuer's dollars account.
    const identifier::UnitDefinition&
        SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid
                                          // for pepsi shares,
                                          // then this is the
                                          // pepsi shares
                                          // asset type id.
    const String& DIVIDEND_MEMO,          // a message attached to the payout
                                          // request.
    const Amount& AMOUNT_PER_SHARE) const
    -> CommandResult  // number of dollars to be paid out
                      // PER SHARE (multiplied by total
                      // number of shares issued.)
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();
    auto dividendAccount = api_.Wallet().Account(DIVIDEND_FROM_accountID);

    if (false == bool(dividendAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load dividend account.")
            .Flush();

        return output;
    }

    try {
        api_.Wallet().UnitDefinition(SHARES_INSTRUMENT_DEFINITION_ID);
    } catch (...) {

        return output;
    }

    // issuerAccount is not owned by this function
    auto issuerAccount =
        api_.Wallet().IssuerAccount(SHARES_INSTRUMENT_DEFINITION_ID);

    if (false == bool(issuerAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Unable to find issuer account for the shares "
            "instrument definition. Are you sure you're the issuer?")
            .Flush();

        return output;
    }

    if (false == dividendAccount.get().VerifyOwner(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Nym doesn't verify as owner of the source account for "
            "the dividend payout.")
            .Flush();

        return output;
    }

    if (false == issuerAccount.get().VerifyOwner(nym)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Nym doesn't verify as owner of issuer account for the "
            "shares (the shares we're paying the dividend on...).")
            .Flush();

        return output;
    }

    OT_ASSERT(issuerAccount.get().GetBalance() <= 0);

    if (0 == issuerAccount.get().GetBalance()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: There are no shares issued for that instrument "
            "definition.")
            .Flush();
        requestNum = 0;
        status = SendResult::UNNECESSARY;

        return output;
    }

    if (AMOUNT_PER_SHARE <= 0) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: The amount per share must be larger than zero.")
            .Flush();

        return output;
    }

    // If there are 100,000 Pepsi shares, then the Pepsi issuer acct will
    // have -100,000 balance. Therefore we multiply by -1, resulting in
    // 100,000. Then we multiple that by the amount to be paid per share,
    // let's say $5, resulting in a totalCost of $500,000 that must be
    // available in the dollar account (in order to successfully pay this
    // dividend.)
    const Amount totalCost =
        ((-1) * issuerAccount.get().GetBalance()) * AMOUNT_PER_SHARE;

    if (dividendAccount.get().GetBalance() < totalCost) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: There's not enough (")(
            dividendAccount.get().GetBalance())(
            ") in the source account, to cover the total cost of the dividend "
            "(")(totalCost)(").")
            .Flush();

        return output;
    }

    std::set<OTManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. Try requesting the "
            "server for a new one.")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        managedNumber->Value())(".")
        .Flush();
    transactionNum = managedNumber->Value();

    const auto SHARES_ISSUER_accountID =
        api_.Factory().Identifier(issuerAccount.get());
    // Expiration (ignored by server -- it sets its own for its vouchers.)
    const auto VALID_FROM = Clock::now();
    const auto VALID_TO = VALID_FROM + std::chrono::hours(24 * 30 * 6);

    // The server only uses the amount and instrument definition from this
    // cheque when it constructs the actual voucher (to the dividend payee.)
    // And remember there might be a hundred shareholders, so the server
    // would create a hundred vouchers in that case.
    auto theRequestVoucher{
        api_.Factory().Cheque(serverID, SHARES_INSTRUMENT_DEFINITION_ID)};
    const bool bIssueCheque = theRequestVoucher->IssueCheque(
        AMOUNT_PER_SHARE,  // <====== Server needs this (AMOUNT_PER_SHARE.)
        transactionNum,    // server actually ignores this and supplies its
                           // own transaction number for any vouchers.
        VALID_FROM,
        VALID_TO,
        SHARES_ISSUER_accountID,
        nymID,
        DIVIDEND_MEMO,
        nymID);

    if (!bIssueCheque) { return output; }

    /*
        NOTE: The above cheque isn't actually USED for anything, except as a
        vehicle to send additional data to the server. For example, the
       server will need to know the asset type ID for the shares. It gets
       that information from this voucher's instrument definition ID. It
       will also need to know the amount-per-share, which is also on this
       voucher, as its amount. The voucher code already does a similar
       thing, and this code already copied the voucher code since they were
       so similar, so we're just using the same mechanism here. It's
       consistent. On the server side, we'll also need to know the issuer
       account ID for the shares instrument definition, so we set that as
       the "from" account on the request voucher (again, just as a way of
       transmitting it.)
        */

    std::unique_ptr<Ledger> inbox(dividendAccount.get().LoadInbox(nym));

    if (false == bool(inbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading inbox for "
            "account ")(DIVIDEND_FROM_accountID)(".")
            .Flush();

        return output;
    }

    std::unique_ptr<Ledger> outbox(dividendAccount.get().LoadOutbox(nym));

    if (nullptr == outbox) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed loading outbox for "
            "account ")(DIVIDEND_FROM_accountID)(".")
            .Flush();

        return output;
    }

    auto transaction{api_.Factory().Transaction(
        nymID,
        DIVIDEND_FROM_accountID,
        serverID,
        transactionType::payDividend,
        originType::not_applicable,
        transactionNum)};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::payDividend, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    // Notice, while the CHEQUE is for AMOUNT_PER_SHARE, the item's AMOUNT
    // is set to totalCost. The server just needs both of those, so that's
    // how we send them (Similar to the voucher code.)
    item->SetAmount(totalCost);
    item->SetNote(String::Factory("Pay Dividend: "));
    theRequestVoucher->SignContract(nym, reason);
    theRequestVoucher->SaveContract();
    item->SetAttachment(String::Factory(*theRequestVoucher));
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        totalCost * (-1),
        *transaction,
        context,
        dividendAccount.get(),
        *outbox,
        reason));

    if (false == bool(balanceItem)) { return output; }

    // Notice the balance agreement is made for the "total cost of the
    // dividend", which we calculated as the issuer's account balance, times
    // -1, times the amount per share. So for 100,000 shares of Pepsi, at a
    // dividend payout of $2 per share, then $200,000 must be removed from
    // my dollar account, in order to cover it. Therefore I sign a balance
    // agreement for $200,000. The server removes it all at once, and then
    // iterates through a loop, sending vouchers to people. If any fail, or
    // there is any left over, then vouchers are sent back to nym again,
    // containingv the difference.
    // TODO failsafe: We can't just loop, std::int64_t-term, and send a
    // voucher at the end. What if it crashes halfway through the loop? It
    // seems that the dividend payout still needs to be "REGISTERED"
    // somewhere until successfully completed. (And therefore, that this
    // concept must be repeated throughout OT for other transactions, not
    // just this example.) This is already done with Cron, but just thinking
    // about how to best do it for "single action" transactions.

    std::shared_ptr<Item> pbalanceItem{balanceItem.release()};
    transaction->AddItem(pbalanceItem);
    AddHashesToTransaction(
        *transaction, context, dividendAccount.get(), reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{
        api_.Factory().Ledger(nymID, DIVIDEND_FROM_accountID, serverID)};
    ledger->GenerateLedger(
        DIVIDEND_FROM_accountID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        DIVIDEND_FROM_accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    dividendAccount.Release();
    issuerAccount.Release();
    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        managed,
        context,
        *message,
        reason);

    return output;
}

// Request the server to withdraw from an asset account and issue a voucher
// (cashier's cheque)
auto OT_API::withdrawVoucher(
    otx::context::Server& context,
    const Identifier& accountID,
    const identifier::Nym& RECIPIENT_NYM_ID,
    const String& CHEQUE_MEMO,
    const Amount amount) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt("Withdrawing a voucher");
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();
    auto account = api_.Wallet().Account(accountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return output;
    }

    auto contractID = api_.Factory().UnitID();
    auto strContractID = String::Factory();
    contractID = account.get().GetInstrumentDefinitionID();
    contractID->GetString(strContractID);

    const auto withdrawalNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);
    const auto voucherNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);

    if ((!withdrawalNumber->Valid()) || (!voucherNumber->Valid())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Not enough Transaction Numbers were available. "
            "(Suggest requesting the server for more).")
            .Flush();

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        withdrawalNumber->Value())(" for withdrawal.")
        .Flush();

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        voucherNumber->Value())(" for voucher.")
        .Flush();

    const auto strChequeMemo = String::Factory(CHEQUE_MEMO.Get());
    const auto strRecipientNymID = String::Factory(RECIPIENT_NYM_ID);
    // Expiration (ignored by server -- it sets its own for its vouchers.)
    const auto VALID_FROM = Clock::now();
    const auto VALID_TO = VALID_FROM + std::chrono::hours(24 * 30 * 6);
    // The server only uses the memo, amount, and recipient from this cheque
    // when it
    // constructs the actual voucher.
    auto theRequestVoucher{api_.Factory().Cheque(serverID, contractID)};

    OT_ASSERT(false != bool(theRequestVoucher));

    bool bIssueCheque = theRequestVoucher->IssueCheque(
        amount,
        voucherNumber->Value(),
        VALID_FROM,
        VALID_TO,
        accountID,
        nymID,
        strChequeMemo,
        (strRecipientNymID->GetLength() > 2) ? RECIPIENT_NYM_ID
                                             : api_.Factory().NymID().get());
    std::unique_ptr<Ledger> inbox(account.get().LoadInbox(nym));
    std::unique_ptr<Ledger> outbox(account.get().LoadOutbox(nym));

    if (nullptr == inbox) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading inbox for acct ")(
            accountID)(".")
            .Flush();

        return output;
    }

    if (nullptr == outbox) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed loading outbox for acct ")(
            accountID)(".")
            .Flush();

        return output;
    }

    if (!bIssueCheque) { return output; }

    auto transaction{api_.Factory().Transaction(
        nymID,
        accountID,
        serverID,
        transactionType::withdrawal,
        originType::not_applicable,
        withdrawalNumber->Value())};
    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::withdrawVoucher, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    item->SetAmount(amount);
    item->SetNote(String::Factory(" "));
    theRequestVoucher->SignContract(nym, reason);
    theRequestVoucher->SaveContract();
    auto strVoucher = String::Factory(*theRequestVoucher);
    item->SetAttachment(strVoucher);
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    std::unique_ptr<Item> balanceItem(inbox->GenerateBalanceStatement(
        amount * (-1), *transaction, context, account.get(), *outbox, reason));

    if (false == bool(item)) { return output; }

    std::shared_ptr<Item> pbalanceItem{balanceItem.release()};
    transaction->AddItem(pbalanceItem);
    AddHashesToTransaction(*transaction, context, account.get(), reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{api_.Factory().Ledger(nymID, accountID, serverID)};
    ledger->GenerateLedger(accountID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    account.Release();
    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

// DEPOSIT PAYMENT PLAN
//
// The Recipient Creates the Payment Plan using ProposePaymentPlan.
// Then the sender Confirms it using ConfirmPaymentPlan.
// Both of the above steps involve attaching transaction numbers to
// the payment plan and signing copies of it.
// This function here is the final step, where the payment plan
// contract is now being deposited by the customer (who is also
// the sender), in a message to the server.
auto OT_API::depositPaymentPlan(
    otx::context::Server& context,
    const String& THE_PAYMENT_PLAN) const -> CommandResult
{
    auto reason = api_.Factory().PasswordPrompt("Depositing a payment plan");
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();

#ifndef OPENTXS_CHAINCONFIG_PAYMENTPLAN
    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": Failed, Payment Plans are compiler disabled.")
            .Flush();
    return output;
#endif

    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();

    auto plan{api_.Factory().PaymentPlan()};

    OT_ASSERT(false != bool(plan));

    const bool validPlan = plan->LoadContractFromString(THE_PAYMENT_PLAN) &&
                           plan->VerifySignature(nym);

    if (false == validPlan) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load payment plan from string, or verify it.")
            .Flush();

        return output;
    }

    const bool bCancelling = (plan->GetRecipientNymID() == nymID);

    if (bCancelling) {
        if (plan->IsCanceled()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Attempted to cancel (pre-emptively, "
                "before activation) a payment plan "
                "that was already set as cancelled.")
                .Flush();

            return output;
        }

        if (!plan->CancelBeforeActivation(nym, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Attempted to cancel (pre-emptively, "
                "before activation) a payment plan, "
                "but the attempt somehow failed.")
                .Flush();

            return output;
        }
    }

    // The logic for accountID comes about because normally the sender is the
    // one
    // who activates the payment plan. BUT the recipient might ALSO activate it
    // as a way of CANCELLING it. So we check to see if the  recipient user is
    // the same as nymID. If so, then we use the RECIPIENT  account here (it's
    // being cancelled.) Otherwise, we use the sender account ID here as normal
    // (it's being activated.)
    const auto& accountID =
        bCancelling ? plan->GetRecipientAcctID() : plan->GetSenderAcctID();

    auto account = api_.Wallet().Account(accountID);
    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return output;
    }

    const auto openingNumber = plan->GetOpeningNumber(nymID);
    auto transaction{api_.Factory().Transaction(
        nymID,
        accountID,
        serverID,
        transactionType::paymentPlan,
        originType::origin_payment_plan,
        openingNumber)};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::paymentPlan, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    item->SetAttachment(String::Factory(*plan));
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    auto statement = context.Statement(*transaction, reason);

    if (false == bool(statement)) { return output; }

    std::shared_ptr<Item> pstatement{statement.release()};
    transaction->AddItem(pstatement);
    std::unique_ptr<Ledger> inbox(account.get().LoadInbox(nym));
    std::unique_ptr<Ledger> outbox(account.get().LoadOutbox(nym));
    AddHashesToTransaction(*transaction, context, account.get(), reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{api_.Factory().Ledger(nymID, accountID, serverID)};

    OT_ASSERT(false != bool(ledger));

    ledger->GenerateLedger(accountID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

// If a smart contract is already running on a specific server, and the Nym
// in question (NYM_ID) is an authorized agent for that smart contract, then
// he can trigger clauses. All he needs is the transaction ID for the smart
// contract, and the name of the clause.
auto OT_API::triggerClause(
    otx::context::Server& context,
    const TransactionNumber& transactionNumber,
    const String& strClauseName,
    const String& pStrParam) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = transactionNumber;
    status = SendResult::Error;
    reply.reset();
    auto payload = Armored::Factory();

    // Optional string parameter. Available as "param_string" inside the
    // script.
    if (pStrParam.Exists()) { payload->SetString(pStrParam); }

    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::triggerClause,
        payload,
        api_.Factory().Identifier(),
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_lTransactionNum = transactionNumber;
    message->m_strNymID2 = strClauseName;

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

auto OT_API::activateSmartContract(
    otx::context::Server& context,
    const String& THE_SMART_CONTRACT) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt("Activating a smart contract");
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();
    std::unique_ptr<OTSmartContract> contract =
        api_.Factory().SmartContract(serverID);

    if (false == contract->LoadContractFromString(THE_SMART_CONTRACT)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to load smart contract from string: ")(
            THE_SMART_CONTRACT)(".")
            .Flush();

        return output;
    }

    OTAgent* agent{nullptr};
    OTParty* party = contract->FindPartyBasedOnNymAsAuthAgent(nym, &agent);

    if (nullptr == party) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: NYM_ID *IS* a valid "
            "Nym, but that Nym is not the authorizing agent for any of "
            "the parties on this contract. Try calling ConfirmParty() "
            "first.")
            .Flush();

        return output;
    }

    OT_ASSERT(nullptr != agent);

    // BELOW THIS POINT, agent and party are both valid, and no need to
    // clean them up. Note: Usually, payment plan or market offer will load
    // up the Nym and accounts, and verify ownership, etc. But in this case,
    // the Nym who actually activates the smart contract is merely the
    // authorizing agent for a single party, where there may be a dozen
    // parties listed on the actual contract.
    //
    // It would be unreasonable to expect a party to have EVERY nym and
    // EVERY account to the entire contract. As long as the Nym is
    // legitimately the authorizing agent for one of the parties, then we
    // allow him to send the activation message to the server.
    //
    // (Let the server sort out the trouble of loading all the nyms, loading
    // all the accounts, verifying all the instrument definition IDs,
    // verifying all the agent names, making sure there are no stashes
    // already created, ETC ETC.)
    //
    // ONE THING I should verify, is that all of the parties are confirmed.
    // I mean, it's not even worth the bother to send the damn thing until
    // then, right? And the Notary ID.

    if (false == contract->AllPartiesHaveSupposedlyConfirmed()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Not all parties to smart "
            "contract are confirmed. Treating this as a request for "
            "cancelation...).")
            .Flush();

        if (contract->IsCanceled()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: attempted to "
                "cancel (pre-emptively, before activation) a smart "
                "contract that was already set as canceled.")
                .Flush();

            return output;
        }

        if (false == contract->CancelBeforeActivation(nym, reason)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: attempted to "
                "cancel (pre-emptively, before activation) a smart "
                "contract, but the attempt somehow failed.")
                .Flush();

            return output;
        }
    }

    if (serverID != contract->GetNotaryID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed. The server ID passed "
            "in doesn't match the one on the contract itself.")
            .Flush();

        return output;
    }

    if (1 > party->GetAccountCount()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed. The activating Nym "
            "must not only be the authorizing agent for one of the "
            "parties, but must also be the authorized agent for one of "
            "that party's asset accounts. That party must have at least "
            "one asset account for this reason. (See code comment below "
            "this message, in the code).")
            .Flush();

        return output;
    }

    // REQUIREMENT: The ACTIVATOR aka the Originator Nym (the party who
    // activates the smart contract on the server) must have at least one
    // asset account as part of the smart contract, for which the
    // authorizing agent for that party is also the authorized agent for
    // that account. This is in order to make sure that the smart contracts
    // will work with the existing infrastructure, so that functions like
    // "GetOpeningNumber()" and "GetClosingNumber()" will continue to work
    // the way they are expected to, and so that there is always at least
    // ONE closing transaction number supplied for the smart contract (at
    // least ONE finalReceipt will actually have to be signed for once it is
    // deactivated) and so we can know who the owner of that account, and
    // what acct # it will be.
    //
    // This is also a requirement because the existing infrastructure uses
    // the transaction system for activating cron items. It was simply not
    // anticipated in the past that cron items, based on valid transactions,
    // wouldn't also therefore be associated with at least one asset
    // account. In fact, the original difference between transactions (vs
    // normal messages) was that transactions dealt with asset accounts,
    // whereas normal messages did not. (Such as, "checkNym" or
    // "getRequestNumber".)
    //
    // A big piece of this is the BALANCE AGREEMENT. Obviously it wasn't
    // anticipated that "balance agreements" would ever be needed for
    // transactions, yet asset accounts wouldn't be. So the below
    // TRANSACTION STATEMENT, for example, expects an asset account, as does
    // the verification code associated with it. Even transaction statements
    // were originally made for situations where the balance wasn't actually
    // changing, (and so a balance agreement wasn't needed), but where a
    // signature on the transaction numbers was still necessary, since one
    // had to be burned in order to prove that the instrument was valid.
    //
    // Unfortunately, transaction statements still expected an asset account
    // to at least EXIST, since they involved market offers (which do not
    // immediately change the balance) or payment plans (which also do not
    // immediately change the balance.) Yet the account's ID was still at
    // least present in the receipt, and a transaction # was still used.
    //
    // In the case of smart contracts, I will probably someday lift this
    // restriction (that the activating nym must also be the authorized
    // agent on one of the asset accounts on the smart contract). But I will
    // have to go over all the plumbing first and deal with that. In
    // the meantime:
    //
    // --- THE ACTIVATING NYM *MUST* ALSO BE THE AUTHORIZED AGENT FOR AT
    // LEAST ONE ASSET ACCOUNT, FOR THAT PARTY. ---

    auto account = party->GetAccountByAgent(agent->GetName().Get());

    if (nullptr == account) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed. The activating Nym "
            "must not only be the authorizing agent for one of the "
            "parties, but must also be the authorized agent for one of "
            "that party's asset accounts. That party must have at least "
            "one asset account for this reason. (See code comment above "
            "this message, in the code).")
            .Flush();

        return output;
    }

    const auto accountID = api_.Factory().Identifier(account->GetAcctID());

    if (accountID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed. The Account ID is "
                                           "blank for asset acct (")(
            account->GetName())(") for "
                                "party (")(party->GetPartyName())(
            "). Did you confirm "
            "this account, before trying to activate this contract?")
            .Flush();

        return output;
    }

    const auto openingNumber = party->GetOpeningTransNo();
    const auto closingNumber = account->GetClosingTransNo();

    if ((openingNumber <= 0) || (closingNumber <= 0)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed. Opening Transaction # "
            "(")(openingNumber)(") or Closing # (")(closingNumber)(
            ") were invalid for asset acct (")(account->GetName())(
            ") for party (")(party->GetPartyName())(
            "). Did you "
            "confirm this account and party before trying to activate "
            "this contract?")
            .Flush();

        return output;
    }

    if (false == context.VerifyIssuedNumber(openingNumber)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed. Opening Transaction # "
                                           "(")(openingNumber)(
            ") wasn't valid/issued to this Nym, "
            "for asset acct (")(account->GetName())(") for party (")(
            party->GetPartyName())(") on server (")(serverID)(
            "). Did you confirm this account and party before trying "
            "to activate this contract?")
            .Flush();

        return output;
    }

    if (false == context.VerifyIssuedNumber(closingNumber)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed. Closing Transaction # "
                                           "(")(closingNumber)(
            ") wasn't issued to this Nym, for ")("asset acct (")(
            account->GetName())(") for party (")(party->GetPartyName())(
            "). Did you confirm this account and "
            "party before trying to activate this contract?")
            .Flush();

        return output;
    }

    // These numbers (the opening and closing transaction #s) were already
    // available on the smart contract -- specifically an opening number is
    // stored for each party, taken from the authorizing agent for that
    // party, and a closing number is stored for each asset account, taken
    // from the authorized agent for that asset account.
    //
    // Now we set the appropriate opening/closing number onto the smart
    // contract, in the way of the existing CronItem system, so that it will
    // work properly within that infrastructure. (This is what makes the
    // activating Nym slightly different / more than the other authorizing
    // agent nyms for each party to the contract. Not only does it have
    // opening/closing numbers on its party like all the others, but its
    // numbers are also those used for the cron item itself.)
    contract->PrepareToActivate(openingNumber, closingNumber, nymID, accountID);
    // This call changes the contract slightly, so it must be re-signed (in
    // order to save changes.)
    contract->ReleaseSignatures();

    if (false == agent->SignContract(*contract, reason)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed re-signing contract, "
                                           "after calling PrepareToActivate().")
            .Flush();

        return output;
    }

    contract->SaveContract();
    auto transaction{api_.Factory().Transaction(
        nymID,
        accountID,
        serverID,
        transactionType::smartContract,
        originType::not_applicable,
        contract->GetTransactionNum())};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::smartContract, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    item->SetAttachment(String::Factory(*contract));
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    auto statement = context.Statement(*transaction, reason);

    if (false == bool(statement)) { return output; }

    std::shared_ptr<Item> pstatement{statement.release()};
    transaction->AddItem(pstatement);
    AddHashesToTransaction(*transaction, context, accountID, reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{api_.Factory().Ledger(nymID, accountID, serverID)};
    ledger->GenerateLedger(accountID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();

    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        accountID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

// Done: make a new transaction (and item) type for smart contract. (for a
// cron item.)

// Done: Add an API call that allows the coder to harvest transaction
// numbers from Cron Items. This will be used on the client side, I believe,
// and not server side (not in smart contracts.)
//

// Done: Make a copy of CancelCronItem(), and use it to make an EXECUTE
// CLAUSE message for Nyms to trigger existing smart contracts!!!!!!!!!
// WAIT: Except it DOESN'T have to be a transaction! Because as long as the
// Nym is a valid party, and the action occurs, it will ALREADY drop
// receipts into the appropriate boxes! But wait... How do we know that the
// Nym REALLY triggered that clause, if there's not a transaction # burned?
// It doesn't matter: the purpose of transaction #s is to make sure that
// CHANGES in BALANCE are signed off on, by leaving the # open, and putting
// a receipt
// in the inbox. But if Nym really is a party to this smart contract (which
// will be verified in any case), then he DOES have transaction #s open
// already, and if any balances change, receipts will be dropped into the
// appropriate inboxes already containing those transaction #s. (As well as
// containing his original signed copy of the cron item, AND as well as
// containing the latest message, with his signature and request number on
// it, which triggered the clause to be called.)  Furthermore, if the clause
// is triggered multiple times, then there must be multiple receipts, and
// each one should feature a NEW instance of the triggering, WITH a valid
// signature on it, and WITH a NEW REQUEST NUMBER ON IT. Without this, none
// of the balances could change anyway. Therefore I conclude that the
// "trigger a clause" message does NOT have to be a transaction, but only
// needs to be a message  :-)
//
// Done: the "trigger clause" message!

// DONE: Change this into a TRANSACTION!
// (So there can be a transaction statement, since canceling a cron
// item removes a transaction number from your issued list.)
///-------------------------------------------------------
/// CANCEL A SPECIFIC OFFER (THAT SAME NYM PLACED PREVIOUSLY ON SAME
/// SERVER.) By transaction number as key.
auto OT_API::cancelCronItem(
    otx::context::Server& context,
    const Identifier& ASSET_ACCOUNT_ID,
    const TransactionNumber& lTransactionNum) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason =
        api_.Factory().PasswordPrompt("Cancelling a recurring transaction");
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();

    if (context.AvailableNumbers() < 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": At least 1 Transaction Number is necessary to cancel any "
            "cron item. Try requesting the server for more numbers (you "
            "are low).")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<OTManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& managedNumber = *managed.rbegin();

    if (false == managedNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. Suggest "
            "requesting the server for one.")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        managedNumber->Value())(".")
        .Flush();
    transactionNum = managedNumber->Value();
    auto transaction{api_.Factory().Transaction(
        nymID,
        ASSET_ACCOUNT_ID,
        serverID,
        transactionType::cancelCronItem,
        originType::not_applicable,
        transactionNum)};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::cancelCronItem, api_.Factory().Identifier())};

    if (false == bool(item)) { return output; }

    item->SetReferenceToNum(transactionNum);
    transaction->SetReferenceToNum(transactionNum);
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    auto statement = context.Statement(*transaction, reason);

    if (false == bool(statement)) { return output; }

    std::shared_ptr<Item> pstatement{statement.release()};
    transaction->AddItem(pstatement);
    AddHashesToTransaction(*transaction, context, ASSET_ACCOUNT_ID, reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();

    // set up the ledger
    auto ledger{api_.Factory().Ledger(nymID, ASSET_ACCOUNT_ID, serverID)};
    ledger->GenerateLedger(ASSET_ACCOUNT_ID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();

    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        ASSET_ACCOUNT_ID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        managed,
        context,
        *message,
        reason);

    return output;
}

// Create an Offer object and add it to one of the server's Market objects.
// This will also create a Trade object and add it to the server's Cron
// object. (The Trade provides the payment authorization for the Offer, as
// well as the rules for processing and expiring it.)
auto OT_API::issueMarketOffer(
    otx::context::Server& context,
    const Identifier& ASSET_ACCOUNT_ID,
    const Identifier& CURRENCY_ACCOUNT_ID,
    const std::int64_t& MARKET_SCALE,       // Defaults to minimum of 1. Market
                                            // granularity.
    const std::int64_t& MINIMUM_INCREMENT,  // This will be multiplied by
                                            // the Scale. Min 1.
    const std::int64_t& TOTAL_ASSETS_ON_OFFER,  // Total assets available
                                                // for sale or purchase.
                                                // Will be multiplied by
                                                // minimum increment.
    const Amount PRICE_LIMIT,                   // Per Minimum Increment...
    const bool bBuyingOrSelling,  //  BUYING == false, SELLING == true.
    const std::chrono::seconds tLifespanInSeconds,  // 86400 == 1 day.
    const char STOP_SIGN,  // For stop orders, set to '<' or '>'
    const Amount ACTIVATION_PRICE) const
    -> CommandResult  // For stop orders, this is
                      // threshhold price.
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt("Issuing a market offer");
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    const auto& serverID = context.Notary();
    auto assetAccount = api_.Wallet().Account(ASSET_ACCOUNT_ID);

    if (false == bool(assetAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load asset account.")
            .Flush();

        return output;
    }

    auto currencyAccount = api_.Wallet().Account(CURRENCY_ACCOUNT_ID);

    if (false == bool(currencyAccount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load currency account.")
            .Flush();

        return output;
    }

    const auto& assetContractID =
        assetAccount.get().GetInstrumentDefinitionID();
    const auto& currencyContractID =
        currencyAccount.get().GetInstrumentDefinitionID();

    if (assetContractID == currencyContractID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": The asset account and currency account cannot "
            "have the same instrument definition ID. (You can't, for "
            "example, trade dollars against other dollars. Why "
            "bother trading them in the first place)?")
            .Flush();

        return output;
    }

    if (context.AvailableNumbers() < 3) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": At least 3 Transaction Numbers are necessary to issue a "
            "market offer. Try requesting the server for more (you are "
            "low).")
            .Flush();
        status = SendResult::TRANSACTION_NUMBERS;

        return output;
    }

    std::set<OTManagedNumber> managed{};
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& openingNumber = *managed.rbegin();

    if (false == openingNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. Suggest "
            "requesting the server for one.")
            .Flush();

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        openingNumber->Value())(".")
        .Flush();
    transactionNum = openingNumber->Value();
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& assetClosingNumber = *managed.rbegin();

    if (false == openingNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No assetClosingNumber numbers were available. Suggest "
            "requesting the server for one.")
            .Flush();

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        assetClosingNumber->Value())(".")
        .Flush();
    managed.insert(
        context.NextTransactionNumber(MessageType::notarizeTransaction));
    auto& currencyClosingNumber = *managed.rbegin();

    if (false == currencyClosingNumber->Valid()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No transaction numbers were available. Suggest "
            "requesting the server for one.")
            .Flush();

        return output;
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
        currencyClosingNumber->Value())(".")
        .Flush();

    // defaults to RIGHT NOW aka OT_API_GetTime() if set to 0 anyway.
    const auto VALID_FROM = GetTime();
    // defaults to 24 hours (a "Day Order") aka OT_API_GetTime() + 86,400
    const auto VALID_TO =
        VALID_FROM + std::chrono::seconds{
                         (std::chrono::seconds{0} == tLifespanInSeconds)
                             ? std::chrono::hours{24}
                             : tLifespanInSeconds};
    std::int64_t lTotalAssetsOnOffer = 1, lMinimumIncrement = 1,
                 lPriceLimit = 0,  // your price limit, per scale of assets.
        lMarketScale = 1, lActivationPrice = 0;

    if (TOTAL_ASSETS_ON_OFFER > 0) {
        lTotalAssetsOnOffer =
            TOTAL_ASSETS_ON_OFFER;  // otherwise, defaults to 1.
    }

    if (MARKET_SCALE > 0) {
        lMarketScale = MARKET_SCALE;  // otherwise, defaults to 1.
    }

    if (MINIMUM_INCREMENT > 0) {
        lMinimumIncrement = MINIMUM_INCREMENT;  // otherwise, defaults to 1.
    }

    if (PRICE_LIMIT > 0) {
        lPriceLimit = PRICE_LIMIT;  // otherwise, defaults to 0. (0 Being a
                                    // market order.)
    }

    char cStopSign = 0;

    if ((ACTIVATION_PRICE > 0) && (('<' == STOP_SIGN) || ('>' == STOP_SIGN))) {
        cStopSign = STOP_SIGN;
        lActivationPrice = ACTIVATION_PRICE;
    }

    lMinimumIncrement *= lMarketScale;  // minimum increment is PER SCALE.
    auto strOfferType = String::Factory("market order");

    if (lPriceLimit > 0) { strOfferType = String::Factory("limit order"); }

    if (0 != cStopSign) {
        if (lPriceLimit > 0) {
            strOfferType->Format(
                "stop limit order, at threshhold: %c%" PRId64,
                cStopSign,
                lActivationPrice);
        } else {
            strOfferType->Format(
                "stop order, at threshhold: %c%" PRId64,
                cStopSign,
                lActivationPrice);
        }
    }

    auto strPrice = String::Factory();

    if (lPriceLimit > 0) {
        strPrice->Format("Price: %" PRId64 "\n", lPriceLimit);
    }

    auto offer{api_.Factory().Offer(
        serverID, assetContractID, currencyContractID, lMarketScale)};

    OT_ASSERT(false != bool(offer));

    auto trade{api_.Factory().Trade(
        serverID,
        assetContractID,
        ASSET_ACCOUNT_ID,
        nymID,
        currencyContractID,
        CURRENCY_ACCOUNT_ID)};

    OT_ASSERT(false != bool(trade));

    bool bCreateOffer = offer->MakeOffer(
        bBuyingOrSelling,        // True == SELLING, False == BUYING
        lPriceLimit,             // Per Minimum Increment...
        lTotalAssetsOnOffer,     // Total assets available for sale or
                                 // purchase.
        lMinimumIncrement,       // The minimum increment that must be bought or
                                 // sold for each transaction
        openingNumber->Value(),  // Transaction number matches on
                                 // transaction, item, offer, and trade.
        VALID_FROM,              // defaults to RIGHT NOW
        VALID_TO);               // defaults to 24 hours (a "Day Order")

    if (false == bCreateOffer) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    bCreateOffer = offer->SignContract(nym, reason);

    if (false == bCreateOffer) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    bCreateOffer = offer->SaveContract();

    if (false == bCreateOffer) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    bool bIssueTrade = trade->IssueTrade(*offer, cStopSign, lActivationPrice);

    if (false == bIssueTrade) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    bIssueTrade = trade->SignContract(nym, reason);

    if (false == bIssueTrade) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    bIssueTrade = trade->SaveContract();

    if (false == bIssueTrade) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create offer or issue trade.")
            .Flush();

        return output;
    }

    // This is new: the closing transaction number is now used for CLOSING
    // recurring cron items, like market offers and payment plans. It's also
    // useful for baskets. Since this is a market offer, it needs a closing
    // number (for later cancellation or expiration.) assetClosingNumber=0,
    // currencyClosingNumber=0;
    trade->AddClosingTransactionNo(assetClosingNumber->Value());
    trade->AddClosingTransactionNo(currencyClosingNumber->Value());
    LogOutput(OT_METHOD)(__FUNCTION__)(": Placing market offer ")(
        openingNumber->Value())(", type: ")(
        bBuyingOrSelling ? "selling" : "buying")(", ")(strOfferType)(", ")(
        strPrice)(".")(" Assets for sale/purchase: ")(lTotalAssetsOnOffer)(
        ". In minimum increments of: ")(lMinimumIncrement)(
        ". At market of scale: ")(lMarketScale)(". Valid From: ")(VALID_FROM)(
        ". To: ")(VALID_TO)
        .Flush();
    auto transaction{api_.Factory().Transaction(
        nymID,
        ASSET_ACCOUNT_ID,
        serverID,
        transactionType::marketOffer,
        originType::origin_market_offer,
        openingNumber->Value())};

    if (false == bool(transaction)) { return output; }

    auto item{api_.Factory().Item(
        *transaction, itemType::marketOffer, CURRENCY_ACCOUNT_ID)};

    if (false == bool(item)) { return output; }

    auto tradeAttachment = String::Factory();
    trade->SaveContractRaw(tradeAttachment);
    item->SetAttachment(tradeAttachment);
    item->SignContract(nym, reason);
    item->SaveContract();
    std::shared_ptr<Item> pitem{item.release()};
    transaction->AddItem(pitem);
    auto statement = context.Statement(*transaction, reason);

    if (false == bool(statement)) { return output; }

    std::shared_ptr<Item> pstatement{statement.release()};
    transaction->AddItem(pstatement);
    AddHashesToTransaction(*transaction, context, ASSET_ACCOUNT_ID, reason);
    transaction->SignContract(nym, reason);
    transaction->SaveContract();
    auto ledger{api_.Factory().Ledger(nymID, ASSET_ACCOUNT_ID, serverID)};

    OT_ASSERT(false != bool(ledger));

    ledger->GenerateLedger(ASSET_ACCOUNT_ID, serverID, ledgerType::message);
    std::shared_ptr<OTTransaction> ptransaction{transaction.release()};
    ledger->AddTransaction(ptransaction);
    ledger->SignContract(nym, reason);
    ledger->SaveContract();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::notarizeTransaction,
        Armored::Factory(String::Factory(*ledger)),
        ASSET_ACCOUNT_ID,
        requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    assetAccount.Release();
    currencyAccount.Release();
    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        managed,
        context,
        *message,
        reason);

    return output;
}

/// GET MARKET LIST
///
/// Connect to a specific server, as a specific Nym, and request the
/// list of markets. (Flush the buffer before calling this. Then after
/// you make this call, wait 50 ms and then pop the buffer and check the
/// server reply for success. From there you can either read the reply
/// data directly out of the reply message, or you can load it from
/// storage (OT will probably auto-store the reply to storage, for your
/// convenience.)
auto OT_API::getMarketList(otx::context::Server& context) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] =
        context.InitializeServerCommand(MessageType::getMarketList, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

/// GET ALL THE OFFERS ON A SPECIFIC MARKET
///
/// A specific Nym is requesting the Server to send a list of the offers
/// on a specific Market ID-- the bid/ask, and prices/amounts,
/// basically--(up to lDepth or server Max)
auto OT_API::getMarketOffers(
    otx::context::Server& context,
    const Identifier& MARKET_ID,
    const std::int64_t& lDepth) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getMarketOffers, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_strNymID2 = String::Factory(MARKET_ID);
    message->m_lDepth = lDepth;

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

///-------------------------------------------------------
/// GET RECENT TRADES FOR A SPECIFIC MARKET ID
///
/// Most likely, ticker data will be made available through a separate
/// ZMQ instance, which will use the publisher/subscriber model to
/// distribute ticker data. From there, those privileged subscribers can
/// distribute it via RSS, store it for future analysis, display charts,
/// etc.
///
/// (So this function is not here to usurp that purpose.)
auto OT_API::getMarketRecentTrades(
    otx::context::Server& context,
    const Identifier& MARKET_ID) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getMarketRecentTrades, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_strNymID2 = String::Factory(MARKET_ID);

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

///-------------------------------------------------------
/// GET ALL THE ACTIVE (in Cron) MARKET OFFERS FOR A SPECIFIC NYM. (ON A
/// SPECIFIC SERVER, OBVIOUSLY.) Remember to use Flush/Call/Wait/Pop to
/// check the server reply for success or fail. Hmm for size reasons,
/// this really will have to return a list of transaction #s, and then I
/// request them one-by-one after that...
auto OT_API::getNymMarketOffers(otx::context::Server& context) const
    -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::getNymMarketOffers, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

// Sends a list of instrument definition ids to the server, which
// replies with a list of the actual receipts with the issuer's
// signature on them, from when the currency was issued.
//
// So you can tell for sure whether or not they are actually issued
// on that server.
//
// Map input: key is instrument definition ID, and value is blank
// (server reply puts issuer's receipts in that spot.)
auto OT_API::queryInstrumentDefinitions(
    otx::context::Server& context,
    const Armored& ENCODED_MAP) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::queryInstrumentDefinitions, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_ascPayload = ENCODED_MAP;

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

auto OT_API::deleteAssetAccount(
    otx::context::Server& context,
    const Identifier& ACCOUNT_ID) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();

    {
        auto account = api_.Wallet().Account(ACCOUNT_ID);

        if (false == bool(account)) { return output; }
    }

    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::unregisterAccount, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_strAcctID = String::Factory(ACCOUNT_ID);

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

auto OT_API::usageCredits(
    otx::context::Server& context,
    const identifier::Nym& NYM_ID_CHECK,
    std::int64_t lAdjustment) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto [newRequestNumber, message] = context.InitializeServerCommand(
        MessageType::usageCredits, NYM_ID_CHECK, requestNum);
    requestNum = newRequestNumber;

    if (false == bool(message)) { return output; }

    message->m_lDepth = lAdjustment;

    if (false == context.FinalizeServerCommand(*message, reason)) {

        return output;
    }

    result = context.SendMessage(
        dynamic_cast<const api::client::internal::Manager&>(api_),
        {},
        context,
        *message,
        reason);

    return output;
}

auto OT_API::unregisterNym(otx::context::Server& context) const -> CommandResult
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    CommandResult output{};
    auto& [requestNum, transactionNum, result] = output;
    auto& [status, reply] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::Error;
    reply.reset();
    auto message{api_.Factory().Message()};
    requestNum = m_pClient->ProcessUserCommand(
        MessageType::unregisterNym,
        context,
        *message,
        api_.Factory().NymID(),
        api_.Factory().Identifier(),
        reason);

    if (0 < requestNum) {
        result = context.SendMessage(
            dynamic_cast<const api::client::internal::Manager&>(api_),
            {},
            context,
            *message,
            reason);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error in "
                                           "m_pClient->ProcessUserCommand().")
            .Flush();
    }

    return output;
}

auto OT_API::CreateProcessInbox(
    const Identifier& accountID,
    otx::context::Server& context,
    [[maybe_unused]] Ledger& inbox) const -> OT_API::ProcessInboxOnly
{
    const std::string account = accountID.str();
    const auto& serverID = context.Notary();
    const auto& nym = *context.Nym();
    const auto& nymID = nym.ID();
    OT_API::ProcessInboxOnly output{};
    auto& [processInbox, number] = output;
    processInbox.reset(
        api_.Factory()
            .Ledger(nymID, accountID, serverID, ledgerType::message)
            .release());

    if (false == bool(processInbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error generating process inbox ledger for "
            "account ")(account)
            .Flush();

        return {};
    }

    auto transaction =
        get_or_create_process_inbox(accountID, context, *processInbox);

    if (nullptr == transaction) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to create processInbox transaction.")
            .Flush();

        return {};
    }

    number = transaction->GetTransactionNum();

    return output;
}

auto OT_API::IncludeResponse(
    const Identifier& accountID,
    const bool accept,
    otx::context::Server& context,
    OTTransaction& source,
    Ledger& response) const -> bool
{
    rLock lock(
        lock_callback_({context.Nym()->ID().str(), context.Notary().str()}));
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    const auto& serverID = context.Notary();
    const auto type = source.GetType();
    const auto inRefTo = String::Factory(source);

    switch (type) {
        case transactionType::pending:
        case transactionType::chequeReceipt:
        case transactionType::voucherReceipt:
        case transactionType::transferReceipt:
        case transactionType::marketReceipt:
        case transactionType::paymentReceipt:
        case transactionType::finalReceipt:
        case transactionType::basketReceipt: {
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong transaction type: ")(
                source.GetTypeString())
                .Flush();

            return false;
        }
    }

    auto& nym = *context.Nym();
    auto& serverNym = context.RemoteNym();
    const bool validSource = source.VerifyAccount(serverNym);

    if (!validSource) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to verify source transaction.")
            .Flush();

        return false;
    }

    auto processInbox =
        get_or_create_process_inbox(accountID, context, response);

    if (nullptr == processInbox) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to find or create processInbox.")
            .Flush();

        return false;
    }

    auto note = String::Factory();
    const auto originNumber = get_origin(serverID, source, note);
    const bool acceptItemAdded = add_accept_item(
        response_type(type, accept),
        originNumber,
        source.GetTransactionNum(),
        note,
        nym,
        source.GetReceiptAmount(reason),
        inRefTo,
        *processInbox);

    if (false == acceptItemAdded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to add response item to process inbox "
            "transaction.")
            .Flush();

        return false;
    }

    return true;
}

auto OT_API::FinalizeProcessInbox(
    const Identifier& accountID,
    otx::context::Server& context,
    Ledger& response,
    Ledger& inbox,
    Ledger& outbox,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT(ledgerType::inbox == inbox.GetType());
    OT_ASSERT(ledgerType::outbox == outbox.GetType());

    class Cleanup
    {
    public:
        Cleanup(OTTransaction& processInbox, otx::context::Server& context)
            : context_(context)
            , success_(false)
            , number_(processInbox.GetTransactionNum())
        {
        }
        Cleanup() = delete;

        void SetSuccess(const bool success) { success_ = success; }

        ~Cleanup()
        {
            if ((0 != number_) && (false == success_)) {
                context_.RecoverAvailableNumber(number_);
            }
        }

    private:
        otx::context::Server& context_;
        bool success_{false};
        TransactionNumber number_{0};
    };

    auto nym = context.Nym();
    auto processInbox = response.GetTransaction(transactionType::processInbox);

    if (false == bool(processInbox)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Response ledger does not contain processInbox.")
            .Flush();

        return false;
    }

    auto balanceStatement(processInbox->GetItem(itemType::balanceStatement));

    if (true == bool(balanceStatement)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This response has already been finalized.")
            .Flush();

        return false;
    }

    // Below this point, any failure will result in the transaction
    // number being recovered
    Cleanup cleanup(*processInbox, context);
    auto account = api_.Wallet().Account(accountID);

    if (false == bool(account)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load account.").Flush();

        return false;
    }

    bool allFound{true};
    Amount totalAccepted{0};
    std::set<TransactionNumber> issuedClosing{};
    std::set<TransactionNumber> inboxRemoving{};

    for (auto& acceptItem : processInbox->GetItemList()) {
        OT_ASSERT(acceptItem);

        const auto inboxNumber = acceptItem->GetReferenceToNum();
        auto inboxItem = inbox.GetTransaction(inboxNumber);

        if (nullptr == inboxItem) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Expected receipt ")(
                inboxNumber)(" not found.")
                .Flush();

            return false;
        }

        const auto originalNumber = inboxItem->GetReferenceToNum();
        const auto type = acceptItem->GetType();
        auto typeDescription = String::Factory();
        acceptItem->GetTypeString(typeDescription);

        switch (type) {
            case itemType::acceptPending:
            case itemType::acceptItemReceipt: {
                allFound &= find_standard(
                    context,
                    *acceptItem,
                    originalNumber,
                    *inboxItem,
                    inbox,
                    totalAccepted,
                    issuedClosing);
            } break;
            case itemType::acceptCronReceipt:
            case itemType::acceptFinalReceipt:
            case itemType::acceptBasketReceipt: {
                allFound &= find_cron(
                    context,
                    *acceptItem,
                    *processInbox,
                    *inboxItem,
                    inbox,
                    totalAccepted,
                    issuedClosing);
            } break;
            default: {
                auto typeName = String::Factory();
                acceptItem->GetTypeString(typeName);
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected item type: ")(
                    typeName)
                    .Flush();

                return false;
            }
        }

        inboxRemoving.insert(inboxNumber);
    }

    if (false == allFound) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Transactions in processInbox "
                                           "message do not match actual inbox.")
            .Flush();

        return false;
    }

    for (const auto& remove : inboxRemoving) {
        if (!inbox.RemoveTransaction(remove)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed removing receipt from temporary inbox: ")(remove)(".")
                .Flush();

            return false;
        }
    }

    balanceStatement.reset(inbox
                               .GenerateBalanceStatement(
                                   totalAccepted,
                                   *processInbox,
                                   context,
                                   account.get(),
                                   outbox,
                                   issuedClosing,
                                   reason)
                               .release());

    if (false == bool(balanceStatement)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to generate balance statement.")
            .Flush();

        return false;
    }

    bool output = true;
    processInbox->AddItem(balanceStatement);
    processInbox->ReleaseSignatures();
    AddHashesToTransaction(*processInbox.get(), context, account.get(), reason);

    output &= (output && processInbox->SignContract(*nym, reason));
    output &= (output && processInbox->SaveContract());

    if (output) {
        response.ReleaseSignatures();
        output &= (output && response.SignContract(*nym, reason));
        output &= (output && response.SaveContract());
    }

    if (output) { cleanup.SetSuccess(true); }

    return output;
}

auto OT_API::find_cron(
    const otx::context::Server& context,
    const Item& item,
    OTTransaction& processInbox,
    OTTransaction& serverTransaction,
    Ledger& inbox,
    Amount& amount,
    std::set<TransactionNumber>& closing) const -> bool
{
    const auto type = item.GetType();

    switch (type) {
        case itemType::acceptCronReceipt: {
        } break;
        case itemType::acceptFinalReceipt: {
            const auto openingNumber = serverTransaction.GetReferenceToNum();
            const auto inboxCount = static_cast<std::size_t>(
                inbox.GetTransactionCountInRefTo(openingNumber));
            std::set<TransactionNumber> referenceNumbers;

            for (const auto& acceptItem : processInbox.GetItemList()) {
                OT_ASSERT(nullptr != acceptItem);

                const auto itemNumber = acceptItem->GetReferenceToNum();
                auto transaction = inbox.GetTransaction(itemNumber);

                if (nullptr == transaction) { continue; }

                if (transaction->GetReferenceToNum() == openingNumber) {
                    referenceNumbers.insert(itemNumber);
                }
            }

            if (referenceNumbers.size() != inboxCount) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": In order to process a finalReceipt, "
                    "all related "
                    "receipts must also be processed.")
                    .Flush();

                return false;
            }
        }
            [[fallthrough]];
        case itemType::acceptBasketReceipt: {
            const auto closingNumber = serverTransaction.GetClosingNum();
            const bool verified = context.VerifyIssuedNumber(closingNumber);

            if (verified) {
                closing.insert(closingNumber);
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Trying to remove closing number (")(closingNumber)(
                    ") that already wasn't on my issued list.")
                    .Flush();

                return false;
            }
        } break;
        default: {
            auto typeName = String::Factory();
            item.GetTypeString(typeName);
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected item type: ")(
                typeName)
                .Flush();

            return false;
        }
    }

    return true;
}

auto OT_API::find_standard(
    const otx::context::Server& context,
    const Item& item,
    const TransactionNumber number,
    OTTransaction& serverTransaction,
    Ledger& inbox,
    Amount& amount,
    std::set<TransactionNumber>& closing) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    const auto& notaryID = context.Notary();
    const auto referenceNum = item.GetReferenceToNum();
    const auto type = item.GetType();

    switch (type) {
        case itemType::acceptPending: {
            amount += serverTransaction.GetReceiptAmount(reason);
        } break;
        case itemType::acceptItemReceipt: {
            auto reference = String::Factory();
            serverTransaction.GetReferenceString(reference);
            auto original{api_.Factory().Item(reference, notaryID, number)};

            if (false == bool(original)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unable to load original item while "
                    "accepting "
                    "item receipt: ")(referenceNum)(".")
                    .Flush();

                return false;
            }

            TransactionNumber issuedNumber{0};
            const auto originalType = original->GetType();

            switch (originalType) {
                case itemType::depositCheque: {
                    auto attachment = String::Factory();
                    original->GetAttachment(attachment);
                    auto cheque{api_.Factory().Cheque()};

                    OT_ASSERT(false != bool(cheque));

                    if (3 > attachment->GetLength()) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Invalid attachment (cheque).")
                            .Flush();

                        return false;
                    }

                    if (false == cheque->LoadContractFromString(attachment)) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": Unable to instantiate cheque.")
                            .Flush();

                        return false;
                    }

                    issuedNumber = cheque->GetTransactionNum();
                } break;
                case itemType::acceptPending: {
                    issuedNumber = original->GetNumberOfOrigin();
                } break;
                default: {
                    auto typeName = String::Factory();
                    original->GetTypeString(typeName);
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unexpected original item type: ")(typeName)
                        .Flush();

                    return false;
                }
            }

            const bool verified = context.VerifyIssuedNumber(issuedNumber);

            if (verified) {
                closing.insert(issuedNumber);
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Trying to remove number (")(issuedNumber)(
                    ") that already wasn't on my "
                    "issued list.")
                    .Flush();

                return false;
            }
        } break;
        default: {
            auto typeName = String::Factory();
            item.GetTypeString(typeName);
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected item type: ")(
                typeName)
                .Flush();

            return false;
        }
    }

    return true;
}

auto OT_API::add_accept_item(
    const itemType type,
    const TransactionNumber originNumber,
    const TransactionNumber referenceNumber,
    const String& note,
    const identity::Nym& nym,
    const Amount amount,
    const String& inRefTo,
    OTTransaction& processInbox) const -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    std::shared_ptr<Item> acceptItem{
        api_.Factory().Item(processInbox, type, api_.Factory().Identifier())};

    if (false == bool(acceptItem)) { return false; }

    acceptItem->SetNumberOfOrigin(originNumber);
    acceptItem->SetReferenceToNum(referenceNumber);
    acceptItem->SetAmount(amount);

    if (note.Exists()) { acceptItem->SetNote(note); }

    if (inRefTo.Exists()) { acceptItem->SetAttachment(inRefTo); }

    bool output = true;
    output &= acceptItem->SignContract(nym, reason);
    output &= (output && acceptItem->SaveContract());

    if (output) { processInbox.AddItem(acceptItem); }

    return output;
}

auto OT_API::get_or_create_process_inbox(
    const Identifier& accountID,
    otx::context::Server& context,
    Ledger& response) const -> OTTransaction*
{
    const auto& nymID = context.Nym()->ID();
    const auto& serverID = context.Notary();
    auto processInbox = response.GetTransaction(transactionType::processInbox);

    if (nullptr == processInbox) {
        const auto number =
            context.NextTransactionNumber(MessageType::processInbox);

        if (false == number->Valid()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(nymID)(
                " is all out of transaction numbers.")
                .Flush();

            return {};
        }

        LogVerbose(OT_METHOD)(__FUNCTION__)(": Allocated transaction number ")(
            number->Value())(".")
            .Flush();

        auto newProcessInbox{api_.Factory().Transaction(
            nymID,
            accountID,
            serverID,
            transactionType::processInbox,
            originType::not_applicable,
            number->Value())};

        if (false == bool(newProcessInbox)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error generating processInbox transaction "
                "for AcctID: ")(accountID)(".")
                .Flush();

            return {};
        }

        // Above this line, the transaction number will be recovered
        // automatically
        number->SetSuccess(true);
        processInbox.reset(newProcessInbox.release());
        response.AddTransaction(processInbox);
    }

    return processInbox.get();
}

auto OT_API::response_type(const transactionType sourceType, const bool success)
    const -> itemType
{
    switch (sourceType) {
        case transactionType::pending: {
            if (success) {
                return itemType::acceptPending;
            } else {
                return itemType::rejectPending;
            }
        }
        case transactionType::marketReceipt:
        case transactionType::paymentReceipt: {
            if (success) {
                return itemType::acceptCronReceipt;
            } else {
                return itemType::disputeCronReceipt;
            }
        }
        case transactionType::chequeReceipt:
        case transactionType::voucherReceipt:
        case transactionType::transferReceipt: {
            if (success) {
                return itemType::acceptItemReceipt;
            } else {
                return itemType::disputeItemReceipt;
            }
        }
        case transactionType::finalReceipt: {
            if (success) {
                return itemType::acceptFinalReceipt;
            } else {
                return itemType::disputeFinalReceipt;
            }
        }
        case transactionType::basketReceipt: {
            if (success) {
                return itemType::acceptBasketReceipt;
            } else {
                return itemType::disputeBasketReceipt;
            }
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected transaction type.")(".")
                .Flush();

            return itemType::error_state;
        }
    }
}

auto OT_API::get_origin(
    const identifier::Server& notaryID,
    const OTTransaction& source,
    String& note) const -> TransactionNumber
{
    const auto sourceType = source.GetType();
    TransactionNumber originNumber{0};

    switch (sourceType) {
        case transactionType::marketReceipt:
        case transactionType::paymentReceipt:
        case transactionType::finalReceipt:
        case transactionType::basketReceipt: {
            originNumber = source.GetReferenceToNum();
        } break;
        case transactionType::transferReceipt:
        case transactionType::pending:
        case transactionType::chequeReceipt:
        case transactionType::voucherReceipt: {
            auto reference = String::Factory();
            source.GetReferenceString(reference);

            if (false == reference->Exists()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": No reference string found on transaction.")
                    .Flush();

                return {};
            }

            auto original{api_.Factory().Item(
                reference, notaryID, source.GetReferenceToNum())};

            if (false == bool(original)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed loading transaction item from "
                    "string.")
                    .Flush();

                return {};
            }

            if (Item::request != original->GetStatus()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Wrong status on original item.")
                    .Flush();

                return {};
            }

            const auto originalType = original->GetType();

            switch (originalType) {
                case itemType::acceptPending:
                case itemType::depositCheque: {
                    break;
                }
                case itemType::transfer: {
                    original->GetNote(note);
                } break;
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Wrong type on original item.")
                        .Flush();

                    return {};
                }
            }

            originNumber = original->GetNumberOfOrigin();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected transaction type in: ")(source.GetTypeString())(
                ".")
                .Flush();

            return {};
        }
    }

    return originNumber;
}

OT_API::~OT_API()
{
    m_pClient.reset();
    Cleanup();
}
}  // namespace opentxs
