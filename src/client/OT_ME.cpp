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

#include "opentxs/stdafx.hpp"

#include "opentxs/client/OT_ME.hpp"

#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/client/commands/CmdAcceptInbox.hpp"
#include "opentxs/client/commands/CmdAcceptPayments.hpp"
#include "opentxs/client/commands/CmdAcceptReceipts.hpp"
#include "opentxs/client/commands/CmdAcceptTransfers.hpp"
#include "opentxs/client/commands/CmdCancel.hpp"
#include "opentxs/client/commands/CmdDeposit.hpp"
#include "opentxs/client/commands/CmdDiscard.hpp"
#include "opentxs/client/commands/CmdExportCash.hpp"
#include "opentxs/client/commands/CmdSendCash.hpp"
#include "opentxs/client/commands/CmdWithdrawCash.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/ServerAction.hpp"
#include "opentxs/client/SwigWrap.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/cash/Purse.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/Helpers.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/Types.hpp"

#define OT_METHOD "opentxs::OT_ME::"

namespace opentxs
{

OT_ME::OT_ME(
    std::recursive_mutex& lock,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const api::client::ServerAction& serverAction,
    const api::client::Wallet& wallet)
    : lock_(lock)
    , exec_(exec)
    , otapi_(otapi)
    , action_(serverAction)
    , wallet_(wallet)
{
}

bool OT_ME::accept_from_paymentbox(
    const std::string& ACCOUNT_ID,
    const std::string& INDICES,
    const std::string& PAYMENT_TYPE) const
{
    rLock lock(lock_);

    CmdAcceptPayments cmd;
    return 1 == cmd.acceptFromPaymentbox(ACCOUNT_ID, INDICES, PAYMENT_TYPE);
}

bool OT_ME::accept_from_paymentbox_overload(
    const std::string& ACCOUNT_ID,
    const std::string& INDICES,
    const std::string& PAYMENT_TYPE,
    std::string* pOptionalOutput /*=nullptr*/) const
{
    rLock lock(lock_);

    CmdAcceptPayments cmd;
    return 1 ==
           cmd.acceptFromPaymentbox(
               ACCOUNT_ID, INDICES, PAYMENT_TYPE, pOptionalOutput);
}

bool OT_ME::accept_inbox_items(
    const std::string& ACCOUNT_ID,
    std::int32_t nItemType,
    const std::string& INDICES) const
{
    rLock lock(lock_);

    switch (nItemType) {
        case 0: {
            CmdAcceptInbox acceptInbox;
            return 1 == acceptInbox.run(ACCOUNT_ID, INDICES);
        }

        case 1: {
            CmdAcceptTransfers acceptTransfers;
            return 1 == acceptTransfers.run(ACCOUNT_ID, INDICES);
        }

        case 2: {
            CmdAcceptReceipts acceptReceipts;
            return 1 == acceptReceipts.run(ACCOUNT_ID, INDICES);
        }

        default:
            otErr << __FUNCTION__ << ": Invalid nItemType.\n";
            break;
    }

    return false;
}

/** Respond to a bailment request with deposit instructions */
std::string OT_ME::acknowledge_bailment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& REQUEST_ID,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);
    auto action = action_.AcknowledgeBailment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(REQUEST_ID),
        THE_MESSAGE);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        exec_.completePeerReply(nymID, REQUEST_ID);
    }

    return strResponse;
}

/** Acknowledge a connection info request */
std::string OT_ME::acknowledge_connection(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& REQUEST_ID,
    const bool ACK,
    const std::string& URL,
    const std::string& LOGIN,
    const std::string& PASSWORD,
    const std::string& KEY) const
{
    rLock lock(lock_);
    auto action = action_.AcknowledgeConnection(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(REQUEST_ID),
        ACK,
        URL,
        LOGIN,
        PASSWORD,
        KEY);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        exec_.completePeerReply(nymID, REQUEST_ID);
    }

    return strResponse;
}

/** Acknowledge a peer notice */
std::string OT_ME::acknowledge_notice(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& REQUEST_ID,
    const bool ACK) const
{
    rLock lock(lock_);
    auto action = action_.AcknowledgeNotice(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(REQUEST_ID),
        ACK);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        exec_.completePeerReply(nymID, REQUEST_ID);
    }

    return strResponse;
}

/** Respond to an outbailment request with withdrawal instructions */
std::string OT_ME::acknowledge_outbailment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& REQUEST_ID,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);
    auto action = action_.AcknowledgeOutbailment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(REQUEST_ID),
        THE_MESSAGE);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        exec_.completePeerReply(nymID, REQUEST_ID);
    }

    return strResponse;
}

// ACTIVATE SMART CONTRACT -- TRANSACTION
//
std::string OT_ME::activate_smart_contract(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& AGENT_NAME,
    const std::string& THE_SMART_CONTRACT) const
{
    rLock lock(lock_);
    std::unique_ptr<OTSmartContract> contract =
        std::make_unique<OTSmartContract>();

    OT_ASSERT(contract)

    contract->LoadContractFromString(String(THE_SMART_CONTRACT));
    auto action = action_.ActivateSmartContract(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCT_ID),
        String(AGENT_NAME),
        contract);

    return action->Run();
}

std::string OT_ME::adjust_usage_credits(
    const std::string& notaryID,
    const std::string& USER_nymID,
    const std::string& targetNymID,
    const std::string& ADJUSTMENT) const
{
    rLock lock(lock_);
    auto action = action_.AdjustUsageCredits(
        Identifier(USER_nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        std::stoll(ADJUSTMENT));

    return action->Run();
}

bool OT_ME::cancel_outgoing_payments(
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& INDICES) const
{
    rLock lock(lock_);

    CmdCancel cancel;
    return 1 == cancel.run(nymID, ACCOUNT_ID, INDICES);
}

// CANCEL (NOT-YET-RUNNING) PAYMENT PLAN -- TRANSACTION
//
std::string OT_ME::cancel_payment_plan(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_PAYMENT_PLAN) const
{
    rLock lock(lock_);
    std::unique_ptr<OTPaymentPlan> plan = std::make_unique<OTPaymentPlan>();

    OT_ASSERT(plan)

    plan->LoadContractFromString(String(THE_PAYMENT_PLAN));
    auto action = action_.CancelPaymentPlan(
        Identifier(nymID), Identifier(notaryID), plan);

    return action->Run();
}

// CHECK USER (download a public key)
//
std::string OT_ME::check_nym(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID) const
{
    rLock lock(lock_);
    auto action = action_.DownloadNym(
        Identifier(nymID), Identifier(notaryID), Identifier(targetNymID));

    return action->Run();
}

// CREATE ASSET ACCOUNT
//
std::string OT_ME::create_asset_acct(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID) const
{
    rLock lock(lock_);
    auto action = action_.RegisterAccount(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(instrumentDefinitionID));

    return action->Run();
}

// CREATE MARKET OFFER -- TRANSACTION
//
std::string OT_ME::create_market_offer(
    const std::string& ASSET_ACCT_ID,
    const std::string& CURRENCY_ACCT_ID,
    std::int64_t scale,
    std::int64_t minIncrement,
    std::int64_t quantity,
    std::int64_t price,
    bool bSelling,
    std::int64_t lLifespanInSeconds,
    const std::string& STOP_SIGN,
    std::int64_t ACTIVATION_PRICE) const
{
    rLock lock(lock_);
    auto action = action_.CreateMarketOffer(
        Identifier(ASSET_ACCT_ID),
        Identifier(CURRENCY_ACCT_ID),
        scale,
        minIncrement,
        quantity,
        price,
        bSelling,
        std::chrono::seconds(lLifespanInSeconds),
        STOP_SIGN,
        ACTIVATION_PRICE);

    return action->Run();
}

#if OT_CASH
bool OT_ME::deposit_cash(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    rLock lock(lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(notaryID, ACCT_ID, nymID, STR_PURSE, "");
}
#endif  // OT_CASH

std::string OT_ME::deposit_cheque(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_CHEQUE) const
{
    rLock lock(lock_);
    std::unique_ptr<Cheque> cheque = std::make_unique<Cheque>();
    cheque->LoadContractFromString(String(STR_CHEQUE.c_str()));
    auto action = action_.DepositCheque(
        Identifier(nymID), Identifier(notaryID), Identifier(ACCT_ID), cheque);

    return action->Run();
}

#if OT_CASH
bool OT_ME::deposit_local_purse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_INDICES) const
{
    rLock lock(lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(notaryID, ACCT_ID, nymID, "", STR_INDICES);
}
#endif  // OT_CASH

// DEPOSIT PAYMENT PLAN  -- TRANSACTION
std::string OT_ME::deposit_payment_plan(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_PAYMENT_PLAN) const
{
    rLock lock(lock_);
    std::unique_ptr<OTPaymentPlan> plan = std::make_unique<OTPaymentPlan>();

    OT_ASSERT(plan)

    plan->LoadContractFromString(String(THE_PAYMENT_PLAN));
    auto action = action_.DepositPaymentPlan(
        Identifier(nymID), Identifier(notaryID), plan);

    return action->Run();
}

#if OT_CASH
std::string OT_ME::deposit_purse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    rLock lock(lock_);
    std::unique_ptr<Purse> purse(Purse::PurseFactory(String(STR_PURSE)));

    OT_ASSERT(purse);

    auto action = action_.DepositCashPurse(
        Identifier(nymID), Identifier(notaryID), Identifier(ACCT_ID), purse);

    return action->Run();
}

std::int32_t OT_ME::depositCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& accountID,
    bool bReimportIfFailure,  // So we don't re-import a purse that wasn't
                              // internal to begin with.
    std::string* pOptionalOutput /*=nullptr*/) const
{
    rLock lock(lock_);

    std::string recipientNymID = SwigWrap::GetAccountWallet_NymID(accountID);
    if (!VerifyStringVal(recipientNymID)) {
        otOut << "\ndepositCashPurse: Unable to find recipient Nym based on "
                 "myacct. \n";
        return -1;
    }

    bool bPasswordProtected = SwigWrap::Purse_HasPassword(notaryID, oldPurse);

    std::string newPurse;                // being deposited.;
    std::string newPurseForSender = "";  // Probably unused in this case.;
    std::string copyOfOldPurse = oldPurse;
    bool bSuccessProcess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        bPasswordProtected,
        false);

    if (!bSuccessProcess || !VerifyStringVal(newPurse)) {
        otOut << "OT_ME_depositCashPurse: new Purse is empty, after processing "
                 "it for deposit. \n";
        return -1;
    }

    std::unique_ptr<Purse> purse(Purse::PurseFactory(String(newPurse)));

    OT_ASSERT(purse);

    auto action = action_.DepositCashPurse(
        Identifier(nymID), Identifier(notaryID), Identifier(accountID), purse);
    std::string strResponse = action->Run();
    std::string strAttempt = "deposit_cash";

    // HERE, WE INTERPRET THE SERVER REPLY, WHETHER SUCCESS, FAIL, OR ERROR...

    std::int32_t nInterpretReply = InterpretTransactionMsgReply(
        notaryID, recipientNymID, accountID, strAttempt, strResponse);

    if (1 == nInterpretReply) {

        if (nullptr != pOptionalOutput) *pOptionalOutput = strResponse;

        // Download all the intermediary files (account balance, inbox, outbox,
        // etc)
        // since they have probably changed from this operation.
        //
        bool bRetrieved = retrieve_account(
            notaryID,
            recipientNymID,
            accountID,
            true);  // bForceDownload defaults to false.;

        otOut << "\nServer response (" << strAttempt
              << "): SUCCESS depositing cash!\n";
        otOut << std::string(bRetrieved ? "Success" : "Failed")
              << " retrieving intermediary files for account.\n";
    } else  // failure. (so we re-import the cash, so as not to lose it...)
    {

        if (!bPasswordProtected && bReimportIfFailure) {
            bool importStatus = SwigWrap::Wallet_ImportPurse(
                notaryID, instrumentDefinitionID, recipientNymID, newPurse);
            otOut << "Since failure in OT_ME_depositCashPurse, "
                     "OT_API_Wallet_ImportPurse called. Status of "
                     "import: "
                  << importStatus << "\n";

            if (!importStatus) {
                // Raise the alarm here that we failed depositing the purse, and
                // then we failed
                // importing it back into our wallet again.
                otOut << "Error: Failed depositing the cash purse, and then "
                         "failed re-importing it back to wallet. Therefore YOU "
                         "must copy the purse NOW and save it to a safe place! "
                         "\n";

                otOut << newPurse << "\n";

                otOut << "AGAIN: Be sure to copy the above purse "
                         "to a safe place, since it FAILED to "
                         "deposit and FAILED to re-import back "
                         "into the wallet. \n";
            }
        } else {
            otOut << "Error: Failed depositing the cash purse. "
                     "Therefore YOU must copy the purse NOW and "
                     "save it to a safe place! \n";

            otOut << newPurse << "\n";

            otOut << "AGAIN: Be sure to copy the above purse to a "
                     "safe place, since it FAILED to deposit. \n";
        }

        return -1;
    }

    //
    // Return status to caller.
    //
    return nInterpretReply;
}
#endif  // OT_CASH

