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

#include "opentxs/client/OT_ME.hpp"

#include "opentxs/api/Api.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/OTAPI_Wrap.hpp"
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
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Func.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTDataFolder.hpp"
#include "opentxs/core/util/OTPaths.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/ext/Helpers.hpp"

#define OT_METHOD "opentxs::OT_ME::"

namespace opentxs
{

OT_ME::OT_ME(std::recursive_mutex& lock, MadeEasy& madeEasy)
    : lock_(lock)
    , made_easy_(madeEasy)
{
}

class OT_ME& OT_ME::It(const std::string wallet)
{
    return OT::App().API().OTME(wallet);
}

bool OT_ME::make_sure_enough_trans_nums(
    int32_t nNumberNeeded,
    const std::string& strMyNotaryID,
    const std::string& strMyNymID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(strMyNymID), Identifier(strMyNotaryID));
    Utility MsgUtil(context.It());
    bool bReturnVal = true;

    // Make sure we have at least one transaction number (to write the
    // cheque...)
    //
    std::int32_t nTransCount =
        OTAPI_Wrap::GetNym_TransactionNumCount(strMyNotaryID, strMyNymID);

    if (nTransCount < nNumberNeeded) {
        otOut << "insure_enough_nums: I don't have enough "
                 "transaction numbers. Grabbing more now...\n";

        MsgUtil.getTransactionNumbers(strMyNotaryID, strMyNymID, true);

        bool msgWasSent = false;
        if (0 > made_easy_.retrieve_nym(
                    strMyNotaryID, strMyNymID, msgWasSent, false)) {

            otOut << "Error: cannot retrieve nym.\n";
            return false;
        }

        // Try again.
        //
        nTransCount =
            OTAPI_Wrap::GetNym_TransactionNumCount(strMyNotaryID, strMyNymID);

        if (nTransCount < nNumberNeeded) {
            otOut
                << "insure_enough_nums: I still don't have enough transaction "
                   "numbers (I have "
                << nTransCount << ", but I need " << nNumberNeeded
                << ".)\n(Tried grabbing some, but failed somehow.)\n";
            return false;
        } else {
            bReturnVal = true;
        }
    }

    return bReturnVal;
}

/** Notify a nym of a pending blockchain deposit */
std::string OT_ME::notify_bailment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& TXID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        NOTIFY_BAILMENT,
        context.It(),
        TARGET_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        TXID);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "NOTIFY_BAILMENT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

/** Request a deposit of some asset in exchange for an OT balance */
std::string OT_ME::initiate_bailment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        INITIATE_BAILMENT,
        context.It(),
        TARGET_NYM_ID,
        INSTRUMENT_DEFINITION_ID);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "INITIATE_BAILMENT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

/** Request a redemption of an OT balance for the underlying asset*/
std::string OT_ME::initiate_outbailment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::int64_t& AMOUNT,
    const std::string& THE_MESSAGE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        INITIATE_OUTBAILMENT,
        context.It(),
        TARGET_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        AMOUNT,
        THE_MESSAGE);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "INITIATE_OUTBAILMENT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::request_connection(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::int64_t TYPE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        REQUEST_CONNECTION, context.It(), TARGET_NYM_ID, TYPE);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REQUEST_CONNECTION");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::store_secret(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::int64_t TYPE,
    const std::string& PRIMARY,
    const std::string& SECONDARY) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        STORE_SECRET, context.It(), TARGET_NYM_ID, PRIMARY, SECONDARY, TYPE);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "STORE_SECRET");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

/** Respond to a bailment request with deposit instructions */
std::string OT_ME::acknowledge_bailment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& REQUEST_ID,
    const std::string& THE_MESSAGE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        ACKNOWLEDGE_BAILMENT,
        context.It(),
        TARGET_NYM_ID,
        REQUEST_ID,
        THE_MESSAGE);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "ACKNOWLEDGE_BAILMENT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        OTAPI_Wrap::completePeerReply(NYM_ID, REQUEST_ID);
    }

    return strResponse;
}

/** Respond to an outbailment request with withdrawal instructions */
std::string OT_ME::acknowledge_outbailment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& REQUEST_ID,
    const std::string& THE_MESSAGE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        ACKNOWLEDGE_OUTBAILMENT,
        context.It(),
        TARGET_NYM_ID,
        REQUEST_ID,
        THE_MESSAGE);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "ACKNOWLEDGE_OUTBAILMENT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        OTAPI_Wrap::completePeerReply(NYM_ID, REQUEST_ID);
    }

    return strResponse;
}

