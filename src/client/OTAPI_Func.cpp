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

#include "opentxs/client/OTAPI_Func.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <stdint.h>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#define OT_METHOD "opentxs::OTAPI_Func::"

namespace opentxs
{
const std::map<OTAPI_Func_Type, std::string> OTAPI_Func::type_name_{
    {NO_FUNC, "NO_FUNC"},
    {REMOVED1, "REMOVED1"},
    {REGISTER_NYM, "REGISTER_NYM"},
    {DELETE_NYM, "DELETE_NYM"},
    {CHECK_NYM, "CHECK_NYM"},
    {SEND_USER_MESSAGE, "SEND_USER_MESSAGE"},
    {SEND_USER_INSTRUMENT, "SEND_USER_INSTRUMENT"},
    {ISSUE_ASSET_TYPE, "ISSUE_ASSET_TYPE"},
    {ISSUE_BASKET, "ISSUE_BASKET"},
    {CREATE_ASSET_ACCT, "CREATE_ASSET_ACCT"},
    {DELETE_ASSET_ACCT, "DELETE_ASSET_ACCT"},
    {ACTIVATE_SMART_CONTRACT, "ACTIVATE_SMART_CONTRACT"},
    {TRIGGER_CLAUSE, "TRIGGER_CLAUSE"},
    {PROCESS_INBOX, "PROCESS_INBOX"},
    {EXCHANGE_BASKET, "EXCHANGE_BASKET"},
    {DEPOSIT_CASH, "DEPOSIT_CASH"},
    {EXCHANGE_CASH, "EXCHANGE_CASH"},
    {DEPOSIT_CHEQUE, "DEPOSIT_CHEQUE"},
    {WITHDRAW_VOUCHER, "WITHDRAW_VOUCHER"},
    {PAY_DIVIDEND, "PAY_DIVIDEND"},
    {WITHDRAW_CASH, "WITHDRAW_CASH"},
    {GET_CONTRACT, "GET_CONTRACT"},
    {SEND_TRANSFER, "SEND_TRANSFER"},
    {GET_MARKET_LIST, "GET_MARKET_LIST"},
    {CREATE_MARKET_OFFER, "CREATE_MARKET_OFFER"},
    {KILL_MARKET_OFFER, "KILL_MARKET_OFFER"},
    {KILL_PAYMENT_PLAN, "KILL_PAYMENT_PLAN"},
    {DEPOSIT_PAYMENT_PLAN, "DEPOSIT_PAYMENT_PLAN"},
    {GET_NYM_MARKET_OFFERS, "GET_NYM_MARKET_OFFERS"},
    {GET_MARKET_OFFERS, "GET_MARKET_OFFERS"},
    {GET_MARKET_RECENT_TRADES, "GET_MARKET_RECENT_TRADES"},
    {GET_MINT, "GET_MINT"},
    {GET_BOX_RECEIPT, "GET_BOX_RECEIPT"},
    {ADJUST_USAGE_CREDITS, "ADJUST_USAGE_CREDITS"},
    {INITIATE_BAILMENT, "INITIATE_BAILMENT"},
    {INITIATE_OUTBAILMENT, "INITIATE_OUTBAILMENT"},
    {ACKNOWLEDGE_BAILMENT, "ACKNOWLEDGE_BAILMENT"},
    {ACKNOWLEDGE_OUTBAILMENT, "ACKNOWLEDGE_OUTBAILMENT"},
    {NOTIFY_BAILMENT, "NOTIFY_BAILMENT"},
    {ACKNOWLEDGE_NOTICE, "ACKNOWLEDGE_NOTICE"},
    {REQUEST_CONNECTION, "REQUEST_CONNECTION"},
    {ACKNOWLEDGE_CONNECTION, "ACKNOWLEDGE_CONNECTION"},
    {REGISTER_CONTRACT_NYM, "REGISTER_CONTRACT_NYM"},
    {REGISTER_CONTRACT_SERVER, "REGISTER_CONTRACT_SERVER"},
    {REGISTER_CONTRACT_UNIT, "REGISTER_CONTRACT_UNIT"},
    {REQUEST_ADMIN, "REQUEST_ADMIN"},
    {SERVER_ADD_CLAIM, "SERVER_ADD_CLAIM"},
    {STORE_SECRET, "STORE_SECRET"},
};

const std::map<OTAPI_Func_Type, bool> OTAPI_Func::type_type_{
    {REGISTER_NYM, false},
    {DELETE_NYM, false},
    {CHECK_NYM, false},
    {SEND_USER_MESSAGE, false},
    {SEND_USER_INSTRUMENT, false},
    {ISSUE_ASSET_TYPE, false},
    {ISSUE_BASKET, false},
    {CREATE_ASSET_ACCT, false},
    {DELETE_ASSET_ACCT, false},
    {ACTIVATE_SMART_CONTRACT, true},
    {TRIGGER_CLAUSE, false},
    {PROCESS_INBOX, true},
    {EXCHANGE_BASKET, true},
    {DEPOSIT_CASH, true},
    {EXCHANGE_CASH, true},
    {DEPOSIT_CHEQUE, true},
    {WITHDRAW_VOUCHER, true},
    {PAY_DIVIDEND, true},
    {WITHDRAW_CASH, true},
    {GET_CONTRACT, false},
    {SEND_TRANSFER, true},
    {GET_MARKET_LIST, false},
    {CREATE_MARKET_OFFER, true},
    {KILL_MARKET_OFFER, true},
    {KILL_PAYMENT_PLAN, true},
    {DEPOSIT_PAYMENT_PLAN, true},
    {GET_NYM_MARKET_OFFERS, false},
    {GET_MARKET_OFFERS, false},
    {GET_MARKET_RECENT_TRADES, false},
    {GET_MINT, false},
    {GET_BOX_RECEIPT, false},
    {ADJUST_USAGE_CREDITS, false},
    {INITIATE_BAILMENT, false},
    {INITIATE_OUTBAILMENT, false},
    {ACKNOWLEDGE_BAILMENT, false},
    {ACKNOWLEDGE_OUTBAILMENT, false},
    {NOTIFY_BAILMENT, false},
    {ACKNOWLEDGE_NOTICE, false},
    {REQUEST_CONNECTION, false},
    {ACKNOWLEDGE_CONNECTION, false},
    {REGISTER_CONTRACT_NYM, false},
    {REGISTER_CONTRACT_SERVER, false},
    {REGISTER_CONTRACT_UNIT, false},
    {REQUEST_ADMIN, false},
    {SERVER_ADD_CLAIM, false},
    {STORE_SECRET, false},
};

OTAPI_Func::OTAPI_Func(
    const api::client::Wallet& wallet,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Func_Type type)
    : type_(type)
    , nymID2("")
    , instrumentDefinitionID("")
    , instrumentDefinitionID2("")
    , accountID("")
    , accountID2("")
    , basket("")
    , strData("")
    , strData2("")
    , strData3("")
    , strData4("")
    , strData5("")
    , bBool(false)
    , nData(0)
    , lData(0)
    , tData(OT_TIME_ZERO)
    , nTransNumsNeeded(0)
    , nRequestNum(-1)
    , wallet_(wallet)
    , context_editor_(wallet_.mutable_ServerContext(nymID, serverID))
    , context_(context_editor_.It())
    , exec_(exec)
    , otapi_(otapi)
    , lock_()
    , last_attempt_()
    , is_transaction_(type_type_.at(type))
    , peer_reply_(nullptr)
    , peer_request_(nullptr)
{
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    if (theType == DELETE_NYM) {
        nTransNumsNeeded = 0;            // Is this true?
    } else if (theType == REGISTER_NYM)  // FYI.
    {
        nTransNumsNeeded = 0;
    } else if (theType == GET_MARKET_LIST)  // FYI
    {
        nTransNumsNeeded = 0;
    } else if (theType == GET_NYM_MARKET_OFFERS)  // FYI
    {
        nTransNumsNeeded = 0;
    } else {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_strParam)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    switch (theType) {
        case (ISSUE_BASKET): {
            basket = p_strParam;
        } break;
        case (CREATE_ASSET_ACCT): {
            nTransNumsNeeded = 1;  // So it's done at least one "transaction
                                   // statement" before it can ever processInbox
                                   // on an account.
            instrumentDefinitionID = p_strParam;
        } break;
        case (GET_MINT):
        case (GET_CONTRACT):
        case (REGISTER_CONTRACT_UNIT): {
            instrumentDefinitionID = p_strParam;
        } break;
        case (CHECK_NYM):
        case (REGISTER_CONTRACT_NYM): {
            nymID2 = p_strParam;
        } break;
        case (DELETE_ASSET_ACCT): {
            accountID = p_strParam;
        } break;
        case (ISSUE_ASSET_TYPE):
        case (GET_MARKET_RECENT_TRADES):
        case (REGISTER_CONTRACT_SERVER):
        case (REQUEST_ADMIN): {
            strData = p_strParam;
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()"
                  << std::endl;
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_strParam,
    const std::string& p_strData)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    if (!VerifyStringVal(p_strData)) {
        otErr << strError << "p_strData" << std::endl;
    }

    switch (theType) {
        case KILL_MARKET_OFFER:
        case KILL_PAYMENT_PLAN:
        case DEPOSIT_CASH:
        case DEPOSIT_CHEQUE:
        case DEPOSIT_PAYMENT_PLAN:
        case PROCESS_INBOX: {
            nTransNumsNeeded = 1;
            accountID = p_strParam;
            strData = p_strData;
        } break;
        case ADJUST_USAGE_CREDITS: {
            nymID2 = p_strParam;  // target nym ID
            strData = p_strData;  // adjustment (up or down.)
        } break;
        case EXCHANGE_CASH: {
            nTransNumsNeeded = 1;
            instrumentDefinitionID = p_strParam;
            strData = p_strData;
        } break;
        case INITIATE_BAILMENT: {
            nymID2 = p_strParam;
            instrumentDefinitionID = p_strData;
            peer_request_ = PeerRequest::Create(
                context_.Nym(),
                proto::PEERREQUEST_BAILMENT,
                Identifier(instrumentDefinitionID),
                context_.Server());

            OT_ASSERT(peer_request_);
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()\n";
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_strParam,
    std::int64_t p_lData)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    lData = p_lData;

    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    switch (theType) {
        case (WITHDRAW_CASH): {
            nTransNumsNeeded = 1;
            accountID = p_strParam;
        } break;
        case (GET_MARKET_OFFERS): {
            strData = p_strParam;
        } break;
        case (REQUEST_CONNECTION): {
            nymID2 = p_strParam;
            peer_request_ = PeerRequest::Create(
                context_.Nym(),
                proto::PEERREQUEST_CONNECTIONINFO,
                static_cast<proto::ConnectionInfoType>(lData),
                Identifier(nymID2),
                context_.Server());

            OT_ASSERT(peer_request_)
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()"
                  << std::endl;
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_nymID2,
    const std::string& p_strData,
    const bool p_Bool)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_nymID2)) {
        otErr << strError << "p_nymID2" << std::endl;
    }

    if (!VerifyStringVal(p_strData)) {
        otErr << strError << "p_strData" << std::endl;
    }

    if (theType == ACKNOWLEDGE_NOTICE) {
        nTransNumsNeeded = 0;
        instrumentDefinitionID = p_strData;
        nymID2 = p_nymID2;
        peer_reply_ = PeerReply::Create(
            context_.Nym(), Identifier(p_strData), context_.Server(), p_Bool);

        OT_ASSERT(peer_reply_)
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_nymID2,
    const std::string& p_strData,
    const std::string& p_strData2)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_nymID2)) {
        otErr << strError << "p_nymID2" << std::endl;
    }