bool OT_ME::discard_incoming_payments(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& INDICES) const
{
    rLock lock(lock_);

    CmdDiscard discard;
    return 1 == discard.run(notaryID, nymID, INDICES);
}

#if OT_CASH
// Difference between this function and the one above?
// This one automatically retrieves the mint beforehand, if necessary,
// and the account files afterward, if appropriate.
//
bool OT_ME::easy_withdraw_cash(const std::string& ACCT_ID, std::int64_t AMOUNT)
    const
{
    rLock lock(lock_);

    CmdWithdrawCash cmd;
    return 1 == cmd.withdrawCash(ACCT_ID, AMOUNT);
}
#endif  // OT_CASH

// EXCHANGE BASKET CURRENCY
//
std::string OT_ME::exchange_basket_currency(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID,
    const std::string& THE_BASKET,
    const std::string& ACCOUNT_ID,
    bool IN_OR_OUT) const
{
    rLock lock(lock_);
    auto action = action_.ExchangeBasketCurrency(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(instrumentDefinitionID),
        Identifier(ACCOUNT_ID),
        Identifier(THE_BASKET),
        IN_OR_OUT);

    return action->Run();
}

#if OT_CASH
bool OT_ME::exchangeCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    std::string& oldPurse,
    const std::vector<std::string>& selectedTokens) const
{
    rLock lock(lock_);
    std::string newPurse;
    std::string newPurseForSender = "";  // Probably unused in this case.;

    bool bProcessSuccess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        oldPurse,
        selectedTokens,
        nymID,
        false,
        false);  // bIsPasswordProtected=false;

    if (bProcessSuccess && !VerifyStringVal(newPurse)) {
        otOut << "OT_ME_exchangeCashPurse: Before server OT_API_exchangePurse "
                 "call, new Purse is empty. Returning false.\n";
        return false;
    }

    std::unique_ptr<Purse> purse(Purse::PurseFactory(String(newPurse)));

    OT_ASSERT(purse);

    auto action = action_.ExchangeCash(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(instrumentDefinitionID),
        purse);
    const std::string strResponse = action->Run();

    if (!VerifyStringVal(strResponse)) {
        otOut << "IN OT_ME_exchangeCashPurse: theRequest.SendTransaction(() "
                 "failed. (I give up.) \n";

        bool importStatus = SwigWrap::Wallet_ImportPurse(
            notaryID, instrumentDefinitionID, nymID, newPurse);
        otOut << "Since failure in OT_ME_exchangeCashPurse, "
                 "OT_API_Wallet_ImportPurse called, status of import: "
              << std::string(importStatus ? "true" : "false") << "\n";
        if (!importStatus) {
            //          Utility.setObj(newPurse)
        }

        return false;
    }

    //  otOut << "OT_ME_exchangeCashPurse ends, status:
    // success.\n")

    return true;
}

// EXPORT CASH (FROM PURSE)
//
std::string OT_ME::export_cash(
    const std::string& notaryID,
    const std::string& FROM_nymID,
    const std::string& instrumentDefinitionID,
    const std::string& TO_nymID,
    const std::string& STR_INDICES,
    bool bPasswordProtected,
    std::string& STR_RETAINED_COPY) const
{
    rLock lock(lock_);

    std::string to_nym_id = TO_nymID;
    CmdExportCash cmd;
    return cmd.exportCash(
        notaryID,
        FROM_nymID,
        instrumentDefinitionID,
        to_nym_id,
        STR_INDICES,
        bPasswordProtected,
        STR_RETAINED_COPY);
}

// Input: server ID, instrumentDefinitionID, Nym of current owner, existing
// purse, list of
// selected tokens, Nym of Recipient, and bool bPasswordProtected.
// Returns: "new Purse"
//
std::string OT_ME::exportCashPurse(
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    const std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    std::string& recipientNymID,
    bool bPasswordProtected,
    std::string& strRetainedCopy) const
{
    rLock lock(lock_);

    //  otOut << "OT_ME_exportCashPurse starts, selectedTokens:" <<
    // selectedTokens << "\n";
    //  Utility.setObj(null);

    if (!bPasswordProtected) {
        // If no recipient, then recipient == Nym.
        //
        if (!VerifyStringVal(recipientNymID) || (recipientNymID.size() == 0)) {
            otOut << "OT_ME_exportCashPurse: recipientNym empty--using NymID "
                     "for recipient instead: "
                  << nymID << "\n";
            recipientNymID = nymID;
        }

        if (!(recipientNymID == nymID)) {
            // Even though we don't use this variable after this point,
            // we've still done something important: loaded and possibly
            // downloaded the recipient Nym, so that later in this function
            // we can reference that recipientNymID in other calls and we know
            // it will work.
            //
            std::string recipientPubKey = load_or_retrieve_encrypt_key(
                notaryID, nymID, recipientNymID);  // this function handles
            // partial IDs for recipient.;

            if (!VerifyStringVal(recipientPubKey)) {
                otOut << "OT_ME_exportCashPurse: recipientPubKey is null\n";
                return "";
            }
        }
    }

    // By this point, we have verified that we can load the public key for the
    // recipient.
    // (IF the exported purse isn't meant to be password-protected.)
    //
    std::string token = "";
    std::string exportedToken = "";
    std::string exportedPurse = "";

    // Next I create another "newPurse" by calling this function.
    //
    std::string newPurse = "";  // for recipient;
    std::string newPurseForSender = "";
    std::string copyOfOldPurse = oldPurse;
    bool bSuccessProcess = processCashPurse(
        newPurse,
        newPurseForSender,
        notaryID,
        instrumentDefinitionID,
        nymID,
        copyOfOldPurse,
        selectedTokens,
        recipientNymID,
        false,
        bPasswordProtected);

    if (bSuccessProcess) {
        strRetainedCopy = newPurseForSender;
    }

    // Whatever is returned from that function, I return here also. Presumably a
    // purse...
    //
    return newPurse;
}
#endif  // OT_CASH

// GET BOX RECEIPT
// Note: nBoxType is 0 for Nymbox, 1 for Inbox, and 2 for Outbox.
// Also, if nBoxType is 0 (nymbox) then you have to pass the NymID in the
// ACCT_ID
// argument, as well as the nymID argument (you have to pass it twice...)
// Otherwise for inbox/outbox, pass the actual ACCT_ID there as normal.
//
std::string OT_ME::get_box_receipt(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    std::int32_t nBoxType,
    std::int64_t TRANS_NUM) const
{
    rLock lock(lock_);
    auto action = action_.DownloadBoxReceipt(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCT_ID),
        static_cast<RemoteBoxType>(nBoxType),
        TRANS_NUM);

    return action->Run();
}

std::string OT_ME::get_market_list(
    const std::string& notaryID,
    const std::string& nymID) const
{
    rLock lock(lock_);
    auto action =
        action_.DownloadMarketList(Identifier(nymID), Identifier(notaryID));

    return action->Run();
}

std::string OT_ME::get_market_offers(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& MARKET_ID,
    std::int64_t MAX_DEPTH) const
{
    rLock lock(lock_);
    auto action = action_.DownloadMarketOffers(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(MARKET_ID),
        MAX_DEPTH);

    return action->Run();
}

std::string OT_ME::get_market_recent_trades(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& MARKET_ID) const
{
    rLock lock(lock_);
    auto action = action_.DownloadMarketRecentTrades(
        Identifier(nymID), Identifier(notaryID), Identifier(MARKET_ID));

    return action->Run();
}

std::string OT_ME::get_nym_market_offers(
    const std::string& notaryID,
    const std::string& nymID) const
{
    rLock lock(lock_);
    auto action = action_.DownloadNymMarketOffers(
        Identifier(nymID), Identifier(notaryID));

    return action->Run();
}