/** Acknowledge a peer notice */
std::string OT_ME::acknowledge_notice(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& REQUEST_ID,
    const bool ACK) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        ACKNOWLEDGE_NOTICE, context.It(), TARGET_NYM_ID, REQUEST_ID, ACK);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "ACKNOWLEDGE_NOTICE");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        OTAPI_Wrap::completePeerReply(NYM_ID, REQUEST_ID);
    }

    return strResponse;
}

/** Acknowledge a connection info request */
std::string OT_ME::acknowledge_connection(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& REQUEST_ID,
    const bool ACK,
    const std::string& URL,
    const std::string& LOGIN,
    const std::string& PASSWORD,
    const std::string& KEY) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        ACKNOWLEDGE_CONNECTION,
        context.It(),
        TARGET_NYM_ID,
        REQUEST_ID,
        URL,
        LOGIN,
        PASSWORD,
        KEY,
        ACK);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "ACKNOWLEDGE_CONNECTION");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    } else {
        OTAPI_Wrap::completePeerReply(NYM_ID, REQUEST_ID);
    }

    return strResponse;
}

std::string OT_ME::register_contract_nym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(REGISTER_CONTRACT_NYM, context.It(), CONTRACT);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REGISTER_CONTRACT_NYM");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::register_contract_server(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(REGISTER_CONTRACT_SERVER, context.It(), CONTRACT);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REGISTER_CONTRACT_SERVER");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::register_contract_unit(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(REGISTER_CONTRACT_UNIT, context.It(), CONTRACT);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REGISTER_CONTRACT_UNIT");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// REGISTER NYM AT SERVER (or download nymfile, if nym already registered.)
//
std::string OT_ME::register_nym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(REGISTER_NYM, context.It());
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REGISTER_NYM");

    if (1 == VerifyMessageSuccess(strResponse)) {
        // Use the getRequestNumber command, thus insuring that the request
        // number is in sync.
        if (0 >= context.It().UpdateRequestNumber()) {
            otOut << OT_METHOD << __FUNCTION__ << ": Registered nym " << NYM_ID
                  << " on notary " << NOTARY_ID
                  << " but failed to obtain request number." << std::endl;
        }
    } else {
        // maybe an invalid server ID or the server contract isn't available
        // (do AddServerContract(..) first)
        otOut << OT_METHOD << __FUNCTION__ << "Failed to register nym "
              << NYM_ID << " on notary " << NOTARY_ID << std::endl;
    }

    return strResponse;
}

std::string OT_ME::request_admin(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& PASSWORD) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(REQUEST_ADMIN, context.It(), PASSWORD);
    std::string strResponse =
        theRequest.SendRequest(theRequest, "REQUEST_ADMIN");
    std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

std::string OT_ME::server_add_claim(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& SECTION,
    const std::string& TYPE,
    const std::string& VALUE,
    const bool PRIMARY) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func theRequest(
        SERVER_ADD_CLAIM, context.It(), PRIMARY, SECTION, TYPE, VALUE);
    const std::string strResponse =
        theRequest.SendRequest(theRequest, "SERVER_ADD_CLAIM");
    const std::int32_t nSuccess = VerifyMessageSuccess(strResponse);

    if (1 != nSuccess) {
        otOut << "Failed to " << __FUNCTION__ << "." << std::endl;
    }

    return strResponse;
}

// CHECK USER (download a public key)
//
std::string OT_ME::check_nym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID) const
{
    return made_easy_.check_nym(NOTARY_ID, NYM_ID, TARGET_NYM_ID);
}

// PING NOTARY
//
std::string OT_ME::ping_notary(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    const auto response = context.It().PingNotary();
    const auto& reply = response.second;

    if (false == bool(reply)) {

        return {};
    }

    return String(*reply).Get();
}

// ISSUE ASSET TYPE
//
std::string OT_ME::issue_asset_type(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_CONTRACT) const
{
    return made_easy_.issue_asset_type(NOTARY_ID, NYM_ID, THE_CONTRACT);
}

// ISSUE BASKET CURRENCY
//
std::string OT_ME::issue_basket_currency(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_BASKET) const
{
    return made_easy_.issue_basket_currency(NOTARY_ID, NYM_ID, THE_BASKET);
}