    if (!VerifyStringVal(p_strData)) {
        otErr << strError << "p_strData" << std::endl;
    }

    if (!VerifyStringVal(p_strData2)) {
        otErr << strError << "p_strData2" << std::endl;
    }

    nTransNumsNeeded = 1;

    if ((theType == SEND_USER_MESSAGE) || (theType == SEND_USER_INSTRUMENT)) {
        nTransNumsNeeded = 0;
        nymID2 = p_nymID2;
        strData = p_strData;
        strData2 = p_strData2;
    } else if (theType == TRIGGER_CLAUSE) {
        strData = p_nymID2;
        strData2 = p_strData;
        strData3 = p_strData2;
    } else if (theType == ACTIVATE_SMART_CONTRACT) {

        accountID = p_nymID2;   // the "official" asset account of the party
                                // activating the contract.;
        strData = p_strData;    // the agent's name for that party, as listed on
                                // the contract.;
        strData2 = p_strData2;  // the smart contract itself.;

        std::int32_t nNumsNeeded =
            exec_.SmartContract_CountNumsNeeded(p_strData2, p_strData);

        if (nNumsNeeded > 0) {
            nTransNumsNeeded = nNumsNeeded;
        }
    } else if (theType == GET_BOX_RECEIPT) {
        nTransNumsNeeded = 0;
        accountID = p_nymID2;  // accountID (inbox/outbox) or NymID (nymbox) is
                               // passed here.;
        nData = stol(p_strData);
        strData = p_strData2;  // transaction number passed here as std::string;
    } else if (theType == ACKNOWLEDGE_BAILMENT) {
        nTransNumsNeeded = 0;
        nymID2 = p_nymID2;
        instrumentDefinitionID = p_strData;
        peer_reply_ = PeerReply::Create(
            context_.Nym(),
            proto::PEERREQUEST_BAILMENT,
            Identifier(p_strData),
            context_.Server(),
            p_strData2);

        OT_ASSERT(peer_reply_)
    } else if (theType == ACKNOWLEDGE_OUTBAILMENT) {
        nTransNumsNeeded = 0;
        nymID2 = p_nymID2;
        instrumentDefinitionID = p_strData;
        peer_reply_ = PeerReply::Create(
            context_.Nym(),
            proto::PEERREQUEST_OUTBAILMENT,
            Identifier(p_strData),
            context_.Server(),
            p_strData2);

        OT_ASSERT(peer_reply_)
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_accountID,
    const std::string& p_strParam,
    std::int64_t p_lData,
    const std::string& p_strData2)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_accountID)) {
        otErr << strError << "p_accountID" << std::endl;
    }

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    lData = p_lData;  // std::int64_t Amount;
    nTransNumsNeeded = 0;

    if (theType == SEND_TRANSFER) {
        if (!VerifyStringVal(p_strData2)) {
            otErr << strError << "p_strData2" << std::endl;
        }
        nTransNumsNeeded = 1;
        accountID = p_accountID;
        accountID2 = p_strParam;
        strData = p_strData2;
    } else if (theType == INITIATE_OUTBAILMENT) {
        nymID2 = p_accountID;
        instrumentDefinitionID = p_strParam;
        peer_request_ = PeerRequest::Create(
            context_.Nym(),
            proto::PEERREQUEST_OUTBAILMENT,
            Identifier(instrumentDefinitionID),
            context_.Server(),
            lData,
            p_strData2);

        OT_ASSERT(peer_request_)
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_accountID,
    const std::string& p_strParam,
    const std::string& p_strData,
    std::int64_t p_lData2)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    strData = p_strData;
    lData = p_lData2;
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_accountID)) {
        otErr << strError << "p_accountID" << std::endl;
    }

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    if (!VerifyStringVal(p_strData) && (STORE_SECRET != theType)) {
        otErr << strError << "p_strData" << std::endl;
    }

    switch (theType) {
        case (WITHDRAW_VOUCHER): {
            accountID = p_accountID;
            nymID2 = p_strParam;
        } break;
        case (PAY_DIVIDEND): {
            accountID = p_accountID;
            instrumentDefinitionID = p_strParam;
        } break;
        case (STORE_SECRET): {
            nTransNumsNeeded = 0;
            nymID2 = p_accountID;
            peer_request_ = PeerRequest::Create(
                context_.Nym(),
                proto::PEERREQUEST_STORESECRET,
                static_cast<proto::SecretType>(lData),
                Identifier(nymID2),
                p_strParam,
                strData,
                context_.Server());

            OT_ASSERT(peer_request_)
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                     "ERROR!!!!!!\n";
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_accountID,
    const std::string& p_strParam,
    const std::string& p_strData,
    const std::string& p_strData2)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_accountID)) {
        otErr << strError << "p_accountID" << std::endl;
    }

    if (!VerifyStringVal(p_strParam)) {
        otErr << strError << "p_strParam" << std::endl;
    }

    if (!VerifyStringVal(p_strData)) {
        otErr << strError << "p_strData" << std::endl;
    }

    nTransNumsNeeded = 1;
    accountID = p_accountID;

    if (theType == SEND_USER_INSTRUMENT) {
        nTransNumsNeeded = 0;
        nymID2 = p_accountID;  // Recipient Nym;
        strData = p_strParam;  // Recipient pubkey;
        strData2 = p_strData;  // Instrument for recipient.;
        accountID =
            p_strData2;  // sender_instrument is attached here. (Optional.);
    } else if (theType == NOTIFY_BAILMENT) {
        if (!VerifyStringVal(p_strData2)) {
            otErr << strError << "p_strData2" << std::endl;
        }
        nTransNumsNeeded = 0;
        nymID2 = p_accountID;
        instrumentDefinitionID = p_strParam;
        peer_request_ = PeerRequest::Create(
            context_.Nym(),
            proto::PEERREQUEST_PENDINGBAILMENT,
            Identifier(instrumentDefinitionID),
            context_.Server(),
            Identifier(nymID2),      // Recepient
            Identifier(p_strData2),  // Request ID
            p_strData);              // txid

        OT_ASSERT(peer_request_)
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    bool boolInput,
    const std::string& data,
    const std::string& data2,
    const std::string& data3)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    strData = data;
    strData2 = data2;
    strData3 = data3;
    bBool = boolInput;
    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(strData)) {
        otErr << strError << "strData" << std::endl;
    }

    if (!VerifyStringVal(strData2)) {
        otErr << strError << "strData2" << std::endl;
    }

    if (!VerifyStringVal(strData3)) {
        otErr << strError << "strData3" << std::endl;
    }

    switch (theType) {
        case (SERVER_ADD_CLAIM): {
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()"
                  << std::endl;
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& p_instrumentDefinitionID,
    const std::string& p_basket,
    const std::string& p_accountID,
    bool p_bBool,
    std::int32_t p_nTransNumsNeeded)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(p_instrumentDefinitionID)) {
        otErr << strError << "p_instrumentDefinitionID" << std::endl;
    }

    if (!VerifyStringVal(p_accountID)) {
        otErr << strError << "p_accountID" << std::endl;
    }

    if (!VerifyStringVal(p_basket)) {
        otErr << strError << "p_basket" << std::endl;
    }

    if (EXCHANGE_BASKET == theType) {
        // FYI. This is a transaction.
        nTransNumsNeeded = p_nTransNumsNeeded;
        bBool = p_bBool;
        instrumentDefinitionID = p_instrumentDefinitionID;
        basket = p_basket;
        accountID = p_accountID;
    } else {
        OT_FAIL
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& account,
    const std::string& account2,
    const std::string& data,
    const std::string& data2,
    const std::string& data3,
    const std::string& data4,
    bool boolInput)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    accountID = account;
    accountID2 = account2;
    strData = data;
    strData2 = data2;
    strData3 = data3;
    strData4 = data4;
    bBool = boolInput;
    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(accountID)) {
        otErr << strError << "accountID" << std::endl;
    }

    if (!VerifyStringVal(accountID2)) {
        otErr << strError << "accountID2" << std::endl;
    }

    if (boolInput) {
        if (!VerifyStringVal(strData)) {
            otErr << strError << "strData" << std::endl;
        }

        if (!VerifyStringVal(strData2)) {
            otErr << strError << "strData2" << std::endl;
        }

        if (!VerifyStringVal(strData3)) {
            otErr << strError << "strData3" << std::endl;
        }

        if (!VerifyStringVal(strData4)) {
            otErr << strError << "strData4" << std::endl;
        }
    }

    switch (theType) {
        case (ACKNOWLEDGE_CONNECTION): {
            nymID2 = account;
            instrumentDefinitionID = account2;
            peer_reply_ = PeerReply::Create(
                context_.Nym(),
                Identifier(account2),
                context_.Server(),
                boolInput,
                data,
                data2,
                data3,
                data4);

            OT_ASSERT(peer_reply_)
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()"
                  << std::endl;
            OT_FAIL
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    const api::client::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Exec& exec,
    const OT_API& otapi,
    const std::string& account,
    const std::string& account2,
    const std::string& data,
    const std::string& data2,
    const std::string& data3,
    const std::string& data4,
    bool boolInput,
    const time64_t time,
    const std::int64_t data5,
    const std::string& data6)
    : OTAPI_Func(wallet, exec, otapi, nymID, serverID, theType)
{
    accountID = account;
    accountID2 = account2;
    strData = data;
    strData2 = data2;
    strData3 = data3;
    strData4 = data4;
    bBool = boolInput;
    tData = time;
    lData = data5;
    const std::string strError =
        "Warning: Empty std::string passed to OTAPI_Func.OTAPI_Func() as: ";

    if (!VerifyStringVal(accountID)) {
        otErr << strError << "accountID" << std::endl;
    }

    if (!VerifyStringVal(accountID2)) {
        otErr << strError << "accountID2" << std::endl;
    }

    if (!VerifyStringVal(strData)) {
        otErr << strError << "strData" << std::endl;
    }

    if (!VerifyStringVal(strData2)) {
        otErr << strError << "strData2" << std::endl;
    }

    if (!VerifyStringVal(strData3)) {
        otErr << strError << "strData3" << std::endl;
    }

    if (!VerifyStringVal(strData4)) {
        otErr << strError << "strData4" << std::endl;
    }

    if (VerifyStringVal(strData5)) {
        strData5 = data6;
    }

    switch (theType) {
        case (CREATE_MARKET_OFFER): {
            nTransNumsNeeded = 3;
        } break;
        default: {
            otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func()"
                  << std::endl;
            OT_FAIL
        }
    }
}

TransactionNumber OTAPI_Func::GetTransactionNumber() const
{
    return std::get<1>(last_attempt_);
}

SendResult OTAPI_Func::LastSendResult() const
{
    Lock lock(lock_);

    return std::get<0>(std::get<2>(last_attempt_));
}

const std::shared_ptr<Message>& OTAPI_Func::Reply() const
{
    Lock lock(lock_);

    return std::get<1>(std::get<2>(last_attempt_));
}

std::string OTAPI_Func::Run(const std::size_t totalRetries)
{
    if (is_transaction_) {

        return send_transaction(totalRetries);
    } else {

        return send_request();
    }
}

void OTAPI_Func::run()
{
    Lock lock(lock_);
    const String data3(strData3);
    auto & [ requestNum, transactionNum, result ] = last_attempt_;
    auto & [ status, reply ] = result;
    requestNum = -1;
    transactionNum = 0;
    status = SendResult::ERROR;
    reply.reset();

    switch (type_) {
        case CHECK_NYM: {
            last_attempt_ = otapi_.checkNym(context_, Identifier(nymID2));
        } break;
        case REGISTER_NYM: {
            last_attempt_ = otapi_.registerNym(context_);
        } break;
        case DELETE_NYM: {
            last_attempt_ = otapi_.unregisterNym(context_);
        } break;
        case SEND_USER_MESSAGE: {
            last_attempt_ = otapi_.sendNymMessage(
                context_, Identifier(nymID2), strData2.c_str());
        } break;
        case SEND_USER_INSTRUMENT: {
            OTPayment thePayment(strData2.c_str());

            if (!thePayment.IsValid() || !thePayment.SetTempValues()) {
                otOut << OT_METHOD << __FUNCTION__
                      << ": Failure loading payment instrument "
                         "(intended for recipient) from string:\n\n"
                      << strData2 << "\n\n";
                return;
            }

            const bool bSenderCopyIncluded = (accountID.size() > 0);

            if (bSenderCopyIncluded) {
                OTPayment theSenderPayment(accountID.c_str());

                if (!theSenderPayment.IsValid() ||
                    !theSenderPayment.SetTempValues()) {
                    otOut << OT_METHOD << __FUNCTION__
                          << ": Failure loading payment instrument (copy "
                          << "intended for sender's records) from string."
                          << std::endl;

                    return;
                }

                last_attempt_ = otapi_.sendNymInstrument(
                    context_,
                    Identifier(nymID2),
                    thePayment,
                    &theSenderPayment);
            } else {
                last_attempt_ = otapi_.sendNymInstrument(
                    context_, Identifier(nymID2), thePayment);
            }
        } break;
        case GET_NYM_MARKET_OFFERS: {
            last_attempt_ = otapi_.getNymMarketOffers(context_);
        } break;
        case CREATE_ASSET_ACCT: {
            last_attempt_ = otapi_.registerAccount(
                context_, Identifier(instrumentDefinitionID));
        } break;
        case DELETE_ASSET_ACCT: {
            last_attempt_ =
                otapi_.deleteAssetAccount(context_, Identifier(accountID));
        } break;
        case ACTIVATE_SMART_CONTRACT: {
            last_attempt_ =
                otapi_.activateSmartContract(context_, strData2.c_str());
        } break;
        case TRIGGER_CLAUSE: {
            last_attempt_ = otapi_.triggerClause(
                context_,
                stoll(strData),
                strData2.c_str(),
                data3.Exists() ? &data3 : nullptr);
        } break;
        case EXCHANGE_BASKET: {
            last_attempt_ = otapi_.exchangeBasket(
                context_,
                Identifier(instrumentDefinitionID),
                basket.c_str(),
                bBool);
        } break;
        case GET_CONTRACT: {
            last_attempt_ = otapi_.getInstrumentDefinition(
                context_, Identifier(instrumentDefinitionID));
        } break;
        case GET_MINT: {
            last_attempt_ =
                otapi_.getMint(context_, Identifier(instrumentDefinitionID));
        } break;
        case ISSUE_BASKET: {
            last_attempt_ = otapi_.issueBasket(
                context_,
                proto::StringToProto<proto::UnitDefinition>(basket.c_str()));
        } break;
        case ISSUE_ASSET_TYPE: {
            last_attempt_ =
                otapi_.registerInstrumentDefinition(context_, strData.c_str());
        } break;
        case EXCHANGE_CASH: {
#if OT_CASH
            otErr << OT_METHOD << __FUNCTION__ << ": TODO (NOT CODED YET)"
                  << std::endl;
#endif  // OT_CASH
        } break;
        case KILL_MARKET_OFFER: {
            last_attempt_ = otapi_.cancelCronItem(
                context_, Identifier(accountID), stoll(strData));
        } break;
        case KILL_PAYMENT_PLAN: {
            last_attempt_ = otapi_.cancelCronItem(
                context_, Identifier(accountID), stoll(strData));
        } break;
        case GET_BOX_RECEIPT: {
            last_attempt_ = otapi_.getBoxReceipt(
                context_, Identifier(accountID), nData, stoll(strData));
        } break;
        case PROCESS_INBOX: {
            last_attempt_ = otapi_.processInbox(
                context_, Identifier(accountID), strData.c_str());
        } break;
        case DEPOSIT_CASH: {
#if OT_CASH
            last_attempt_ = otapi_.notarizeDeposit(
                context_, Identifier(accountID), strData.c_str());
#endif  // OT_CASH
        } break;
        case DEPOSIT_CHEQUE: {
            last_attempt_ = otapi_.depositCheque(
                context_, Identifier(accountID), strData.c_str());
        } break;
        case DEPOSIT_PAYMENT_PLAN: {
            last_attempt_ =
                otapi_.depositPaymentPlan(context_, strData.c_str());
        } break;
        case WITHDRAW_CASH: {
#if OT_CASH
            last_attempt_ = otapi_.notarizeWithdrawal(
                context_, Identifier(accountID), lData);
#endif  // OT_CASH
        } break;
        case WITHDRAW_VOUCHER: {
            last_attempt_ = otapi_.withdrawVoucher(
                context_,
                Identifier(accountID),
                Identifier(nymID2),
                strData.c_str(),
                lData);
        } break;
        case PAY_DIVIDEND: {
            last_attempt_ = otapi_.payDividend(
                context_,
                Identifier(accountID),
                Identifier(instrumentDefinitionID),
                strData.c_str(),
                lData);
        } break;
        case SEND_TRANSFER: {
            last_attempt_ = otapi_.notarizeTransfer(
                context_,
                Identifier(accountID),
                Identifier(accountID2),
                lData,
                strData.c_str());
        } break;
        case GET_MARKET_LIST: {
            last_attempt_ = otapi_.getMarketList(context_);
        } break;
        case GET_MARKET_OFFERS: {
            last_attempt_ =
                otapi_.getMarketOffers(context_, Identifier(strData), lData);
        } break;
        case GET_MARKET_RECENT_TRADES: {
            last_attempt_ =
                otapi_.getMarketRecentTrades(context_, Identifier(strData));
        } break;
        case CREATE_MARKET_OFFER: {
            const Identifier ASSET_ACCT_ID(accountID);
            const Identifier CURRENCY_ACCT_ID(accountID2);
            const std::int64_t MARKET_SCALE = stoll(strData);
            const std::int64_t MINIMUM_INCREMENT = stoll(strData2);
            const std::int64_t TOTAL_ASSETS_ON_OFFER = stoll(strData3);
            const Amount PRICE_LIMIT = stoll(strData4);
            const auto& bBuyingOrSelling = bBool;
            const auto& tLifespanInSeconds = tData;
            const auto& STOP_SIGN = strData5;
            const auto& ACTIVATION_PRICE = lData;
            char cStopSign = 0;

            if (0 == STOP_SIGN.compare("<")) {
                cStopSign = '<';
            } else if (0 == STOP_SIGN.compare(">")) {
                cStopSign = '>';
            }

            if (!STOP_SIGN.empty() &&
                ((ACTIVATION_PRICE == 0) ||
                 ((cStopSign != '<') && (cStopSign != '>')))) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": If STOP_SIGN is provided, it must be \"<\" "
                         "or \">\", and in that case ACTIVATION_PRICE "
                         "must be non-zero.\n";

                return;
            }

            const auto str_asset_notary_id =
                exec_.GetAccountWallet_NotaryID(accountID);
            const auto str_currency_notary_id =
                exec_.GetAccountWallet_NotaryID(accountID2);
            const auto str_asset_nym_id =
                exec_.GetAccountWallet_NymID(accountID);
            const auto str_currency_nym_id =
                exec_.GetAccountWallet_NymID(accountID2);

            if (str_asset_notary_id.empty() || str_currency_notary_id.empty() ||
                str_asset_nym_id.empty() || str_currency_nym_id.empty()) {
                otErr << OT_METHOD << __FUNCTION__
                      << ": Failed determining server or nym ID for "
                         "either asset or currency account.\n";

                return;
            }

            last_attempt_ = otapi_.issueMarketOffer(
                context_,
                ASSET_ACCT_ID,
                CURRENCY_ACCT_ID,
                (0 == MARKET_SCALE) ? 1 : MARKET_SCALE,
                (0 == MINIMUM_INCREMENT) ? 1 : MINIMUM_INCREMENT,
                (0 == TOTAL_ASSETS_ON_OFFER) ? 1 : TOTAL_ASSETS_ON_OFFER,
                PRICE_LIMIT,
                bBuyingOrSelling,
                tLifespanInSeconds,
                cStopSign,
                ACTIVATION_PRICE);
        } break;
        case ADJUST_USAGE_CREDITS: {
            last_attempt_ = otapi_.usageCredits(
                context_, Identifier(nymID2), stoll(strData));
        } break;
        case INITIATE_BAILMENT:
        case INITIATE_OUTBAILMENT:
        case NOTIFY_BAILMENT:
        case REQUEST_CONNECTION:
        case STORE_SECRET: {
            OT_ASSERT(peer_request_);

            last_attempt_ = otapi_.initiatePeerRequest(
                context_, Identifier(nymID2), peer_request_);
        } break;
        case ACKNOWLEDGE_BAILMENT:
        case ACKNOWLEDGE_OUTBAILMENT:
        case ACKNOWLEDGE_NOTICE:
        case ACKNOWLEDGE_CONNECTION: {
            OT_ASSERT(peer_reply_);

            last_attempt_ = otapi_.initiatePeerReply(
                context_,
                Identifier(nymID2),
                Identifier(instrumentDefinitionID),
                peer_reply_);
        } break;
        case REGISTER_CONTRACT_NYM: {
            last_attempt_ = otapi_.registerContract(
                context_, ContractType::NYM, Identifier(nymID2));
        } break;
        case REGISTER_CONTRACT_SERVER: {
            last_attempt_ = otapi_.registerContract(
                context_, ContractType::SERVER, Identifier(strData));
        } break;
        case REGISTER_CONTRACT_UNIT: {
            last_attempt_ = otapi_.registerContract(
                context_,
                ContractType::UNIT,
                Identifier(instrumentDefinitionID));
        } break;
        case REQUEST_ADMIN: {
            last_attempt_ = otapi_.requestAdmin(context_, strData.c_str());
        } break;
        case SERVER_ADD_CLAIM: {
            last_attempt_ = otapi_.serverAddClaim(
                context_,
                strData.c_str(),
                strData2.c_str(),
                strData3.c_str(),
                bBool);
        } break;
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Error: unhandled function "
                  << " type: " << type_ << std::endl;

            OT_FAIL;
        }
    }
}