// GET PAYMENT INSTRUMENT (from payments inbox, by index.)
//
std::string OT_ME::get_payment_instrument(
    const std::string& notaryID,
    const std::string& nymID,
    std::int32_t nIndex,
    const std::string& PRELOADED_INBOX) const
{
    rLock lock(lock_);

    std::string strInstrument;
    std::string strInbox =
        VerifyStringVal(PRELOADED_INBOX)
            ? PRELOADED_INBOX
            : exec_.LoadPaymentInbox(
                  notaryID, nymID);  // Returns nullptr, or an inbox.

    if (!VerifyStringVal(strInbox)) {
        otWarn << "\n\n get_payment_instrument:  "
                  "OT_API_LoadPaymentInbox Failed. (Probably just "
                  "doesn't exist yet.)\n\n";
        return "";
    }

    std::int32_t nCount =
        exec_.Ledger_GetCount(notaryID, nymID, nymID, strInbox);
    if (0 > nCount) {
        otOut
            << "Unable to retrieve size of payments inbox ledger. (Failure.)\n";
        return "";
    }
    if (nIndex > (nCount - 1)) {
        otOut << "Index " << nIndex
              << " out of bounds. (The last index is: " << (nCount - 1)
              << ". The first is 0.)\n";
        return "";
    }

    strInstrument =
        exec_.Ledger_GetInstrument(notaryID, nymID, nymID, strInbox, nIndex);
    if (!VerifyStringVal(strInstrument)) {
        otOut << "Failed trying to get payment instrument from payments box.\n";
        return "";
    }

    return strInstrument;
}

#if OT_CASH
// Imports a purse into the wallet.
// NOTE:   UNUSED currently.
bool OT_ME::importCashPurse(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID,
    std::string& userInput,
    bool isPurse) const
{
    rLock lock(lock_);

    //  otOut << "OT_ME_importCashPurse, notaryID:" << notaryID << "
    // nymID:" << nymID << " instrumentDefinitionID:" <<
    // instrumentDefinitionID);
    //  otOut << "OT_ME_importCashPurse, userInput purse:" <<
    // userInput <<);

    if (!isPurse)  // it's not a purse. Must be a
                   // token, so let's create a purse
                   // for it.
    {
        //      otOut << "OT_ME_importCashPurse, isPurse:" +
        // isPurse)

        std::string purse = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID, nymID == owner, signer;

        if (!VerifyStringVal(purse)) {
            otOut << "OT_ME_importCashPurse: Error: "
                     "OT_API_CreatePurse returned null\n";
            return false;
        }
        //      otOut << "OT_ME_importCashPurse, OT_API_CreatePurse
        // return :" + purse);

        std::string newPurse = SwigWrap::Purse_Push(
            notaryID, instrumentDefinitionID, nymID, nymID, purse, userInput);
        if (!VerifyStringVal(newPurse)) {
            otOut << "OT_ME_importCashPurse: Error: "
                     "OT_API_Purse_Push returned null\n";
            return false;
        }
        //      otOut << "OT_ME_importCashPurse, OT_API_Purse_Push
        // returned :" + newPurse);
        userInput = newPurse;
    }
    //  otOut << "OT_ME_importCashPurse, Before calling
    // OT_API_Wallet_ImportPurse, final purse:" + userInput);
    //  otOut << "OT_ME_importCashPurse just before api ,
    // notaryID:" + notaryID + " nymID:" + nymID + " instrumentDefinitionID:" +
    // instrumentDefinitionID);

    // Here we have either a purse that was passed in, or a purse that we
    // created so
    // we could add the token that was passed in. Either way, we have a purse
    // now, so
    // let's import it into the wallet.
    //
    return 1 == SwigWrap::Wallet_ImportPurse(
                    notaryID, instrumentDefinitionID, nymID, userInput);
}
#endif  // OT_CASH

/** Request a deposit of some asset in exchange for an OT balance */
std::string OT_ME::initiate_bailment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& instrumentDefinitionID) const
{
    rLock lock(lock_);
    auto action = action_.InitiateBailment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(instrumentDefinitionID));
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

/** Request a redemption of an OT balance for the underlying asset*/
std::string OT_ME::initiate_outbailment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& instrumentDefinitionID,
    const std::int64_t& AMOUNT,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);
    auto action = action_.InitiateOutbailment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(instrumentDefinitionID),
        AMOUNT,
        THE_MESSAGE);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// This code was repeating a lot, so I just added a function for it.
//
// It uses the above functions.
//
std::int32_t OT_ME::InterpretTransactionMsgReply(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Attempt,
    const std::string& str_Response) const
{
    rLock lock(lock_);

    std::int32_t nMessageSuccess = VerifyMessageSuccess(str_Response);

    if ((-1) == nMessageSuccess) {
        otOut << __FUNCTION__ << ": Message error: " << str_Attempt << "\n";
        return (-1);
    } else if (0 == nMessageSuccess) {
        otOut << __FUNCTION__ << ": Server reply (" << str_Attempt
              << "): Message failure.\n";
        return 0;
    }
    // (else 1.)
    std::int32_t nBalanceSuccess = VerifyMsgBalanceAgrmntSuccess(
        notaryID, nymID, ACCOUNT_ID, str_Response);

    if ((-1) == nBalanceSuccess) {
        otOut << __FUNCTION__ << ": Balance agreement error: " << str_Attempt
              << "\n";
        return (-1);
    } else if (0 == nBalanceSuccess) {
        otOut << __FUNCTION__ << ": Server reply (" << str_Attempt
              << "): Balance agreement failure.\n";
        return 0;
    }
    // (else 1.)
    std::int32_t nTransSuccess =
        VerifyMsgTrnxSuccess(notaryID, nymID, ACCOUNT_ID, str_Response);

    if ((-1) == nTransSuccess) {
        otOut << __FUNCTION__ << ": Transaction error: " << str_Attempt << "\n";
        return (-1);
    } else if (0 == nTransSuccess) {
        otOut << __FUNCTION__ << ": Server reply (" << str_Attempt
              << "): Transaction failure.\n";
        return 0;
    }
    // (else 1.)
    //
    // Success!
    //
    return 1;
}

// ISSUE ASSET TYPE
//
std::string OT_ME::issue_asset_type(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_CONTRACT) const
{
    rLock lock(lock_);
    auto action = action_.IssueUnitDefinition(
        Identifier(nymID),
        Identifier(notaryID),
        proto::StringToProto<proto::UnitDefinition>(
            String(THE_CONTRACT.c_str())));

    return action->Run();
}

// ISSUE BASKET CURRENCY
//
std::string OT_ME::issue_basket_currency(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& THE_BASKET) const
{
    rLock lock(lock_);
    auto action = action_.IssueBasketCurrency(
        Identifier(nymID),
        Identifier(notaryID),
        proto::StringToProto<proto::UnitDefinition>(
            String(THE_BASKET.c_str())));

    return action->Run();
}

// KILL MARKET OFFER -- TRANSACTION
//
std::string OT_ME::kill_market_offer(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ASSET_ACCT_ID,
    std::int64_t TRANS_NUM) const
{
    rLock lock(lock_);
    auto action = action_.KillMarketOffer(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ASSET_ACCT_ID),
        TRANS_NUM);

    return action->Run();
}

// KILL (ACTIVE) PAYMENT PLAN -- TRANSACTION
//
std::string OT_ME::kill_payment_plan(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    std::int64_t TRANS_NUM) const
{
    rLock lock(lock_);
    auto action = action_.KillPaymentPlan(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCT_ID),
        TRANS_NUM);

    return action->Run();
}

// load_or_retrieve_pubkey()
//
// Load targetNymID from local storage.
// If not there, then retrieve targetNymID from server,
// using nymID to send check_nym request. Then re-load
// and return. (Might still return null.)
//
std::string OT_ME::load_or_retrieve_encrypt_key(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID) const
{
    rLock lock(lock_);
    std::string strPubkey = load_public_encryption_key(targetNymID);

    if (!VerifyStringVal(strPubkey)) {
        std::string strResponse = check_nym(notaryID, nymID, targetNymID);

        if (1 == VerifyMessageSuccess(strResponse)) {
            strPubkey = load_public_encryption_key(targetNymID);
        }
    }

    return strPubkey;  // might be null.
}

// load_public_key():
//
// Load a public key from local storage, and return it (or null).
//
// TODO: Need to fix ugly error messages by passing a bChecking in here
// so the calling function can try to load the pubkey just to see if it's there,
// without causing ugly error logs when there's no error.
std::string OT_ME::load_public_encryption_key(const std::string& nymID) const
{
    rLock lock(lock_);
    otOut << "\nload_public_encryption_key: Trying to load public "
             "key, assuming Nym isn't in the local wallet...\n";
    std::string strPubkey = SwigWrap::LoadPubkey_Encryption(
        nymID);  // This version is for "other people";

    if (!VerifyStringVal(strPubkey)) {
        otOut << "\nload_public_encryption_key: Didn't find the Nym (" << nymID
              << ") as an 'other' user, so next, checking to see if there's "
                 "a pubkey available for one of the local private Nyms...\n";
        strPubkey = SwigWrap::LoadUserPubkey_Encryption(
            nymID);  // This version is for "the user sitting at the machine.";

        if (!VerifyStringVal(strPubkey)) {
            otOut << "\nload_public_encryption_key: Didn't find "
                     "him as a local Nym either... returning nullptr.\n";
        }
    }

    return strPubkey;  // might be null.;
}

bool OT_ME::make_sure_enough_trans_nums(
    std::int32_t nNumberNeeded,
    const std::string& strMyNotaryID,
    const std::string& strMyNymID) const
{
    return action_.GetTransactionNumbers(
        Identifier(strMyNymID), Identifier(strMyNotaryID), nNumberNeeded);
}

/** Notify a nym of a pending blockchain deposit */
std::string OT_ME::notify_bailment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::string& instrumentDefinitionID,
    const std::string& TXID,
    const std::string& REQUEST_ID) const
{
    rLock lock(lock_);
    auto action = action_.NotifyBailment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        Identifier(instrumentDefinitionID),
        Identifier(REQUEST_ID),
        TXID);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// PAY DIVIDEND -- TRANSACTION
//
std::string OT_ME::pay_dividend(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& SOURCE_ACCT_ID,
    const std::string& SHARES_instrumentDefinitionID,
    const std::string& STR_MEMO,
    std::int64_t AMOUNT_PER_SHARE) const
{
    rLock lock(lock_);
    auto action = action_.PayDividend(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(SHARES_instrumentDefinitionID),
        Identifier(SOURCE_ACCT_ID),
        STR_MEMO,
        AMOUNT_PER_SHARE);

    return action->Run();
}