// EXCHANGE BASKET CURRENCY
//
std::string OT_ME::exchange_basket_currency(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& THE_BASKET,
    const std::string& ACCOUNT_ID,
    bool IN_OR_OUT) const
{
    return made_easy_.exchange_basket_currency(
        INSTRUMENT_DEFINITION_ID,
        NOTARY_ID,
        NYM_ID,
        THE_BASKET,
        ACCOUNT_ID,
        IN_OR_OUT);
}

// RETRIEVE CONTRACT
//
std::string OT_ME::retrieve_contract(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT_ID) const
{
    return made_easy_.retrieve_contract(NOTARY_ID, NYM_ID, CONTRACT_ID);
}

// LOAD OR RETRIEVE CONTRACT
//
std::string OT_ME::load_or_retrieve_contract(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& CONTRACT_ID) const
{
    return made_easy_.load_or_retrieve_contract(NOTARY_ID, NYM_ID, CONTRACT_ID);
}

// CREATE ASSET ACCOUNT
//
std::string OT_ME::create_asset_acct(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    return made_easy_.create_asset_acct(
        NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
}

// DELETE ASSET ACCOUNT
//
std::string OT_ME::unregister_account(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID) const
{
    return made_easy_.unregister_account(NOTARY_ID, NYM_ID, ACCOUNT_ID);
}

// UNREGISTER NYM FROM SERVER
//
std::string OT_ME::unregister_nym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    return made_easy_.unregister_nym(NOTARY_ID, NYM_ID);
}

std::string OT_ME::stat_asset_account(const std::string& ACCOUNT_ID) const
{
    return made_easy_.stat_asset_account(ACCOUNT_ID);
}

// DOWNLOAD ACCOUNT FILES (account balance, inbox, outbox, etc)
//
bool OT_ME::retrieve_account(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    bool bForceDownload) const
{
    return made_easy_.retrieve_account(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, bForceDownload);
}

bool OT_ME::retrieve_nym(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    bool bForceDownload) const
{
    bool msgWasSent = false;
    if (0 > made_easy_.retrieve_nym(
                NOTARY_ID, NYM_ID, msgWasSent, bForceDownload)) {
        otOut << "Error: cannot retrieve nym.\n";
        return false;
    }

    return true;
}

// SEND TRANSFER -- TRANSACTION
//
std::string OT_ME::send_transfer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_FROM,
    const std::string& ACCT_TO,
    std::int64_t AMOUNT,
    const std::string& NOTE) const
{
    return made_easy_.send_transfer(
        NOTARY_ID, NYM_ID, ACCT_FROM, ACCT_TO, AMOUNT, NOTE);
}

// PROCESS INBOX -- TRANSACTION
//
std::string OT_ME::process_inbox(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& RESPONSE_LEDGER) const
{
    return made_easy_.process_inbox(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, RESPONSE_LEDGER);
}

bool OT_ME::accept_inbox_items(
    const std::string& ACCOUNT_ID,
    std::int32_t nItemType,
    const std::string& INDICES) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

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

bool OT_ME::discard_incoming_payments(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INDICES) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdDiscard discard;
    return 1 == discard.run(NOTARY_ID, NYM_ID, INDICES);
}

bool OT_ME::cancel_outgoing_payments(
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& INDICES) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdCancel cancel;
    return 1 == cancel.run(NYM_ID, ACCOUNT_ID, INDICES);
}

bool OT_ME::accept_from_paymentbox(
    const std::string& ACCOUNT_ID,
    const std::string& INDICES,
    const std::string& PAYMENT_TYPE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdAcceptPayments cmd;
    return 1 == cmd.acceptFromPaymentbox(ACCOUNT_ID, INDICES, PAYMENT_TYPE);
}

bool OT_ME::accept_from_paymentbox_overload(
    const std::string& ACCOUNT_ID,
    const std::string& INDICES,
    const std::string& PAYMENT_TYPE,
    std::string* pOptionalOutput /*=nullptr*/) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdAcceptPayments cmd;
    return 1 ==
           cmd.acceptFromPaymentbox(
               ACCOUNT_ID, INDICES, PAYMENT_TYPE, pOptionalOutput);
}