std::int32_t OTAPI_Func::send()
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation{OT_METHOD};
    strLocation += __FUNCTION__;
    strLocation += ": " + type_name_.at(type_);
    otapi_.FlushMessageBuffer();
    run();
    const auto& requestNumber = std::get<0>(last_attempt_);
    const auto& status = std::get<0>(std::get<2>(last_attempt_));

    switch (status) {
        case SendResult::TRANSACTION_NUMBERS:
        case SendResult::INVALID_REPLY:
        case SendResult::TIMEOUT:
        case SendResult::ERROR: {
            otOut << strLocation << ": Failed to send message due to error."
                  << std::endl;

            nRequestNum = -1;
        } break;
        case SendResult::UNNECESSARY: {
            otOut << strLocation << ": Didn't send this message, but NO error "
                  << "occurred, either. (For example, a request to process an "
                  << "empty Nymbox will return 0, meaning, nothing was sent, "
                  << "but also no error occurred.)" << std::endl;

            nRequestNum = 0;
        } break;
        case SendResult::VALID_REPLY: {
            // BY this point, we definitely have the request number, which means
            // the message was actually SENT. (At least.) This also means we can
            // use nRun later to query for a copy of that sent message (like if
            // we need to clawback the transaction numbers from it later, once
            // we confirm whether the server actually never got it.)

            nRequestNum = requestNumber;
        } break;
        default: {
            OT_FAIL
        }
    }

    return nRequestNum;
}