// PING NOTARY
//
std::string OT_ME::ping_notary(
    const std::string& notaryID,
    const std::string& nymID) const
{
    auto context =
        wallet_.mutable_ServerContext(Identifier(nymID), Identifier(notaryID));
    const auto response = context.It().PingNotary();
    const auto& reply = response.second;

    if (false == bool(reply)) {

        return {};
    }

    return String(*reply).Get();
}

// PROCESS INBOX -- TRANSACTION
//
std::string OT_ME::process_inbox(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& RESPONSE_LEDGER) const
{
    rLock lock(lock_);
    std::unique_ptr<Ledger> ledger(new Ledger(
        Identifier(nymID), Identifier(ACCOUNT_ID), Identifier(notaryID)));

    OT_ASSERT(ledger)

    const auto loaded = ledger->LoadLedgerFromString(String(RESPONSE_LEDGER));

    OT_ASSERT(loaded);

    auto action = action_.ProcessInbox(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCOUNT_ID),
        ledger);

    return action->Run();
}

#if OT_CASH
// processCashPurse pops the selected tokens off of oldPurse, changes their
// owner to recipientNymID,
// and pushes them onto newPurse, owned by recipientNymID. Meanwhile any
// unselected tokens are pushed
// onto newPurseUnselectedTokens, owned by nymID, which is saved to local
// storage (overwriting...)
// newPurse is returned from this function.
//
// Thus, for example, if there's a problem depositing/exporting/exchanging
// newPurse, then it needs
// to be merged back into the old purse again, which is otherwise still missing
// those tokens. (We
// should keep a copy of them in the payments outbox or something like that, in
// the meantime.)
//
// What is newPurseForSender for?
// Sometimes when processCashPurse is called, a Nym is just removing tokens from
// his purse so he can
// deposit them into his own account. Meaning the "sender" Nym (who owns the
// purse) is the same as the
// "recipient" Nym (who owns the account.) In which case, newPurse (for
// recipient) and newPurseForSender
// will contain the exact same thing. But in the case where they are different
// Nyms (like if I am
// exporting these tokens from my purse in order to give them to someone else)
// then I will want a copy
// of the exported purse for the recipient, and a copy of the same exported
// purse for the sender.
// processCashPurse returns both of those for me. The reason is because
// otherwise, if I just encrypted
// some of my cash tokens to some external Nym, then only THAT Nym's private key
// will ever open them up
// again. Even if I have them in my outpayments box, I can never recover them
// because I can never again
// decrypt them. Therefore I make a special copy of the outgoing purse which is
// encrypted to my own key,
// and put that into my outpayments box instead of the one I sent to the
// recipient (or including both
// of them.) That way the cash can still be recovered if necessary, from my
// outpayments box, and re-imported
// back into my cash purse again.
//
bool OT_ME::processCashPurse(
    std::string& newPurse,
    std::string& newPurseForSender,
    const std::string& notaryID,
    const std::string& instrumentDefinitionID,
    const std::string& nymID,
    std::string& oldPurse,
    const std::vector<std::string>& selectedTokens,
    const std::string& recipientNymID,
    bool bPWProtectOldPurse,
    bool bPWProtectNewPurse) const
{
    rLock lock(lock_);

    // By this point, we know that "selected tokens" has a size of 0, or MORE
    // THAN ONE. (But NOT 1 exactly.)
    // (At least, if this function was called by exportCashPurse.)
    std::string strLocation = "OT_ME_processCashPurse";

    // This block handles cases where NO TOKENS ARE SELECTED.
    //
    // (Meaning: "PROCESS" THEM ALL.)
    //
    if (selectedTokens.size() < 1) {
        // newPurse is created, OWNED BY RECIPIENT.
        //
        newPurse =
            (bPWProtectNewPurse ? SwigWrap::CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : SwigWrap::CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID is owner,
                                                // nymID is signer;

        if (!VerifyStringVal(newPurse)) {
            otOut << strLocation << ": "
                  << (bPWProtectNewPurse ? "OT_API_CreatePurse_Passphrase"
                                         : "OT_API_CreatePurse")
                  << " returned null\n";
            return false;
        }

        // We'll create an extra copy of the newPurse, which is encrypted to the
        // sender (instead of the recipient or
        // some passphrase.) We'll call it newPurseForSender. This way the
        // sender can later have the option to recover
        // the cash from his outbox.
        //
        newPurseForSender = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID is owner, nymID is signer;

        if (!VerifyStringVal(newPurseForSender)) {
            otOut << strLocation
                  << ": Failure: OT_API_CreatePurse returned null\n";
            return false;
        }

        // Iterate through the OLD PURSE. (as tempOldPurse.)
        //
        std::int32_t count =
            SwigWrap::Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek into TOKEN, from the top token on the stack. (And it's STILL
            // on top after this call.)
            //
            std::string token = SwigWrap::Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Now pop the token off of tempOldPurse (our iterator for the old
            // purse).
            // Store updated copy of purse (sans token) into "str1".
            //
            std::string str1 = SwigWrap::Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(token) || !VerifyStringVal(str1)) {
                otOut << strLocation
                      << ": OT_API_Purse_Peek or OT_API_Purse_Pop "
                         "returned null... SHOULD NEVER HAPPEN. "
                         "Returning null.\n";
                return false;
            }

            // Since pop succeeded, copy the output to tempOldPurse
            // (for next iteration, in case any continues happen below.)
            // Now tempOldPurse contains what it did before, MINUS ONE TOKEN.
            // (The exported one.)
            //
            tempOldPurse = str1;

            std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
            std::string strRecipient =
                bPWProtectNewPurse ? newPurse : recipientNymID;

            std::string strSenderAsRecipient =
                nymID;  // Used as the "owner" of newPurseForSender. (So the
                        // sender can recover his sent coins that got encrypted
                        // to someone else's key.);

            // Change the OWNER on token, from NymID to RECIPIENT.
            // (In this block, we change ALL the tokens in the purse.)
            //
            std::string exportedToken = SwigWrap::Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,          // signer ID
                strSender,      // old owner
                strRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(exportedToken)) {
                otOut << strLocation << ": 1, OT_API_Token_ChangeOwner "
                                        "returned null...(should never "
                                        "happen) Returning null.\n";
                return false;
            }

            // SAVE A COPY FOR THE SENDER...
            //
            std::string retainedToken = SwigWrap::Token_ChangeOwner(
                notaryID,
                instrumentDefinitionID,
                token,
                nymID,                  // signer ID
                strSender,              // old owner
                strSenderAsRecipient);  // new owner
            // If change failed, then continue.
            //
            if (!VerifyStringVal(retainedToken)) {
                otOut << strLocation << ":  2, OT_API_Token_ChangeOwner "
                                        "returned null...(should never "
                                        "happen) Returning null.\n";
                return false;
            }

            //          strSender    = bPWProtectOldPurse ? "" : nymID // unused
            // here. not needed.
            strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

            // PUSH the EXPORTED TOKEN (new owner) into the new purse (again,
            // recipient/newPurse is new owner) and save results in
            // "strPushedForRecipient".
            // Results are, FYI, newPurse+exportedToken.
            //
            std::string strPushedForRecipient = SwigWrap::Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,         // server, asset, signer
                strRecipient,  // owner is either nullptr (for
                               // password-protected
                               // purse) or recipientNymID
                newPurse,
                exportedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRecipient)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 1 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // PUSH the RETAINED TOKEN (copy for original owner) into the
            // newPurseForSender and save results in "strPushedForRetention".
            // Results are, FYI, newPurseForSender+retainedToken.
            //
            std::string strPushedForRetention = SwigWrap::Purse_Push(
                notaryID,
                instrumentDefinitionID,
                nymID,                 // server, asset, signer
                strSenderAsRecipient,  // This version of the purse is the
                // outgoing copy (for the SENDER's notes).
                // Thus strSenderAsRecipient.
                newPurseForSender,
                retainedToken);  // purse, token

            // If push failed, then continue.
            if (!VerifyStringVal(strPushedForRetention)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Push 2 returned null... "
                         "(should never happen) Returning null.\n";
                return false;
            }

            // Since push succeeded, copy "strPushedForRecipient" (containing
            // newPurse         +exportedToken) into newPurse.
            // Since push succeeded, copy "strPushedForRetention" (containing
            // newPurseForSender+retainedToken) into newPurseForSender.
            //
            newPurse = strPushedForRecipient;
            newPurseForSender = strPushedForRetention;
        }  // for

        // Save tempOldPurse to local storage. (For OLD Owner.)
        // By now, all of the tokens have been popped off of this purse, so it
        // is EMPTY.
        // We're now saving the empty purse, since the user exported all of the
        // tokens.
        //
        // THERE MAYBE SHOULD BE AN EXTRA MODAL HERE, that says,
        // "Moneychanger will now save your purse, EMPTY, back to local storage.
        // Are you sure you want to do this?"
        //

        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that
                                  // is, it's encrypted to a Nym.)
        {
            if (!SwigWrap::SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    tempOldPurse))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation << ": OT_API_SavePurse "
                                        "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse =
                tempOldPurse;  // We never cared about this with Nym-owned
                               // old purse, since it saves to storage
                               // anyway, in the above block. But now in
                               // the case of password-protected purses,
                               // we set the oldPurse to contain the new
                               // version of itself (containing the tokens
                               // that had been left unselected) so the
                               // caller can do what he wills with it.;
        }
    }

    // Else, SPECIFIC TOKENS were selected, so process those only...
    //
    else {
        //      otOut << "Tokens in Cash Purse being processed");

        // newPurseSelectedTokens is created (CORRECTLY) with recipientNymID as
        // owner. (Or with a symmetric key / passphrase.)
        // newPurseUnSelectedTokens is created (CORRECTLY) with NymID as owner.
        // (Unselected tokens aren't being exported...)
        //
        std::string newPurseUnSelectedTokens = SwigWrap::Purse_Empty(
            notaryID,
            instrumentDefinitionID,
            nymID,
            oldPurse);  // Creates an empty copy of oldPurse.;
        std::string newPurseSelectedTokens =
            (bPWProtectNewPurse ? SwigWrap::CreatePurse_Passphrase(
                                      notaryID, instrumentDefinitionID, nymID)
                                : SwigWrap::CreatePurse(
                                      notaryID,
                                      instrumentDefinitionID,
                                      recipientNymID,
                                      nymID));  // recipientNymID = owner,
                                                // nymID = signer;
        std::string newPurseSelectedForSender = SwigWrap::CreatePurse(
            notaryID,
            instrumentDefinitionID,
            nymID,
            nymID);  // nymID = owner, nymID = signer. This is a copy of
                     // newPurseSelectedTokens that's encrypted to the SENDER
                     // (for putting in his outpayments box, so he can still
                     // decrypt if necessary.);

        if (!VerifyStringVal(newPurseSelectedForSender)) {
            otOut << strLocation << ":  OT_API_CreatePurse returned null\n";
            return false;
        }
        if (!VerifyStringVal(newPurseSelectedTokens)) {
            otOut << strLocation
                  << ":  OT_API_CreatePurse or "
                     "OT_API_CreatePurse_Passphrase returned null\n";
            return false;
        }
        if (!VerifyStringVal((newPurseUnSelectedTokens))) {
            otOut << strLocation << ":  OT_API_Purse_Empty returned null\n";
            return false;
        }

        // Iterate through oldPurse, using tempOldPurse as iterator.
        //
        std::int32_t count =
            SwigWrap::Purse_Count(notaryID, instrumentDefinitionID, oldPurse);
        std::string tempOldPurse = oldPurse;

        for (std::int32_t i = 0; i < count; ++i) {
            // Peek at the token on top of the stack.
            // (Without removing it.)
            //
            std::string token = SwigWrap::Purse_Peek(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            // Remove the top token from the stack, and return the updated stack
            // in "str1".
            //
            std::string str1 = SwigWrap::Purse_Pop(
                notaryID, instrumentDefinitionID, nymID, tempOldPurse);

            if (!VerifyStringVal(str1) || !VerifyStringVal(token)) {
                otOut << strLocation
                      << ":  OT_API_Purse_Peek or "
                         "OT_API_Purse_Pop returned null... returning Null. "
                         "(SHOULD NEVER HAPPEN.)\n";
                return false;
            }

            // Putting updated purse into iterator, so any subsequent continues
            // will work properly.
            //
            tempOldPurse = str1;

            // Grab the TokenID for that token. (Token still has OLD OWNER.)
            //
            std::string tokenID =
                SwigWrap::Token_GetID(notaryID, instrumentDefinitionID, token);

            if (!VerifyStringVal(tokenID)) {
                otOut << strLocation
                      << ":  OT_API_Token_GetID returned null... "
                         "SHOULD NEVER HAPPEN. Returning now.\n";
                return false;
            }

            // At this point, we check TokenID (identifying the current token)
            // to see if it's on the SELECTED LIST.
            //
            if (find(selectedTokens.begin(), selectedTokens.end(), tokenID) !=
                selectedTokens.end())  // We ARE exporting
                                       // this token. (Its
                                       // ID was on the
                                       // list.)
            {
                // CHANGE OWNER from NYM to RECIPIENT
                // "token" will now contain the EXPORTED TOKEN, with the NEW
                // OWNER.
                //
                std::string strSender = bPWProtectOldPurse ? oldPurse : nymID;
                std::string strRecipient = bPWProtectNewPurse
                                               ? newPurseSelectedTokens
                                               : recipientNymID;

                std::string strSenderAsRecipient =
                    nymID;  // Used as the "owner" of newPurseSelectedForSender.
                // (So the sender can recover his sent coins that got
                // encrypted to someone else's key.);

                std::string exportedToken = SwigWrap::Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,          // server, asset, token,;
                    nymID,          // signer nym
                    strSender,      // old owner
                    strRecipient);  // new owner
                if (!VerifyStringVal(exportedToken)) {
                    otOut << strLocation << ": 1  OT_API_Token_ChangeOwner "
                                            "returned null... SHOULD NEVER "
                                            "HAPPEN. Returning now.\n";
                    return false;
                }

                std::string retainedToken = SwigWrap::Token_ChangeOwner(
                    notaryID,
                    instrumentDefinitionID,
                    token,                  // server, asset, token,;
                    nymID,                  // signer nym
                    strSender,              // old owner
                    strSenderAsRecipient);  // new owner
                if (!VerifyStringVal(retainedToken)) {
                    otOut << strLocation << ": 2  OT_API_Token_ChangeOwner "
                                            "returned null... SHOULD NEVER "
                                            "HAPPEN. Returning now.\n";
                    return false;
                }

                // Push exported version of token into new purse for recipient
                // (for selected tokens.)
                //
                //              strSender    = bPWProtectOldPurse ? "" : nymID
                // // unused here. Not needed.
                strRecipient = bPWProtectNewPurse ? "" : recipientNymID;

                std::string strPushedForRecipient = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,         // server, asset, signer;
                    strRecipient,  // owner is either nullptr (for
                    // password-protected purse) or recipientNymID
                    newPurseSelectedTokens,
                    exportedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRecipient)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                // Done: push a copy of these into a purse for the original
                // owner as well, so he has his OWN copy
                // to save in his payments outbox (that HE can decrypt...) so if
                // the cash is lost, for example, he can still
                // recover it. If the recipient receives it and deposits it
                // correctly, the cash in your payment outbox is now
                // worthless and can be discarded, although its existence may be
                // valuable to you as a receipt.
                //
                std::string strPushedForRetention = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,  // server, asset, signer;
                    strSenderAsRecipient,
                    newPurseSelectedForSender,
                    retainedToken);  // purse, token
                if (!VerifyStringVal(strPushedForRetention)) {
                    otOut << strLocation
                          << ":  OT_API_Purse_Push "
                             "newPurseSelectedForSender returned null... "
                             "SHOULD NEVER HAPPEN (returning.)\n";
                    return false;
                }

                newPurseSelectedTokens = strPushedForRecipient;
                newPurseSelectedForSender = strPushedForRetention;

            } else  // The token, this iteration, is NOT being exported, but is
                    // remaining with the original owner.
            {
                std::string strSender = bPWProtectOldPurse ? "" : nymID;

                std::string str = SwigWrap::Purse_Push(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,      // server, asset, signer;
                    strSender,  // owner is either nullptr (for
                                // password-protected
                                // purse) or nymID
                    newPurseUnSelectedTokens,
                    token);  // purse, token
                if (!VerifyStringVal(str)) {
                    otOut << strLocation
                          << ": OT_API_Purse_Push "
                             "newPurseUnSelectedTokens returned null... "
                             "SHOULD NEVER HAPPEN. Returning false.\n";
                    return false;
                }

                newPurseUnSelectedTokens = str;
            }
        }  // for

        if (!bPWProtectOldPurse)  // If old purse is NOT password-protected
                                  // (that
                                  // is, it's encrypted to a Nym.)
        {
            if (!SwigWrap::SavePurse(
                    notaryID,
                    instrumentDefinitionID,
                    nymID,
                    newPurseUnSelectedTokens))  // if FAILURE.
            {
                // No error message if saving fails??
                // No modal?
                //
                // FT: adding log.
                otOut << strLocation << ":  OT_API_SavePurse "
                                        "FAILED. SHOULD NEVER HAPPEN!!!!!!\n";
                return false;
            }
        } else  // old purse IS password protected. (So return its updated
                // version.)
        {
            oldPurse =
                newPurseUnSelectedTokens;  // We never cared about this with
            // Nym-owned old purse, since it saves
            // to storage anyway, in the above
            // block. But now in the case of
            // password-protected purses, we set
            // the oldPurse to contain the new
            // version of itself (containing the
            // tokens that had been left
            // unselected) so the caller can do
            // what he wills with it.;
        }

        // The SELECTED tokens (with Recipient as owner of purse AND tokens
        // within) are returned as the "newPurse".
        // The SELECTED tokens (with Sender as owner of purse AND tokens within)
        // are returned as "newPurseForSender".
        //
        newPurse = newPurseSelectedTokens;
        newPurseForSender = newPurseSelectedForSender;
    }

    return true;
}
#endif  // OT_CASH