// load_public_key():
//
// Load a public key from local storage, and return it (or null).
//
std::string OT_ME::load_public_encryption_key(const std::string& NYM_ID) const
{
    return made_easy_.load_public_encryption_key(NYM_ID);
}

std::string OT_ME::load_public_signing_key(const std::string& NYM_ID) const
{
    return made_easy_.load_public_signing_key(NYM_ID);
}

//
// load_or_retrieve_pubkey()
//
// Load TARGET_NYM_ID from local storage.
// If not there, then retrieve TARGET_NYM_ID from server,
// using NYM_ID to send check_nym request. Then re-load
// and return. (Might still return null.)
//
std::string OT_ME::load_or_retrieve_encrypt_key(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& TARGET_NYM_ID) const
{
    return made_easy_.load_or_retrieve_encrypt_key(
        NOTARY_ID, NYM_ID, TARGET_NYM_ID);
}

// SEND USER MESSAGE (requires recipient public key)
//
std::string OT_ME::send_user_msg_pubkey(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_MESSAGE) const
{
    return made_easy_.send_user_msg_pubkey(
        NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID, RECIPIENT_PUBKEY, THE_MESSAGE);
}

// SEND USER INSTRUMENT (requires recipient public key)
//
std::string OT_ME::send_user_pmnt_pubkey(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_INSTRUMENT) const
{
    return made_easy_.send_user_pmnt_pubkey(
        NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID, RECIPIENT_PUBKEY, THE_INSTRUMENT);
}

// SEND USER CASH (requires recipient public key)
//
std::string OT_ME::send_user_cash_pubkey(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& RECIPIENT_PUBKEY,
    const std::string& THE_INSTRUMENT,
    const std::string& INSTRUMENT_FOR_SENDER) const
{
    return made_easy_.send_user_cash_pubkey(
        NOTARY_ID,
        NYM_ID,
        RECIPIENT_NYM_ID,
        RECIPIENT_PUBKEY,
        THE_INSTRUMENT,
        INSTRUMENT_FOR_SENDER);
}

// SEND USER MESSAGE (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_msg(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& THE_MESSAGE) const
{
    return made_easy_.send_user_msg(
        NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID, THE_MESSAGE);
}

// SEND USER PAYMENT (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_payment(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& THE_PAYMENT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    std::string strRecipientPubkey =
        load_or_retrieve_encrypt_key(NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID);

    if (!VerifyStringVal(strRecipientPubkey)) {
        otOut << "OT_ME_send_user_payment: Unable to load or "
                 "retrieve public encryption key for recipient: "
              << RECIPIENT_NYM_ID << "\n";
        return strRecipientPubkey;  // basically this means "return null".
    }

    return send_user_pmnt_pubkey(
        NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID, strRecipientPubkey, THE_PAYMENT);
}

// SEND USER CASH (only requires recipient's ID, and retrieves pubkey
// automatically)
//
std::string OT_ME::send_user_cash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& RECIPIENT_NYM_ID,
    const std::string& THE_PAYMENT,
    const std::string& SENDERS_COPY) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    std::string strRecipientPubkey =
        load_or_retrieve_encrypt_key(NOTARY_ID, NYM_ID, RECIPIENT_NYM_ID);

    if (!VerifyStringVal(strRecipientPubkey)) {
        otOut << "OT_ME_send_user_payment: Unable to load or "
                 "retrieve public encryption key for recipient: "
              << RECIPIENT_NYM_ID << "\n";
        return strRecipientPubkey;  // basically this means "return null".
    }

    return send_user_cash_pubkey(
        RECIPIENT_NYM_ID,
        NOTARY_ID,
        NYM_ID,
        strRecipientPubkey,
        THE_PAYMENT,
        SENDERS_COPY);
}

bool OT_ME::withdraw_and_send_cash(
    const std::string& ACCT_ID,
    const std::string& RECIPIENT_NYM_ID,
    std::int64_t AMOUNT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdSendCash sendCash;
    return 1 ==
           sendCash.run(
               "",
               "",
               ACCT_ID,
               "",
               RECIPIENT_NYM_ID,
               std::to_string(AMOUNT),
               "",
               "");
}