std::string OTAPI_Func::send_transaction(std::size_t totalRetries)
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation =
        "OTAPI_Func::SendTransaction: " + type_name_.at(type_);

    if (!MsgUtil.getIntermediaryFiles(
            String(context_.Server()).Get(),
            String(context_.Nym()->ID()).Get(),
            accountID,
            false))  // bForceDownload=false))
    {
        otOut << strLocation << ", getIntermediaryFiles returned false. (It "
                                "couldn't download files that it needed.)\n";
        return "";
    }

    // GET TRANSACTION NUMBERS HERE IF NECESSARY.
    //
    std::int32_t getnym_trnsnum_count = exec_.GetNym_TransactionNumCount(
        String(context_.Server()).Get(), String(context_.Nym()->ID()).Get());
    std::int32_t configTxnCount = MsgUtil.getNbrTransactionCount();
    bool b1 = (nTransNumsNeeded > configTxnCount);
    std::int32_t comparative = 0;

    if (b1) {
        comparative = nTransNumsNeeded;
    } else {
        comparative = configTxnCount;
    }

    if (getnym_trnsnum_count < comparative) {
        otOut << strLocation << ", I don't have enough transaction numbers to "
                                "perform this transaction. Grabbing more "
                                "now...\n";
        MsgUtil.setNbrTransactionCount(comparative);
        MsgUtil.getTransactionNumbers(
            String(context_.Server()).Get(),
            String(context_.Nym()->ID()).Get());
        MsgUtil.setNbrTransactionCount(configTxnCount);
    }

    // second try
    getnym_trnsnum_count = exec_.GetNym_TransactionNumCount(
        String(context_.Server()).Get(), String(context_.Nym()->ID()).Get());
    if (getnym_trnsnum_count < comparative) {
        otOut
            << strLocation
            << ", failure: MsgUtil.getTransactionNumbers. (Trying again...)\n";

        // (the final parameter, the 'false' is us telling getTransNumbers that
        // it can skip the first call to getTransNumLowLevel)
        //
        MsgUtil.setNbrTransactionCount(comparative);
        MsgUtil.getTransactionNumbers(
            String(context_.Server()).Get(),
            String(context_.Nym()->ID()).Get(),
            false);
        MsgUtil.setNbrTransactionCount(configTxnCount);
    }

    // third try
    getnym_trnsnum_count = exec_.GetNym_TransactionNumCount(
        String(context_.Server()).Get(), String(context_.Nym()->ID()).Get());
    if (getnym_trnsnum_count < comparative) {
        otOut
            << strLocation
            << ", failure: MsgUtil.getTransactionNumbers. (Trying again...)\n";

        // (the final parameter, the 'false' is us telling getTransNumbers that
        // it can skip the first call to getTransNumLowLevel)
        //
        MsgUtil.setNbrTransactionCount(comparative);
        MsgUtil.getTransactionNumbers(
            String(context_.Server()).Get(),
            String(context_.Nym()->ID()).Get(),
            false);
        MsgUtil.setNbrTransactionCount(configTxnCount);
    }

    // Giving up, if still a failure by this point.
    //
    getnym_trnsnum_count = exec_.GetNym_TransactionNumCount(
        String(context_.Server()).Get(), String(context_.Nym()->ID()).Get());

    if (getnym_trnsnum_count < comparative) {
        otOut
            << strLocation
            << ", third failure: MsgUtil.getTransactionNumbers. (Giving up.)\n";
        return "";
    }

    bool bCanRetryAfterThis = false;

    std::string strResult = send_once(true, true, bCanRetryAfterThis);

    if (VerifyStringVal(strResult)) {
        otOut << " Getting Intermediary files.. \n";

        if (!MsgUtil.getIntermediaryFiles(
                String(context_.Server()).Get(),
                String(context_.Nym()->ID()).Get(),
                accountID,
                true)) {
            otOut << strLocation << ", getIntermediaryFiles returned false. "
                                    "(After a success sending the transaction. "
                                    "Strange...)\n";
            return "";
        }

        return strResult;
    }

    //
    // Maybe we have an old Inbox or something.
    //

    // TODO!!  SECURITY:  This is where a GOOD CLIENT (vs. a test client)
    // will verify these std::intermediary files against your LAST SIGNED
    // RECEIPT,
    // using SwigWrap::VerifySomethingorother().
    // See verifyFiles() at the bottom of this file.
    // Add some kind of warning Modal Dialog here, since it's actually
    // normal for a NEW account (never had any receipts yet.) But for
    // any other account, this should ALWAYS VERIFY!
    //
    // Another note: this should happen INSIDE the getIntermediaryFiles call
    // itself,
    // and all similar calls.  You simply should not download those files,
    // without verifying them also. Otherwise you could end up signing
    // a future bad receipt, based on malicious, planted std::intermediary
    // files.

    auto nRetries = totalRetries;

    while ((nRetries > 0) && !VerifyStringVal(strResult) &&
           bCanRetryAfterThis) {
        --nRetries;

        bool bWillRetryAfterThis = true;

        if ((nRetries == 0) || !bCanRetryAfterThis) {
            bWillRetryAfterThis = false;
        }

        if (exec_.CheckConnection(String(context_.Server()).Get())) {
            strResult =
                send_once(true, bWillRetryAfterThis, bCanRetryAfterThis);
        }

        // In case of failure, we want to get these before we re-try.
        // But in case of success, we also want to get these, so we can
        // see the results of our success. So we get these either way...
        //
        if (VerifyStringVal(strResult)) {
            if (!MsgUtil.getIntermediaryFiles(
                    String(context_.Server()).Get(),
                    String(context_.Nym()->ID()).Get(),
                    accountID,
                    true)) {
                otOut << strLocation
                      << ", getIntermediaryFiles (loop) returned false even "
                         "after successfully sending the transaction.\n";
                return "";
            }
            break;
        }
    }

    return strResult;
}