std::string OT_ME::register_contract_nym(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT) const
{
    rLock lock(lock_);
    auto action = action_.PublishNym(
        Identifier(nymID), Identifier(notaryID), Identifier(CONTRACT));
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::register_contract_server(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT) const
{
    rLock lock(lock_);
    auto action = action_.PublishServerContract(
        Identifier(nymID), Identifier(notaryID), Identifier(CONTRACT));
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::register_contract_unit(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT) const
{
    rLock lock(lock_);
    auto action = action_.PublishUnitDefinition(
        Identifier(nymID), Identifier(notaryID), Identifier(CONTRACT));
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// REGISTER NYM AT SERVER (or download nymfile, if nym already registered.)
//
std::string OT_ME::register_nym(
    const std::string& notaryID,
    const std::string& nymID) const
{
    rLock lock(lock_);
    std::string strResponse{""};
    // action should destruct when finished
    {
        auto action =
            action_.RegisterNym(Identifier(nymID), Identifier(notaryID));
        strResponse = action->Run();
    }

    if (1 == VerifyMessageSuccess(strResponse)) {
        auto context = wallet_.mutable_ServerContext(
            Identifier(nymID), Identifier(notaryID));
        // Use the getRequestNumber command, thus insuring that the request
        // number is in sync.
        if (0 >= context.It().UpdateRequestNumber()) {
            otOut << OT_METHOD << __FUNCTION__ << ": Registered nym " << nymID
                  << " on notary " << notaryID
                  << " but failed to obtain request number." << std::endl;
        }
    } else {
        // maybe an invalid server ID or the server contract isn't available
        // (do AddServerContract(..) first)
        otOut << OT_METHOD << __FUNCTION__ << "Failed to register nym " << nymID
              << " on notary " << notaryID << std::endl;
    }

    return strResponse;
}

std::string OT_ME::request_admin(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& PASSWORD) const
{
    rLock lock(lock_);
    auto action =
        action_.RequestAdmin(Identifier(nymID), Identifier(notaryID), PASSWORD);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::request_connection(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::int64_t TYPE) const
{
    rLock lock(lock_);
    auto action = action_.InitiateRequestConnection(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        static_cast<proto::ConnectionInfoType>(TYPE));
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// DOWNLOAD ACCOUNT FILES (account balance, inbox, outbox, etc)
//
bool OT_ME::retrieve_account(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    bool bForceDownload) const
{
    rLock lock(lock_);
    auto context =
        wallet_.mutable_ServerContext(Identifier(nymID), Identifier(notaryID));
    Utility MsgUtil(context.It(), otapi_);
    bool bResponse = MsgUtil.getIntermediaryFiles(
        notaryID, nymID, ACCOUNT_ID, bForceDownload);

    return bResponse;
}

#if OT_CASH
// DOWNLOAD PUBLIC MINT
std::string OT_ME::retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID) const
{
    rLock lock(lock_);
    auto action = action_.DownloadMint(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(instrumentDefinitionID));

    return action->Run();
}
#endif  // OT_CASH

// LOAD OR RETRIEVE CONTRACT
//
std::string OT_ME::load_or_retrieve_contract(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT_ID) const
{
    rLock lock(lock_);

    std::string strContract = SwigWrap::GetAssetType_Contract(CONTRACT_ID);

    if (!VerifyStringVal(strContract)) {
        std::string strResponse =
            retrieve_contract(notaryID, nymID, CONTRACT_ID);

        if (1 == VerifyMessageSuccess(strResponse)) {
            strContract = SwigWrap::GetAssetType_Contract(CONTRACT_ID);
        }
    }

    return strContract;  // might be null.
}

#if OT_CASH
// LOAD MINT (from local storage)
//
// To load a mint withOUT retrieving it from server, call:
//
// var strMint = OT_API_LoadMint(notaryID, instrumentDefinitionID);
// It returns the mint, or null.
// LOAD MINT (from local storage).
// Also, if necessary, RETRIEVE it from the server first.
//
// Returns the mint, or null.
//
std::string OT_ME::load_or_retrieve_mint(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& instrumentDefinitionID) const
{
    rLock lock(lock_);
    std::string response = check_nym(notaryID, nymID, nymID);

    if (1 != VerifyMessageSuccess(response)) {
        otOut << "OT_ME_load_or_retrieve_mint: Cannot verify nym for IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << instrumentDefinitionID
              << "\n";
        return "";
    }

    // HERE, WE MAKE SURE WE HAVE THE PROPER MINT...
    //
    // Download the public mintfile if it's not there, or if it's expired.
    // Also load it up into memory as a std::string (just to make sure it
    // works.)

    // expired or missing.
    if (!SwigWrap::Mint_IsStillGood(notaryID, instrumentDefinitionID)) {
        otWarn << "OT_ME_load_or_retrieve_mint: Mint file is "
                  "missing or expired. Downloading from "
                  "server...\n";

        response = retrieve_mint(notaryID, nymID, instrumentDefinitionID);

        if (1 != VerifyMessageSuccess(response)) {
            otOut << "OT_ME_load_or_retrieve_mint: Unable to "
                     "retrieve mint for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << instrumentDefinitionID
                  << "\n";
            return "";
        }

        if (!SwigWrap::Mint_IsStillGood(notaryID, instrumentDefinitionID)) {
            otOut << "OT_ME_load_or_retrieve_mint: Retrieved "
                     "mint, but still 'not good' for IDs: \n";
            otOut << "  Notary ID: " << notaryID << "\n";
            otOut << "     Nym ID: " << nymID << "\n";
            otOut << "   Instrument Definition Id: " << instrumentDefinitionID
                  << "\n";
            return "";
        }
    }
    // else // current mint IS available already on local storage (and not
    // expired.)

    // By this point, the mint is definitely good, whether we had to download it
    // or not.
    // It's here, and it's NOT expired. (Or we would have returned already.)

    std::string strMint = SwigWrap::LoadMint(notaryID, instrumentDefinitionID);
    if (!VerifyStringVal(strMint)) {
        otOut << "OT_ME_load_or_retrieve_mint: Unable to load mint for IDs: \n";
        otOut << "  Notary ID: " << notaryID << "\n";
        otOut << "     Nym ID: " << nymID << "\n";
        otOut << "   Instrument Definition Id: " << instrumentDefinitionID
              << "\n";
    }

    return strMint;
}
#endif  // OT_CASH

// RETRIEVE CONTRACT
//
std::string OT_ME::retrieve_contract(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& CONTRACT_ID) const
{
    rLock lock(lock_);
    auto action = action_.DownloadContract(
        Identifier(nymID), Identifier(notaryID), Identifier(CONTRACT_ID));

    return action->Run();
}

// RETRIEVE NYM INTERMEDIARY FILES
// Returns:
//  True if I have enough numbers, or if there was success getting more
// transaction numbers.
//  False if I didn't have enough numbers, tried to get more, and failed
// somehow.
//
std::int32_t OT_ME::retrieve_nym(
    const std::string& strNotaryID,
    const std::string& strMyNymID,
    bool& bWasMsgSent,
    bool bForceDownload) const
{
    rLock lock(lock_);
    auto context = wallet_.mutable_ServerContext(
        Identifier(strMyNymID), Identifier(strNotaryID));
    Utility MsgUtil(context.It(), otapi_);

    if (0 >= context.It().UpdateRequestNumber()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed calling getRequestNumber" << std::endl;

        return -1;
    } else  // If it returns 1, we know for sure that the request number is in
            // sync.
    {
        otInfo << "SUCCESS syncronizing the request number." << std::endl;
    }

    std::int32_t nGetAndProcessNymbox = MsgUtil.getAndProcessNymbox_4(
        strNotaryID, strMyNymID, bWasMsgSent, bForceDownload);

    return nGetAndProcessNymbox;
}

bool OT_ME::retrieve_nym(
    const std::string& notaryID,
    const std::string& nymID,
    bool bForceDownload) const
{
    bool msgWasSent = false;
    if (0 > retrieve_nym(notaryID, nymID, msgWasSent, bForceDownload)) {
        otOut << "Error: cannot retrieve nym.\n";
        return false;
    }

    return true;
}

// SEND TRANSFER -- TRANSACTION
//
std::string OT_ME::send_transfer(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_FROM,
    const std::string& ACCT_TO,
    std::int64_t AMOUNT,
    const std::string& NOTE) const
{
    rLock lock(lock_);
    auto action = action_.SendTransfer(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCT_FROM),
        Identifier(ACCT_TO),
        AMOUNT,
        NOTE);

    return action->Run();
}

#if OT_CASH
// SEND USER CASH (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_cash(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& recipientNymID,
    const std::string& THE_PAYMENT,
    const std::string& SENDERS_COPY) const
{
    rLock lock(lock_);
    std::unique_ptr<Purse> recipientCopy(
        Purse::PurseFactory(String(THE_PAYMENT)));
    std::unique_ptr<Purse> senderCopy(
        Purse::PurseFactory(String(SENDERS_COPY)));

    OT_ASSERT(recipientCopy);
    OT_ASSERT(senderCopy);

    auto action = action_.SendCash(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(recipientNymID),
        recipientCopy,
        senderCopy);

    return action->Run();
}

#endif  // OT_CASH

// SEND USER MESSAGE (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_msg(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& recipientNymID,
    const std::string& THE_MESSAGE) const
{
    rLock lock(lock_);
    auto action = action_.SendMessage(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(recipientNymID),
        THE_MESSAGE);

    return action->Run();
}

// SEND USER PAYMENT (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_payment(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& recipientNymID,
    const std::string& THE_PAYMENT) const
{
    rLock lock(lock_);
    std::unique_ptr<OTPayment> payment =
        std::make_unique<OTPayment>(String(THE_PAYMENT.c_str()));
    auto action = action_.SendPayment(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(recipientNymID),
        payment);

    return action->Run();
}