// GET PAYMENT INSTRUMENT (from payments inbox, by index.)
//
std::string OT_ME::get_payment_instrument(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    std::int32_t nIndex,
    const std::string& PRELOADED_INBOX) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    std::string strInstrument;
    std::string strInbox =
        VerifyStringVal(PRELOADED_INBOX)
            ? PRELOADED_INBOX
            : OTAPI_Wrap::LoadPaymentInbox(
                  NOTARY_ID, NYM_ID);  // Returns nullptr, or an inbox.

    if (!VerifyStringVal(strInbox)) {
        otWarn << "\n\n get_payment_instrument:  "
                  "OT_API_LoadPaymentInbox Failed. (Probably just "
                  "doesn't exist yet.)\n\n";
        return "";
    }

    std::int32_t nCount =
        OTAPI_Wrap::Ledger_GetCount(NOTARY_ID, NYM_ID, NYM_ID, strInbox);
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

    strInstrument = OTAPI_Wrap::Ledger_GetInstrument(
        NOTARY_ID, NYM_ID, NYM_ID, strInbox, nIndex);
    if (!VerifyStringVal(strInstrument)) {
        otOut << "Failed trying to get payment instrument from payments box.\n";
        return "";
    }

    return strInstrument;
}

// GET BOX RECEIPT
// Note: nBoxType is 0 for Nymbox, 1 for Inbox, and 2 for Outbox.
// Also, if nBoxType is 0 (nymbox) then you have to pass the NymID in the
// ACCT_ID
// argument, as well as the NYM_ID argument (you have to pass it twice...)
// Otherwise for inbox/outbox, pass the actual ACCT_ID there as normal.
//
std::string OT_ME::get_box_receipt(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    std::int32_t nBoxType,
    std::int64_t TRANS_NUM) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        GET_BOX_RECEIPT,
        context.It(),
        ACCT_ID,
        std::to_string(nBoxType),
        std::to_string(TRANS_NUM));
    return request.SendRequest(request, "GET_BOX_RECEIPT");
}

// DOWNLOAD PUBLIC MINT
//
std::string OT_ME::retrieve_mint(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    return made_easy_.retrieve_mint(
        NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
}

// LOAD MINT (from local storage)
//
// To load a mint withOUT retrieving it from server, call:
//
// var strMint = OT_API_LoadMint(NOTARY_ID, INSTRUMENT_DEFINITION_ID);
// It returns the mint, or null.
// LOAD MINT (from local storage).
// Also, if necessary, RETRIEVE it from the server first.
//
// Returns the mint, or null.
//
std::string OT_ME::load_or_retrieve_mint(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID) const
{
    return made_easy_.load_or_retrieve_mint(
        NOTARY_ID, NYM_ID, INSTRUMENT_DEFINITION_ID);
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
    std::lock_guard<std::recursive_mutex> lock(lock_);

    std::string strNotaryID =
        OTAPI_Wrap::GetAccountWallet_NotaryID(ASSET_ACCT_ID);
    std::string strNymID = OTAPI_Wrap::GetAccountWallet_NymID(ASSET_ACCT_ID);
    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(strNymID), Identifier(strNotaryID));
    OTAPI_Func request(
        CREATE_MARKET_OFFER,
        context.It(),
        ASSET_ACCT_ID,
        CURRENCY_ACCT_ID,
        std::to_string(scale),
        std::to_string(minIncrement),
        std::to_string(quantity),
        std::to_string(price),
        bSelling);
    // Cannot have more than 10 parameters in a function call, in this script.
    // So I am forced to set the final parameters by hand, before sending the
    // transaction:
    //
    request.tData = OTTimeGetTimeFromSeconds(lLifespanInSeconds);
    request.lData = ACTIVATION_PRICE;
    if (VerifyStringVal(STOP_SIGN)) {
        request.strData5 = STOP_SIGN;
    }

    return request.SendTransaction(request, "CREATE_MARKET_OFFER");
}

// KILL MARKET OFFER -- TRANSACTION
//
std::string OT_ME::kill_market_offer(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ASSET_ACCT_ID,
    std::int64_t TRANS_NUM) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        KILL_MARKET_OFFER,
        context.It(),
        ASSET_ACCT_ID,
        std::to_string(TRANS_NUM));
    return request.SendTransaction(request, "KILL_MARKET_OFFER");
}

// KILL (ACTIVE) PAYMENT PLAN -- TRANSACTION
//
std::string OT_ME::kill_payment_plan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    std::int64_t TRANS_NUM) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        KILL_PAYMENT_PLAN, context.It(), ACCT_ID, std::to_string(TRANS_NUM));
    return request.SendTransaction(request, "KILL_PAYMENT_PLAN");
}