std::string OTAPI_Func::send_request()
{
    Utility MsgUtil(context_, otapi_);
    bool bCanRetryAfterThis = false;
    std::string strResult = send_once(false, true, bCanRetryAfterThis);

    if (!VerifyStringVal(strResult) && bCanRetryAfterThis) {
        if (exec_.CheckConnection(String(context_.Server()).Get())) {
            strResult = send_once(false, false, bCanRetryAfterThis);
        }
    }
    return strResult;
}

std::string OTAPI_Func::send_once(
    const bool bIsTransaction,
    const bool bWillRetryAfterThis,
    bool& bCanRetryAfterThis)
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation =
        "OTAPI_Func::SendRequestOnce: " + type_name_.at(type_);
    ;

    bCanRetryAfterThis = false;

    std::string strReply = "";
    std::int32_t nlocalRequestNum = send();

    if ((nlocalRequestNum == -1) || (nlocalRequestNum == 0)) {
        // bCanRetryAfterThis is already false here.
        return "";
    } else  // 1 and default.
    {
        if (nlocalRequestNum < -1) {
            return "";
        }

        strReply = MsgUtil.ReceiveReplyLowLevel(
            String(context_.Server()).Get(),
            String(context_.Nym()->ID()).Get(),
            nlocalRequestNum,
            type_name_.at(type_));
    }

    // Below this point, we definitely have a request number.
    // (But not yet necessarily a reply...)
    //

    // nlocalRequestNum is positive and contains the request number from
    // sending.
    //
    // nReplySuccess contains status of the REPLY to that sent message.
    // nReplySuccess contains:
    //   0 == FAILURE reply msg from server,
    //   1 == SUCCESS reply msg from server (transaction could be success or
    // fail.)
    //  -1 == (ERROR)
    //
    // strReply contains the reply itself (or null.)
    //
    std::int32_t nReplySuccess = VerifyMessageSuccess(strReply);

    bool bMsgReplyError = (!VerifyStringVal(strReply) || (nReplySuccess < 0));

    bool bMsgReplySuccess = (!bMsgReplyError && (nReplySuccess > 0));
    bool bMsgReplyFailure = (!bMsgReplyError && (nReplySuccess == 0));

    bool bMsgTransSuccess{false};
    bool bMsgTransFailure{false};

    bool bMsgAnyError{false};
    bool bMsgAnyFailure{false};

    bool bMsgAllSuccess{false};

    // If you EVER are in a situation where you have to harvest numbers
    // back, it will ONLY be for transactions, not normal messages. (Those
    // are the only ones that USE transaction numbers.)
    //
    if (bIsTransaction)  // This request contains a TRANSACTION...
    {
        std::int32_t nTransSuccess{-1};
        std::int32_t nBalanceSuccess{-1};
        if (bMsgReplySuccess)  // If message was success, then let's see if the
                               // transaction was, too.
        {
            nBalanceSuccess = exec_.Message_GetBalanceAgreementSuccess(
                String(context_.Server()).Get(),
                String(context_.Nym()->ID()).Get(),
                accountID,
                strReply);

            if (nBalanceSuccess > 0) {
                // Sometimes a transaction is sent that is meant to "fail" in
                // order to cancel itself from ever being run in the future.
                // It's being cancelled. In that case, whether the server reply
                // itself is acknowledged or rejection, either way,
                // IsCancelled() will be set to TRUE.
                // This is used when cancelling a cheque, or a payment plan, or
                // a smart contract, so that it can never be activated at some
                // future time.
                //
                // Therefore when we see that IsCancelled is set to TRUE, we
                // std::interpret it as a "success" as far as the UI is
                // concerned,
                // even though behind the scenes, it is still "rejected" and
                // transaction numbers were harvested from it.
                //
                std::int32_t nTransCancelled =
                    exec_.Message_IsTransactionCanceled(
                        String(context_.Server()).Get(),
                        String(context_.Nym()->ID()).Get(),
                        accountID,
                        strReply);

                // If it's not cancelled, then we assume it's a normal
                // transaction (versus a cancellation)
                // and we check for success/failure as normal...
                //
                if (1 != nTransCancelled) {
                    nTransSuccess = exec_.Message_GetTransactionSuccess(
                        String(context_.Server()).Get(),
                        String(context_.Nym()->ID()).Get(),
                        accountID,
                        strReply);
                } else  // If it WAS cancelled, then for the UI we say "Success"
                        // even though OT behind the scenes is harvesting as
                        // though it failed.
                {       // (Which is what we want to happen, in the case that a
                    // cancellation was performed.)
                    // This way, the UI won't go off doing a bunch of
                    // unnecessary retries for a "failed" transaction.
                    // (After all, if it was cancelled, then we know for a fact
                    // that all future retries will fail anyway.)
                    //
                    nTransSuccess = 1;
                }
            } else {
                nTransSuccess = -1;
            }
        } else {
            nBalanceSuccess = -1;
            nTransSuccess = -1;
        }
        // All of these booleans (except "error") represent RECEIVED ANSWERS
        // from the server.
        // In other words, "failure" does not mean "failed to find message."
        // Rather, it means "DEFINITELY got a reply, and that reply says
        // status==failure."
        //

        bool bMsgBalanceError =
            (!VerifyStringVal(strReply) || (nBalanceSuccess < 0));
        bool bMsgBalanceSuccess =
            (!bMsgReplyError && !bMsgBalanceError && (nBalanceSuccess > 0));
        bool bMsgBalanceFailure =
            (!bMsgReplyError && !bMsgBalanceError && (nBalanceSuccess == 0));

        bool bMsgTransError =
            (!VerifyStringVal(strReply) || (nTransSuccess < 0));
        bMsgTransSuccess =
            (!bMsgReplyError && !bMsgBalanceError && !bMsgTransError &&
             (nTransSuccess > 0));
        bMsgTransFailure =
            (!bMsgReplyError && !bMsgBalanceError && !bMsgTransError &&
             (nTransSuccess == 0));

        bMsgAnyError = (bMsgReplyError || bMsgBalanceError || bMsgTransError);
        bMsgAnyFailure =
            (bMsgReplyFailure || bMsgBalanceFailure || bMsgTransFailure);

        bMsgAllSuccess =
            (bMsgReplySuccess && bMsgBalanceSuccess && bMsgTransSuccess);

    } else  // it's NOT a transaction, but a normal message..
    {
        bMsgTransSuccess = false;
        bMsgTransFailure = false;

        bMsgAnyError = (bMsgReplyError);
        bMsgAnyFailure = (bMsgReplyFailure);

        bMsgAllSuccess = (bMsgReplySuccess);
    }

    // We know the message SENT. The above logic is about figuring out whether
    // the reply message,
    // the transaction inside it, and the balance agreement inside that
    // transaction, whether
    // any of those three things is a definite error, a definite failure, or a
    // definite success.
    // (Any one of those things could be true, OR NOT, and we can only act as if
    // they are, if we
    // have definitive proof in any of those cases.)
    //
    // The below logic is about what sort of REPLY we may have gotten (if
    // anything.)
    // Without a definite reply we cannot claw back. But the Nymbox can show us
    // this answer,
    // either now, or later...
    //
    if (bMsgAllSuccess) {
        // the Msg was a complete success, including the message
        // AND the transaction AND the balance agreement.
        // Therefore, there's DEFINITELY nothing to clawback.
        //
        // (Thus I RemoveSentMessage for the message, since
        // I'm totally done with it now. NO NEED TO HARVEST anything, either.)
        //
        //          var nRemoved =
        // SwigWrap::RemoveSentMessage(Integer.toString(nlocalRequestNum),
        // String(context_.Server()).Get(),
        // String(context_.Nym()->ID()).Get());
        //
        // NOTE: The above call is unnecessary, since a successful reply means
        // we already received the successful server reply, and OT's
        // "ProcessServerReply"
        // already removed the sent message from the sent buffer (so no need to
        // do that here.)

        //          otOut << strLocation << ", SendRequestOnce():
        // OT_API_RemoveSentMessage: " + nRemoved);

        return strReply;

    }
    // In this case we want to grab the Nymbox and see if the replyNotice is
    // there.
    // If it IS, then OT server DEFINITELY replied to it (the MESSAGE was a
    // success,
    // whether the transaction inside of it was success or fail.) So we know
    // bMsgReplySuccess
    // is true, if we find a replyNotice.
    // From there, we can also check for transaction success.
    //
    else if (bMsgAnyError || bMsgAnyFailure)  // let's resync, and clawback
                                              // whatever transaction numbers we
                                              // might have used on the
                                              // Request...
    {
        bool bWasGetReqSent = false;
        const auto nGetRequestNumber =
            context_.UpdateRequestNumber(bWasGetReqSent);

        // GET REQUEST WAS A SUCCESS.
        //
        if (bWasGetReqSent && (nGetRequestNumber > 0)) {
            bCanRetryAfterThis = true;

            // But--if it was a TRANSACTION, then we're not done syncing yet!
            //
            if (bIsTransaction) {
                bCanRetryAfterThis = false;

                //
                // Maybe we have an old Inbox or something.
                // NEW CODE HERE FOR DEBUGGING (THIS BLOCK)
                //
                bool bForceDownload = true;
                if (!MsgUtil.getIntermediaryFiles(
                        String(context_.Server()).Get(),
                        String(context_.Nym()->ID()).Get(),
                        accountID,
                        bForceDownload)) {
                    otOut << strLocation << ", getIntermediaryFiles returned "
                                            "false. (After a failure to send "
                                            "the transaction. Thus, I give "
                                            "up.)\n";
                    return "";
                }

                bool bWasFound = false;
                bool bWasSent = false;

                OTfourbool the_foursome = {bMsgReplySuccess,
                                           bMsgReplyFailure,
                                           bMsgTransSuccess,
                                           bMsgTransFailure};

                bForceDownload = false;

                std::int32_t nProcessNymboxResult =
                    MsgUtil.getAndProcessNymbox_8(
                        String(context_.Server()).Get(),
                        String(context_.Nym()->ID()).Get(),
                        bWasSent,
                        bForceDownload,
                        nlocalRequestNum,
                        bWasFound,
                        bWillRetryAfterThis,
                        the_foursome);

                // bHarvestingForRetry,// bHarvestingForRetry is INPUT, in the
                // case nlocalRequestNum needs to be harvested before a flush
                // occurs.

                //  bMsgReplySuccess,    // bMsgReplySuccess is INPUT, and is in
                // case nlocalRequestNum needs to be HARVESTED before a FLUSH
                // happens.
                //  bMsgReplyFailure,    // bMsgReplyFailure is INPUT, and is in
                // case nlocalRequestNum needs to be HARVESTED before a FLUSH
                // happens.
                //  bMsgTransSuccess,    // bMsgTransSuccess is INPUT, and is in
                // case nlocalRequestNum needs to be HARVESTED before a FLUSH
                // happens.
                //  bMsgTransFailure)    // Etc.

                // FIX: Add '(' and ')' here to silence warning. But where?
                if ((bWasSent && (1 == nProcessNymboxResult)) ||
                    (!bWasSent && (0 == nProcessNymboxResult)))  // success
                                                                 // processing
                                                                 // Nymbox.
                {
                    bCanRetryAfterThis = true;
                }

                // This means a request number was returned, since the
                // processNymbox failed,
                // and hasn't been properly harvested, so we either need to
                // harvest it for a re-try,
                // or flush it.
                //
                else if (bWasSent && (nProcessNymboxResult > 1)) {
                    std::string strNymbox = exec_.LoadNymboxNoVerify(
                        String(context_.Server()).Get(),
                        String(context_.Nym()->ID()).Get());  // FLUSH SENT
                                                              // MESSAGES!!!!
                                                              // (AND
                                                              // HARVEST.);

                    if (VerifyStringVal(strNymbox)) {
                        exec_.FlushSentMessages(
                            false,
                            String(context_.Server()).Get(),
                            String(context_.Nym()->ID()).Get(),
                            strNymbox);
                    }
                }
            }  // if (bIsTransaction)

        }  // if getRequestNumber was success.
        else {
            otOut << strLocation
                  << ", Failure: Never got reply from server, "
                     "so tried getRequestNumber, and that failed too. "
                     "(I give up.)\n";

            // Note: cannot harvest transaction nums here because I do NOT know
            // for sure
            // whether the server has replied to the message or not! (Not until
            // I successfully
            // download my Nymbox.) Therefore, do NOT harvest or flush, but hold
            // back and wait
            // until the next attempt does a successful getNymbox and THEN do a
            // "flush sent" after
            // that. (That's the time we'll know for SURE what happened to the
            // original reply.)
            //
            // (Therefore LEAVE the sent message in the sent queue.)

            return "";
        }
    }  // else if (bMsgAnyError || bMsgAnyFailure)

    // Returning an empty std::string.

    return "";
}

const std::shared_ptr<PeerRequest>& OTAPI_Func::SentPeerRequest() const
{
    return peer_request_;
}

const std::shared_ptr<PeerReply>& OTAPI_Func::SentPeerReply() const
{
    return peer_reply_;
}

OTAPI_Func::~OTAPI_Func() {}
}  // namespace opentxs