std::string OT_ME::server_add_claim(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& SECTION,
    const std::string& TYPE,
    const std::string& VALUE,
    const bool PRIMARY) const
{
    rLock lock(lock_);
    auto action = action_.AddServerClaim(
        Identifier(nymID),
        Identifier(notaryID),
        static_cast<proto::ContactSectionName>(std::stoi(SECTION)),
        static_cast<proto::ContactItemType>(std::stoi(TYPE)),
        VALUE,
        PRIMARY);
    std::string strResponse = action->Run();
    const std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::stat_asset_account(const std::string& ACCOUNT_ID) const
{
    rLock lock(lock_);
    std::string strNymID = SwigWrap::GetAccountWallet_NymID(ACCOUNT_ID);

    if (!VerifyStringVal(strNymID)) {
        otOut << "\nstat_asset_account: Cannot find account wallet for: "
              << ACCOUNT_ID << "\n";
        return "";
    }

    std::string strInstrumentDefinitionID =
        SwigWrap::GetAccountWallet_InstrumentDefinitionID(ACCOUNT_ID);

    if (!VerifyStringVal(strInstrumentDefinitionID)) {
        otOut << "\nstat_asset_account: Cannot cannot determine instrument "
                 "definition for: "
              << ACCOUNT_ID << "\n";
        return "";
    }

    std::string strName = SwigWrap::GetAccountWallet_Name(ACCOUNT_ID);
    std::string strNotaryID = SwigWrap::GetAccountWallet_NotaryID(ACCOUNT_ID);
    std::int64_t lBalance = SwigWrap::GetAccountWallet_Balance(ACCOUNT_ID);
    std::string strAssetTypeName =
        SwigWrap::GetAssetType_Name(strInstrumentDefinitionID);
    std::string strNymName = SwigWrap::GetNym_Name(strNymID);
    std::string strServerName = SwigWrap::GetServer_Name(strNotaryID);

    return "   Balance: " +
           SwigWrap::FormatAmount(strInstrumentDefinitionID, lBalance) +
           "   (" + strName + ")\nAccount ID: " + ACCOUNT_ID + " ( " + strName +
           " )\nAsset Type: " + strInstrumentDefinitionID + " ( " +
           strAssetTypeName + " )\nOwner Nym : " + strNymID + " ( " +
           strNymName + " )\nServer    : " + strNotaryID + " ( " +
           strServerName + " )";
}

std::string OT_ME::store_secret(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& targetNymID,
    const std::int64_t TYPE,
    const std::string& PRIMARY,
    const std::string& SECONDARY) const
{
    rLock lock(lock_);
    auto action = action_.InitiateStoreSecret(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(targetNymID),
        static_cast<proto::SecretType>(TYPE),
        PRIMARY,
        SECONDARY);
    std::string strResponse = action->Run();
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// TRIGGER CLAUSE (on running smart contract) -- TRANSACTION
//
std::string OT_ME::trigger_clause(
    const std::string& notaryID,
    const std::string& nymID,
    std::int64_t TRANS_NUM,
    const std::string& CLAUSE_NAME,
    const std::string& STR_PARAM) const
{
    rLock lock(lock_);
    auto action = action_.TriggerClause(
        Identifier(nymID),
        Identifier(notaryID),
        TRANS_NUM,
        CLAUSE_NAME,
        STR_PARAM);

    return action->Run();
}

// DELETE ASSET ACCOUNT
//
std::string OT_ME::unregister_account(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID) const
{
    rLock lock(lock_);
    auto action = action_.UnregisterAccount(
        Identifier(nymID), Identifier(notaryID), Identifier(ACCOUNT_ID));

    return action->Run();
}

// UNREGISTER NYM FROM SERVER
//
std::string OT_ME::unregister_nym(
    const std::string& notaryID,
    const std::string& nymID) const
{
    rLock lock(lock_);
    auto action =
        action_.UnregisterNym(Identifier(nymID), Identifier(notaryID));

    return action->Run();
}

std::int32_t OT_ME::VerifyMessageSuccess(const std::string& str_Message) const
{
    rLock lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = exec_.Message_GetSuccess(str_Message);

    switch (nStatus) {
        case (-1):
            otOut << __FUNCTION__
                  << ": Error calling OT_API_Message_GetSuccess, for message:\n"
                  << str_Message << "\n";
            break;
        case (0):
            otWarn << __FUNCTION__
                   << ": Reply received: success == FALSE. Reply message:\n"
                   << str_Message << "\n";
            break;
        case (1):
            otWarn << __FUNCTION__ << ": Reply received: success == TRUE.\n";
            break;
        default:
            otOut << __FUNCTION__
                  << ": Error. (This should never happen!) nStatus: " << nStatus
                  << ", Input:\n"
                  << str_Message << "\n";
            nStatus = (-1);
            break;
    }

    return nStatus;
}

std::int32_t OT_ME::VerifyMsgBalanceAgrmntSuccess(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Message) const
{
    rLock lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = exec_.Message_GetBalanceAgreementSuccess(
        notaryID, nymID, ACCOUNT_ID, str_Message);

    switch (nStatus) {
        case (-1):
            otOut << __FUNCTION__
                  << ": Error calling Msg_GetBlnceAgrmntSuccess, for message:\n"
                  << str_Message << "\n";
            break;
        case (0):
            otWarn << __FUNCTION__
                   << ": Reply received: success == FALSE. Reply message:\n"
                   << str_Message << "\n";
            break;
        case (1):
            otWarn << __FUNCTION__ << ": Reply received: success == TRUE.\n";
            break;
        default:
            otOut << __FUNCTION__
                  << ": Error. (This should never happen!) nStatus: " << nStatus
                  << ", Input:\n"
                  << str_Message << "\n";
            nStatus = (-1);
            break;
    }

    return nStatus;
}

std::int32_t OT_ME::VerifyMsgTrnxSuccess(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Message) const
{
    rLock lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = exec_.Message_GetTransactionSuccess(
        notaryID, nymID, ACCOUNT_ID, str_Message);

    switch (nStatus) {
        case (-1):
            otOut << __FUNCTION__
                  << ": Error calling Msg_GetTransactionSuccess, for message:\n"
                  << str_Message << "\n";
            break;
        case (0):
            otWarn << __FUNCTION__
                   << ": Reply received: success == FALSE. Reply message:\n"
                   << str_Message << "\n";
            break;
        case (1):
            otWarn << __FUNCTION__ << ": Reply received: success == TRUE.\n";
            break;
        default:
            otOut << __FUNCTION__
                  << ": Error. (This should never happen!) nStatus: " << nStatus
                  << ", Input:\n"
                  << str_Message << "\n";
            nStatus = (-1);
            break;
    }

    return nStatus;
}

#if OT_CASH
bool OT_ME::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& recipientNymID,
    std::int64_t AMOUNT) const
{
    rLock lock(lock_);

    CmdSendCash sendCash;
    return 1 ==
           sendCash.run(
               "",
               "",
               ACCT_ID,
               "",
               recipientNymID,
               std::to_string(AMOUNT),
               "",
               "");
}

// WITHDRAW CASH -- TRANSACTION
//
std::string OT_ME::withdraw_cash(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    std::int64_t AMOUNT) const
{
    rLock lock(lock_);
    auto action = action_.WithdrawCash(
        Identifier(nymID), Identifier(notaryID), Identifier(ACCT_ID), AMOUNT);

    return action->Run();
}
#endif  // OT_CASH

// WITHDRAW VOUCHER -- TRANSACTION
//
std::string OT_ME::withdraw_voucher(
    const std::string& notaryID,
    const std::string& nymID,
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& STR_MEMO,
    std::int64_t AMOUNT) const
{
    rLock lock(lock_);
    auto action = action_.WithdrawVoucher(
        Identifier(nymID),
        Identifier(notaryID),
        Identifier(ACCT_ID),
        Identifier(RECIPIENT_NYM_ID),
        AMOUNT,
        STR_MEMO);

    return action->Run();
}

// used for passing and returning values when giving a
// lambda function to a loop function.
//
// cppcheck-suppress uninitMemberVar
the_lambda_struct::the_lambda_struct() {}

OTDB::OfferListNym* loadNymOffers(
    const std::string& notaryID,
    const std::string& nymID)
{
    OTDB::OfferListNym* offerList = nullptr;

    if (OTDB::Exists("nyms", notaryID, "offers", nymID + ".bin")) {
        otWarn << "Offers file exists... Querying nyms...\n";
        OTDB::Storable* storable = OTDB::QueryObject(
            OTDB::STORED_OBJ_OFFER_LIST_NYM,
            "nyms",
            notaryID,
            "offers",
            nymID + ".bin");

        if (nullptr == storable) {
            otOut << "Unable to verify storable object. Probably doesn't "
                     "exist.\n";
            return nullptr;
        }

        otWarn << "QueryObject worked. Now dynamic casting from storable to a "
                  "(nym) offerList...\n";
        offerList = dynamic_cast<OTDB::OfferListNym*>(storable);

        if (nullptr == offerList) {
            otOut
                << "Unable to dynamic cast a storable to a (nym) offerList.\n";
            return nullptr;
        }
    }

    return offerList;
}

MapOfMaps* convert_offerlist_to_maps(OTDB::OfferListNym& offerList)
{
    std::string strLocation = "convert_offerlist_to_maps";

    MapOfMaps* map_of_maps = nullptr;

    // LOOP THROUGH THE OFFERS and sort them std::into a map_of_maps, key is:
    // scale-instrumentDefinitionID-currencyID
    // the value for each key is a sub-map, with the key: transaction ID and
    // value: the offer data itself.
    //
    std::int32_t nCount = offerList.GetOfferDataNymCount();
    std::int32_t nTemp = nCount;

    if (nCount > 0) {
        for (std::int32_t nIndex = 0; nIndex < nCount; ++nIndex) {

            nTemp = nIndex;
            OTDB::OfferDataNym* offerDataPtr = offerList.GetOfferDataNym(nTemp);

            if (!offerDataPtr) {
                otOut << strLocation << ": Unable to reference (nym) offerData "
                                        "on offerList, at index: "
                      << nIndex << "\n";
                return map_of_maps;
            }

            OTDB::OfferDataNym& offerData = *offerDataPtr;
            std::string strScale = offerData.scale;
            std::string strInstrumentDefinitionID =
                offerData.instrument_definition_id;
            std::string strCurrencyTypeID = offerData.currency_type_id;
            std::string strSellStatus = offerData.selling ? "SELL" : "BUY";
            std::string strTransactionID = offerData.transaction_id;

            std::string strMapKey = strScale + "-" + strInstrumentDefinitionID +
                                    "-" + strCurrencyTypeID;

            SubMap* sub_map = nullptr;
            if (nullptr != map_of_maps && !map_of_maps->empty() &&
                (map_of_maps->count(strMapKey) > 0)) {
                sub_map = (*map_of_maps)[strMapKey];
            }

            if (nullptr != sub_map) {
                otWarn << strLocation << ": The sub-map already exists!\n";

                // Let's just add this offer to the existing submap
                // (There must be other offers already there for the same
                // market, since the submap already exists.)
                //
                // the sub_map for this market is mapped by BUY/SELL ==> the
                // actual offerData.
                //

                (*sub_map)[strTransactionID] = &offerData;
            } else  // submap does NOT already exist for this market. (Create
                    // it...)
            {
                otWarn << strLocation
                       << ": The sub-map does NOT already exist!\n";
                //
                // Let's create the submap with this new offer, and add it
                // to the main map.
                //
                sub_map = new SubMap;
                (*sub_map)[strTransactionID] = &offerData;

                if (nullptr == map_of_maps) {
                    map_of_maps = new MapOfMaps;
                }

                (*map_of_maps)[strMapKey] = sub_map;
            }

            // Supposedly by this point I have constructed a map keyed by the
            // market, which returns a sub_map for each market. Each sub map
            // uses the key "BUY" or "SELL" and that points to the actual
            // offer data. (Like a Multimap.)
            //
            // Therefore we have sorted out all the buys and sells for each
            // market. Later on, we can loop through the main map, and for each
            // market, we can loop through all the buys and sells.
        }  // for (constructing the map_of_maps and all the sub_maps, so that
           // the
           // offers are sorted
           // by market and buy/sell status.
    }

    return map_of_maps;
}

std::int32_t output_nymoffer_data(
    const OTDB::OfferDataNym& offer_data,
    std::int32_t nIndex,
    const MapOfMaps&,
    const SubMap&,
    the_lambda_struct&)  // if 10 offers are printed for the
                         // SAME market, nIndex will be 0..9
{  // extra_vals unused in this function, but not in others that share this
    // parameter profile.
    // (It's used as a lambda.)

    std::string strScale = offer_data.scale;
    std::string strInstrumentDefinitionID = offer_data.instrument_definition_id;
    std::string strCurrencyTypeID = offer_data.currency_type_id;
    std::string strSellStatus = offer_data.selling ? "SELL" : "BUY";
    std::string strTransactionID = offer_data.transaction_id;
    std::string strAvailableAssets = std::to_string(
        std::stoll(offer_data.total_assets) -
        std::stoll(offer_data.finished_so_far));

    if (0 == nIndex)  // first iteration! (Output a header.)
    {
        otOut << "\nScale:\t\t" << strScale << "\n";
        otOut << "Asset:\t\t" << strInstrumentDefinitionID << "\n";
        otOut << "Currency:\t" << strCurrencyTypeID << "\n";
        otOut << "\nIndex\tTrans#\tType\tPrice\tAvailable\n";
    }

    //
    // Okay, we have the offer_data, so let's output it!
    //
    std::cout << (nIndex) << "\t" << offer_data.transaction_id << "\t"
              << strSellStatus << "\t" << offer_data.price_per_scale << "\t"
              << strAvailableAssets << "\n";

    return 1;
}

// If you have a buy offer, to buy silver for $30, and to sell silver for $35,
// what happens tomorrow when the market shifts, and you want to buy for $40
// and sell for $45 ?
//
// Well, now you need to cancel certain sell orders from yesterday! Because why
// on earth would you want to sell silver for $35 while buying it for $40?
// (knotwork raised ) That would be buy-high, sell-low.
//
// Any rational trader would cancel the old $35 sell order before placing a new
// $40 buy order!
//
// Similarly, if the market went DOWN such that my old offers were $40 buy / $45
// sell, and my new offers are going to be $30 buy / $35 sell, then I want to
// cancel certain buy orders for yesterday. After all, why on earth would you
// want to buy silver for $40 meanwhile putting up a new sell order at $35!
// You would immediately just turn around, after buying something, and sell it
// for LESS?
//
// Since the user would most likely be forced anyway to do this, for reasons of
// self-interest, it will probably end up as the default behavior here.
//

// RETURN VALUE: extra_vals will contain a list of offers that need to be
// removed AFTER

std::int32_t find_strange_offers(
    const OTDB::OfferDataNym& offer_data,
    const std::int32_t,
    const MapOfMaps&,
    const SubMap&,
    the_lambda_struct& extra_vals)  // if 10 offers are
                                    // printed
                                    // for the SAME market,
                                    // nIndex will be 0..9
{
    std::string strLocation = "find_strange_offers";
    /*
    me: How about this — when you do "opentxs newoffer" I can alter that
    script to automatically cancel any sell offers for a lower amount
    than my new buy offer, if they're on the same market at the same scale.
    and vice versa. Vice versa meaning, cancel any bid offers for a higher
    amount than my new sell offer.

    knotwork: yeah that would work.

    So when placing a buy offer, check all the other offers I already have at
    the same scale,
    same asset and currency ID. (That is, the same "market" as denoted by
    strMapKey in "opentxs showmyoffers")
    For each, see if it's a sell offer and if so, if the amount is lower than
    the amount on
    the new buy offer, then cancel that sell offer from the market. (Because I
    don't want to buy-high, sell low.)

    Similarly, if placing a sell offer, then check all the other offers I
    already have at the
    same scale, same asset and currency ID, (the same "market" as denoted by
    strMapKey....) For
    each, see if it's a buy offer and if so, if the amount is higher than the
    amount of my new
    sell offer, then cancel that buy offer from the market. (Because I don't
    want some old buy offer
    for $10 laying around for the same stock that I'm SELLING for $8! If I dump
    100 shares, I'll receive
    $800--I don't want my software to automatically turn around and BUY those
    same shares again for $1000!
    That would be a $200 loss.)

    This is done here. This function gets called once for each offer that's
    active for this Nym.
    extra_vals contains the relevant info we're looking for, and offer_data
    contains the current
    offer (as we loop through ALL this Nym's offers, this function gets called
    for each one.)
    So here we just need to compare once, and add to the list if the comparison
    matches.
    */
    /*
    attr the_lambda_struct::the_vector        // used for returning a list of
    something.
    attr the_lambda_struct::the_asset_acct    // for newoffer, we want to remove
    existing offers for the same accounts in certain cases.
    attr the_lambda_struct::the_currency_acct // for newoffer, we want to remove
    existing offers for the same accounts in certain cases.
    attr the_lambda_struct::the_scale         // for newoffer as well.
    attr the_lambda_struct::the_price         // for newoffer as well.
    attr the_lambda_struct::bSelling          // for newoffer as well.
    */
    otLog4 << strLocation << ": About to compare the new potential offer "
                             "against one of the existing ones...";

    if ((extra_vals.the_asset_acct == offer_data.asset_acct_id) &&
        (extra_vals.the_currency_acct == offer_data.currency_acct_id) &&
        (extra_vals.the_scale == offer_data.scale)) {
        otLog4 << strLocation << ": the account IDs and the scale match...";

        // By this point we know the current offer_data has the same asset acct,
        // currency acct, and scale
        // as the offer we're comparing to all the rest.
        //
        // But that's not enough: we also need to compare some prices:
        //
        // So when placing a buy offer, check all the other offers I already
        // have.
        // For each, see if it's a sell offer and if so, if the amount is lower
        // than the amount on
        // the new buy offer, then cancel that sell offer from the market.
        // (Because I don't want to buy-high, sell low.)
        //
        if (!extra_vals.bSelling && offer_data.selling &&
            (stoll(offer_data.price_per_scale) < stoll(extra_vals.the_price))) {
            extra_vals.the_vector.push_back(offer_data.transaction_id);
        }
        // Similarly, when placing a sell offer, check all the other offers I
        // already have.
        // For each, see if it's a buy offer and if so, if the amount is higher
        // than the amount of my new
        // sell offer, then cancel that buy offer from the market.
        //
        else if (
            extra_vals.bSelling && !offer_data.selling &&
            (stoll(offer_data.price_per_scale) > stoll(extra_vals.the_price))) {
            extra_vals.the_vector.push_back(offer_data.transaction_id);
        }
    }
    // We don't actually do the removing here, since we are still looping
    // through the maps.
    // So we just add the IDs to a vector so that the caller can do the removing
    // once this loop is over.

    return 1;
}

std::int32_t iterate_nymoffers_sub_map(
    const MapOfMaps& map_of_maps,
    SubMap& sub_map,
    LambdaFunc the_lambda)
{
    the_lambda_struct extra_vals;
    return iterate_nymoffers_sub_map(
        map_of_maps, sub_map, the_lambda, extra_vals);
}

// low level. map_of_maps and sub_map must be good. (assumed.)
//
// extra_vals allows you to pass any extra data you want std::into your
// lambda, for when it is called. (Like a functor.)
//
std::int32_t iterate_nymoffers_sub_map(
    const MapOfMaps& map_of_maps,
    SubMap& sub_map,
    LambdaFunc the_lambda,
    the_lambda_struct& extra_vals)
{
    // the_lambda must be good (assumed) and must have the parameter profile
    // like this sample:
    // def the_lambda(offer_data, nIndex, map_of_maps, sub_map, extra_vals)
    //
    // if 10 offers are printed for the SAME market, nIndex will be 0..9

    std::string strLocation = "iterate_nymoffers_sub_map";

    // Looping through the map_of_maps, we are now on a valid sub_map in this
    // iteration.
    // Therefore let's loop through the offers on that sub_map and output them!
    //
    // var range_sub_map = sub_map.range();

    SubMap* sub_mapPtr = &sub_map;
    if (!sub_mapPtr) {
        otOut << strLocation << ": No range retrieved from sub_map. It must be "
                                "non-existent, I guess.\n";
        return -1;
    }
    if (sub_map.empty()) {
        // Should never happen since we already made sure all the sub_maps
        // have data on them. Therefore if this range is empty now, it's a
        // chaiscript
        // bug (extremely unlikely.)
        //
        otOut << strLocation << ": Error: A range was retrieved for the "
                                "sub_map, but the range is empty.\n";
        return -1;
    }

    std::int32_t nIndex = -1;
    for (auto it = sub_map.begin(); it != sub_map.end(); ++it) {
        ++nIndex;
        // var offer_data_pair = range_sub_map.front();

        if (nullptr == it->second) {
            otOut << strLocation << ": Looping through range_sub_map range, "
                                    "and first offer_data_pair fails to "
                                    "verify.\n";
            return -1;
        }

        OTDB::OfferDataNym& offer_data = *it->second;
        std::int32_t nLambda = (*the_lambda)(
            offer_data,
            nIndex,
            map_of_maps,
            sub_map,
            extra_vals);  // if 10 offers are printed for the SAME
                          // market, nIndex will be 0..9;
        if (-1 == nLambda) {
            otOut << strLocation << ": Error: the_lambda failed.\n";
            return -1;
        }
    }
    sub_map.clear();

    return 1;
}

std::int32_t iterate_nymoffers_maps(
    MapOfMaps& map_of_maps,
    LambdaFunc the_lambda)  // low level. map_of_maps
                            // must be
                            // good. (assumed.)
{
    the_lambda_struct extra_vals;
    return iterate_nymoffers_maps(map_of_maps, the_lambda, extra_vals);
}

// extra_vals allows you to pass any extra data you want std::into your
// lambda, for when it is called. (Like a functor.)
//
std::int32_t iterate_nymoffers_maps(
    MapOfMaps& map_of_maps,
    LambdaFunc the_lambda,
    the_lambda_struct& extra_vals)  // low level.
                                    // map_of_maps
                                    // must be good.
                                    // (assumed.)
{
    // the_lambda must be good (assumed) and must have the parameter profile
    // like this sample:
    // def the_lambda(offer_data, nIndex, map_of_maps, sub_map, extra_vals)
    // //
    // if 10 offers are printed for the SAME market, nIndex will be 0..9

    std::string strLocation = "iterate_nymoffers_maps";

    // Next let's loop through the map_of_maps and output the offers for each
    // market therein...
    //
    // var range_map_of_maps = map_of_maps.range();
    MapOfMaps* map_of_mapsPtr = &map_of_maps;
    if (!map_of_mapsPtr) {
        otOut << strLocation << ": No range retrieved from map_of_maps.\n";
        return -1;
    }
    if (map_of_maps.empty()) {
        otOut << strLocation << ": A range was retrieved for the map_of_maps, "
                                "but the range is empty.\n";
        return -1;
    }

    for (auto it = map_of_maps.begin(); it != map_of_maps.end(); ++it) {
        // var sub_map_pair = range_map_of_maps.front();
        if (nullptr == it->second) {
            otOut << strLocation << ": Looping through map_of_maps range, and "
                                    "first sub_map_pair fails to verify.\n";
            return -1;
        }

        std::string strMapKey = it->first;

        SubMap& sub_map = *it->second;
        if (sub_map.empty()) {
            otOut << strLocation << ": Error: Sub_map is empty (Then how is it "
                                    "even here?? Submaps are only added based "
                                    "on existing offers.)\n";
            return -1;
        }

        std::int32_t nSubMap = iterate_nymoffers_sub_map(
            map_of_maps, sub_map, the_lambda, extra_vals);
        if (-1 == nSubMap) {
            otOut << strLocation
                  << ": Error: while trying to iterate_nymoffers_sub_map.\n";
            return -1;
        }
    }
    map_of_maps.clear();

    return 1;
}
}  // namespace opentxs