// CANCEL (NOT-YET-RUNNING) PAYMENT PLAN -- TRANSACTION
//
std::string OT_ME::cancel_payment_plan(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& THE_PAYMENT_PLAN) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    // NOTE: We have to include the account ID as well. Even though the API call
    // itself doesn't need it (it retrieves it from the plan itself, as we are
    // about to do here) we still have to provide the accountID for OTAPI_Func,
    // which uses it to grab the intermediary files, as part of its automated
    // sync duties. (FYI.)

    std::string strRecipientAcctID =
        OTAPI_Wrap::Instrmnt_GetRecipientAcctID(THE_PAYMENT_PLAN);

    // NOTE: Normally the SENDER (PAYER) is the one who deposits a payment plan.
    // But in this case, the RECIPIENT (PAYEE) deposits it -- which means
    // "Please cancel this plan." It SHOULD fail, since it's only been signed
    // by the recipient, and not the sender. And that failure is what burns
    // the transaction number on the plan, so that it can no longer be used.
    //
    // So how do we know the difference between an ACTUAL "failure" versus a
    // purposeful "failure" ?
    // Because if the failure comes from cancelling the plan, the server reply
    // transaction will have IsCancelled() set to true.
    //
    // (Therefore theRequest.SendTransaction is smart enough to check for that.)

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        DEPOSIT_PAYMENT_PLAN,
        context.It(),
        strRecipientAcctID,
        THE_PAYMENT_PLAN);
    return request.SendTransaction(request, "CANCEL_PAYMENT_PLAN");
}

// ACTIVATE SMART CONTRACT -- TRANSACTION
//
std::string OT_ME::activate_smart_contract(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& AGENT_NAME,
    const std::string& THE_SMART_CONTRACT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        ACTIVATE_SMART_CONTRACT,
        context.It(),
        ACCT_ID,
        AGENT_NAME,
        THE_SMART_CONTRACT);
    return request.SendTransaction(request, "ACTIVATE_SMART_CONTRACT");
}

// TRIGGER CLAUSE (on running smart contract) -- TRANSACTION
//
std::string OT_ME::trigger_clause(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    std::int64_t TRANS_NUM,
    const std::string& CLAUSE_NAME,
    const std::string& STR_PARAM) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        TRIGGER_CLAUSE,
        context.It(),
        std::to_string(TRANS_NUM),
        CLAUSE_NAME,
        STR_PARAM);
    return request.SendRequest(request, "TRIGGER_CLAUSE");
}

// WITHDRAW CASH -- TRANSACTION
//
std::string OT_ME::withdraw_cash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    std::int64_t AMOUNT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(WITHDRAW_CASH, context.It(), ACCT_ID, AMOUNT);
    return request.SendTransaction(request, "WITHDRAW_CASH");
}

// Difference between this function and the one above?
// This one automatically retrieves the mint beforehand, if necessary,
// and the account files afterward, if appropriate.
//
bool OT_ME::easy_withdraw_cash(const std::string& ACCT_ID, std::int64_t AMOUNT)
    const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdWithdrawCash cmd;
    return 1 == cmd.withdrawCash(ACCT_ID, AMOUNT);
}

// EXPORT CASH (FROM PURSE)
//
std::string OT_ME::export_cash(
    const std::string& NOTARY_ID,
    const std::string& FROM_NYM_ID,
    const std::string& INSTRUMENT_DEFINITION_ID,
    const std::string& TO_NYM_ID,
    const std::string& STR_INDICES,
    bool bPasswordProtected,
    std::string& STR_RETAINED_COPY) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    std::string to_nym_id = TO_NYM_ID;
    CmdExportCash cmd;
    return cmd.exportCash(
        NOTARY_ID,
        FROM_NYM_ID,
        INSTRUMENT_DEFINITION_ID,
        to_nym_id,
        STR_INDICES,
        bPasswordProtected,
        STR_RETAINED_COPY);
}

// WITHDRAW VOUCHER -- TRANSACTION
//
std::string OT_ME::withdraw_voucher(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& RECIP_NYM_ID,
    const std::string& STR_MEMO,
    std::int64_t AMOUNT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        WITHDRAW_VOUCHER,
        context.It(),
        ACCT_ID,
        RECIP_NYM_ID,
        STR_MEMO,
        AMOUNT);
    return request.SendTransaction(request, "WITHDRAW_VOUCHER");
}

// PAY DIVIDEND -- TRANSACTION
//
std::string OT_ME::pay_dividend(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& SOURCE_ACCT_ID,
    const std::string& SHARES_INSTRUMENT_DEFINITION_ID,
    const std::string& STR_MEMO,
    std::int64_t AMOUNT_PER_SHARE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        PAY_DIVIDEND,
        context.It(),
        SOURCE_ACCT_ID,
        SHARES_INSTRUMENT_DEFINITION_ID,
        STR_MEMO,
        AMOUNT_PER_SHARE);
    return request.SendTransaction(request, "PAY_DIVIDEND");
}

std::string OT_ME::deposit_cheque(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& STR_CHEQUE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(DEPOSIT_CHEQUE, context.It(), ACCT_ID, STR_CHEQUE);
    return request.SendTransaction(request, "DEPOSIT_CHEQUE");
}

bool OT_ME::deposit_cash(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& STR_PURSE) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(NOTARY_ID, ACCT_ID, NYM_ID, STR_PURSE, "");
}

bool OT_ME::deposit_local_purse(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCT_ID,
    const std::string& STR_INDICES) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    CmdDeposit cmd;
    return 1 == cmd.depositPurse(NOTARY_ID, ACCT_ID, NYM_ID, "", STR_INDICES);
}

std::string OT_ME::get_market_list(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(GET_MARKET_LIST, context.It());
    return request.SendRequest(request, "GET_MARKET_LIST");
}

std::string OT_ME::get_market_offers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID,
    std::int64_t MAX_DEPTH) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(GET_MARKET_OFFERS, context.It(), MARKET_ID, MAX_DEPTH);
    return request.SendRequest(request, "GET_MARKET_OFFERS");
}

std::string OT_ME::get_nym_market_offers(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(GET_NYM_MARKET_OFFERS, context.It());
    return request.SendRequest(request, "GET_NYM_MARKET_OFFERS");
}

std::string OT_ME::get_market_recent_trades(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& MARKET_ID) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(GET_MARKET_RECENT_TRADES, context.It(), MARKET_ID);
    return request.SendRequest(request, "GET_MARKET_RECENT_TRADES");
}

std::string OT_ME::adjust_usage_credits(
    const std::string& NOTARY_ID,
    const std::string& USER_NYM_ID,
    const std::string& TARGET_NYM_ID,
    const std::string& ADJUSTMENT) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    auto context = OT::App().Contract().mutable_ServerContext(
        Identifier(USER_NYM_ID), Identifier(NOTARY_ID));
    OTAPI_Func request(
        ADJUST_USAGE_CREDITS, context.It(), TARGET_NYM_ID, ADJUSTMENT);
    return request.SendRequest(request, "ADJUST_USAGE_CREDITS");
}

int32_t OT_ME::VerifyMessageSuccess(const std::string& str_Message) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = OTAPI_Wrap::Message_GetSuccess(str_Message);

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

int32_t OT_ME::VerifyMsgBalanceAgrmntSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Message) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = OTAPI_Wrap::Message_GetBalanceAgreementSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, str_Message);

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

int32_t OT_ME::VerifyMsgTrnxSuccess(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Message) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

    if (str_Message.size() < 10) {
        otWarn << __FUNCTION__ << ": Error str_Message is: Too Short: \n"
               << str_Message << "\n\n";
        return -1;
    }

    std::int32_t nStatus = OTAPI_Wrap::Message_GetTransactionSuccess(
        NOTARY_ID, NYM_ID, ACCOUNT_ID, str_Message);

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

// This code was repeating a lot, so I just added a function for it.
//
// It uses the above functions.
//
int32_t OT_ME::InterpretTransactionMsgReply(
    const std::string& NOTARY_ID,
    const std::string& NYM_ID,
    const std::string& ACCOUNT_ID,
    const std::string& str_Attempt,
    const std::string& str_Response) const
{
    std::lock_guard<std::recursive_mutex> lock(lock_);

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
        NOTARY_ID, NYM_ID, ACCOUNT_ID, str_Response);

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
        VerifyMsgTrnxSuccess(NOTARY_ID, NYM_ID, ACCOUNT_ID, str_Response);

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
}  // namespace opentxs
