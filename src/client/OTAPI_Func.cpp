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

#include "opentxs/api/Api.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Wallet.hpp"
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
OTAPI_Func::OTAPI_Func(
    api::Wallet& wallet,
    OTAPI_Exec& exec,
    OT_API& otapi,
    const Identifier& nymID,
    const Identifier& serverID,
    const OTAPI_Func_Type type)
    : funcType(type)
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
    , transaction_number_(0)
    , wallet_(wallet)
    , context_editor_(wallet_.mutable_ServerContext(nymID, serverID))
    , context_(context_editor_.It())
    , exec_(exec)
    , otapi_(otapi)
    , last_reply_(nullptr)
    , last_send_status_(SendResult::ERROR)
    , peer_reply_(nullptr)
    , peer_request_(nullptr)
{
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi)
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
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
    } else if (theType == NOTIFY_BAILMENT) {
        nTransNumsNeeded = 0;
        nymID2 = p_nymID2;
        instrumentDefinitionID = p_strData;
        peer_request_ = PeerRequest::Create(
            context_.Nym(),
            proto::PEERREQUEST_PENDINGBAILMENT,
            Identifier(instrumentDefinitionID),
            context_.Server(),
            Identifier(nymID2),
            p_strData2);

        OT_ASSERT(peer_request_)
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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

    nTransNumsNeeded = 1;
    accountID = p_accountID;

    if (theType == SEND_USER_INSTRUMENT) {
        if (!VerifyStringVal(p_accountID)) {
            otErr << strError << "p_accountID" << std::endl;
        }
        if (!VerifyStringVal(p_strParam)) {
            otErr << strError << "p_strParam" << std::endl;
        }
        if (!VerifyStringVal(p_strData)) {
            otErr << strError << "p_strData" << std::endl;
        }
        nTransNumsNeeded = 0;
        nymID2 = p_accountID;  // Recipient Nym;
        strData = p_strParam;  // Recipient pubkey;
        strData2 = p_strData;  // Instrument for recipient.;
        accountID =
            p_strData2;  // sender_instrument is attached here. (Optional.);
    } else {
        otOut << "ERROR! WRONG TYPE passed to OTAPI_Func.OTAPI_Func() "
                 "ERROR!!!!!!\n";
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        }
    }
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
    }

    nTransNumsNeeded = p_nTransNumsNeeded;
    bBool = p_bBool;
    instrumentDefinitionID = p_instrumentDefinitionID;
    basket = p_basket;
    accountID = p_accountID;
}

OTAPI_Func::OTAPI_Func(
    OTAPI_Func_Type theType,
    api::Wallet& wallet,
    const Identifier& nymID,
    const Identifier& serverID,
    OTAPI_Exec& exec,
    OT_API& otapi,
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
        case (CREATE_MARKET_OFFER): {
            nTransNumsNeeded = 3;
        } break;
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
        }
    }
}

// -1 means error, no message was sent.
//  0 means NO error, yet still no message was sent.
// >0 means (usually) the request number is being returned.
std::int32_t OTAPI_Func::Run() const
{
    Lock lock(lock_);
    const String data3(strData3);

    switch (funcType) {
        case CHECK_NYM: {
            auto[requestNum, transactionNum, result] =
                otapi_.checkNym(context_, Identifier(nymID2));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case REGISTER_NYM: {
            auto[requestNum, transactionNum, result] =
                otapi_.registerNym(context_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case DELETE_NYM: {
            auto[requestNum, transactionNum, result] =
                otapi_.unregisterNym(context_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case SEND_USER_MESSAGE: {
            auto[requestNum, transactionNum, result] = otapi_.sendNymMessage(
                context_, Identifier(nymID2), strData2.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case SEND_USER_INSTRUMENT: {
            CommandResult send{};
            auto & [ requestNum, transactionNum, result ] = send;
            auto & [ status, reply ] = result;
            requestNum = -1;
            transactionNum = 0;
            status = SendResult::ERROR;
            reply.reset(nullptr);
            OTPayment thePayment(strData2.c_str());

            if (!thePayment.IsValid() || !thePayment.SetTempValues()) {
                otOut << OT_METHOD << __FUNCTION__
                      << ": Failure loading payment instrument "
                         "(intended for recipient) from string:\n\n"
                      << strData2 << "\n\n";
                return -1;
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

                    return -1;
                }

                send = otapi_.sendNymInstrument(
                    context_,
                    Identifier(nymID2),
                    thePayment,
                    &theSenderPayment);
            } else {
                send = otapi_.sendNymInstrument(
                    context_, Identifier(nymID2), thePayment);
            }

            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_NYM_MARKET_OFFERS: {
            auto[requestNum, transactionNum, result] =
                otapi_.getNymMarketOffers(context_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case CREATE_ASSET_ACCT: {
            auto[requestNum, transactionNum, result] = otapi_.registerAccount(
                context_, Identifier(instrumentDefinitionID));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case DELETE_ASSET_ACCT: {
            auto[requestNum, transactionNum, result] =
                otapi_.deleteAssetAccount(context_, Identifier(accountID));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case ACTIVATE_SMART_CONTRACT: {
            auto[requestNum, transactionNum, result] =
                otapi_.activateSmartContract(context_, strData2.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case TRIGGER_CLAUSE: {
            auto[requestNum, transactionNum, result] = otapi_.triggerClause(
                context_,
                stoll(strData),
                strData2.c_str(),
                data3.Exists() ? &data3 : nullptr);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case EXCHANGE_BASKET: {
            auto[requestNum, transactionNum, result] = otapi_.exchangeBasket(
                context_,
                Identifier(instrumentDefinitionID),
                basket.c_str(),
                bBool);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_CONTRACT: {
            auto[requestNum, transactionNum, result] =
                otapi_.getInstrumentDefinition(
                    context_, Identifier(instrumentDefinitionID));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_MINT: {
            auto[requestNum, transactionNum, result] =
                otapi_.getMint(context_, Identifier(instrumentDefinitionID));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case ISSUE_BASKET: {
            auto[requestNum, transactionNum, result] = otapi_.issueBasket(
                context_,
                proto::StringToProto<proto::UnitDefinition>(basket.c_str()));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case ISSUE_ASSET_TYPE: {
            auto[requestNum, transactionNum, result] =
                otapi_.registerInstrumentDefinition(context_, strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case EXCHANGE_CASH: {
#if OT_CASH
            otErr << OT_METHOD << __FUNCTION__ << ": TODO (NOT CODED YET)"
                  << std::endl;
            transaction_number_.store(0);
            last_send_status_ = SendResult::ERROR;
            last_reply_.reset();

            return -1;
#else
            return -1;
#endif  // OT_CASH
        } break;
        case KILL_MARKET_OFFER: {
            auto[requestNum, transactionNum, result] = otapi_.cancelCronItem(
                context_, Identifier(accountID), stoll(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case KILL_PAYMENT_PLAN: {
            auto[requestNum, transactionNum, result] = otapi_.cancelCronItem(
                context_, Identifier(accountID), stoll(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_BOX_RECEIPT: {
            auto[requestNum, transactionNum, result] = otapi_.getBoxReceipt(
                context_, Identifier(accountID), nData, stoll(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case PROCESS_INBOX: {
            auto[requestNum, transactionNum, result] = otapi_.processInbox(
                context_, Identifier(accountID), strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case DEPOSIT_CASH: {
#if OT_CASH
            auto[requestNum, transactionNum, result] = otapi_.notarizeDeposit(
                context_, Identifier(accountID), strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
#else
            return -1;
#endif  // OT_CASH
        } break;
        case DEPOSIT_CHEQUE: {
            auto[requestNum, transactionNum, result] = otapi_.depositCheque(
                context_, Identifier(accountID), strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case DEPOSIT_PAYMENT_PLAN: {
            auto[requestNum, transactionNum, result] =
                otapi_.depositPaymentPlan(context_, strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case WITHDRAW_CASH: {
#if OT_CASH
            auto[requestNum, transactionNum, result] =
                otapi_.notarizeWithdrawal(
                    context_, Identifier(accountID), lData);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
#else
            return -1;
#endif  // OT_CASH
        } break;
        case WITHDRAW_VOUCHER: {
            auto[requestNum, transactionNum, result] = otapi_.withdrawVoucher(
                context_,
                Identifier(accountID),
                Identifier(nymID2),
                strData.c_str(),
                lData);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case PAY_DIVIDEND: {
            auto[requestNum, transactionNum, result] = otapi_.payDividend(
                context_,
                Identifier(accountID),
                Identifier(instrumentDefinitionID),
                strData.c_str(),
                lData);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case SEND_TRANSFER: {
            auto[requestNum, transactionNum, result] = otapi_.notarizeTransfer(
                context_,
                Identifier(accountID),
                Identifier(accountID2),
                lData,
                strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_MARKET_LIST: {
            auto[requestNum, transactionNum, result] =
                otapi_.getMarketList(context_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_MARKET_OFFERS: {
            auto[requestNum, transactionNum, result] =
                otapi_.getMarketOffers(context_, Identifier(strData), lData);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case GET_MARKET_RECENT_TRADES: {
            auto[requestNum, transactionNum, result] =
                otapi_.getMarketRecentTrades(context_, Identifier(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
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

                return -1;
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

                return -1;
            }

            auto[requestNum, transactionNum, result] = otapi_.issueMarketOffer(
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
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case ADJUST_USAGE_CREDITS: {
            auto[requestNum, transactionNum, result] = otapi_.usageCredits(
                context_, Identifier(nymID2), stoll(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case INITIATE_BAILMENT:
        case INITIATE_OUTBAILMENT:
        case NOTIFY_BAILMENT:
        case REQUEST_CONNECTION:
        case STORE_SECRET: {
            auto[requestNum, transactionNum, result] =
                otapi_.initiatePeerRequest(
                    context_, Identifier(nymID2), peer_request_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case ACKNOWLEDGE_BAILMENT:
        case ACKNOWLEDGE_OUTBAILMENT:
        case ACKNOWLEDGE_NOTICE:
        case ACKNOWLEDGE_CONNECTION: {
            auto[requestNum, transactionNum, result] = otapi_.initiatePeerReply(
                context_,
                Identifier(nymID2),
                Identifier(instrumentDefinitionID),
                peer_reply_);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case REGISTER_CONTRACT_NYM: {
            auto[requestNum, transactionNum, result] = otapi_.registerContract(
                context_, ContractType::NYM, Identifier(nymID2));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case REGISTER_CONTRACT_SERVER: {
            auto[requestNum, transactionNum, result] = otapi_.registerContract(
                context_, ContractType::SERVER, Identifier(strData));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case REGISTER_CONTRACT_UNIT: {
            auto[requestNum, transactionNum, result] = otapi_.registerContract(
                context_,
                ContractType::UNIT,
                Identifier(instrumentDefinitionID));
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case REQUEST_ADMIN: {
            auto[requestNum, transactionNum, result] =
                otapi_.requestAdmin(context_, strData.c_str());
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        case SERVER_ADD_CLAIM: {
            auto[requestNum, transactionNum, result] = otapi_.serverAddClaim(
                context_,
                strData.c_str(),
                strData2.c_str(),
                strData3.c_str(),
                bBool);
            auto & [ status, reply ] = result;
            transaction_number_.store(transactionNum);
            last_send_status_ = status;
            last_reply_.reset(reply.release());

            if (0 == requestNum) {

                return 0;
            }

            if (SendResult::VALID_REPLY != status) {

                return -1;
            }

            return requestNum;
        } break;
        default: {
        }
    }

    otOut << "ERROR! OTAPI_Func.Send() activated, with bad function type: "
          << funcType << "\n";

    return -1;
}

std::int32_t OTAPI_Func::SendRequestLowLevel(
    OTAPI_Func& theFunction,
    const std::string& IN_FUNCTION) const
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation = "OTAPI_Func::SendRequestLowLevel: " + IN_FUNCTION;

    otapi_.FlushMessageBuffer();

    std::int32_t nRun =
        theFunction.Run();  // <===== ATTEMPT TO SEND THE MESSAGE HERE...;

    if (nRun == -1)  // if the requestNumber returned by the send-attempt is -1,
                     // that means it DIDN'T SEND (error)
    {
        otOut << strLocation << ": Failed to send message due to error.\n";

        theFunction.nRequestNum = -1;
    } else if (nRun == 0)  // if the requestNumber returned by the send-attempt
                           // is 0, it means no error, but nothing was sent.
                           // (Like processing an empty box.)
    {
        otOut << strLocation << ": Didn't send this message, but NO error "
                                "occurred, either. (For example, a request to "
                                "process an empty Nymbox will return 0, "
                                "meaning, nothing was sent, but also no error "
                                "occurred.)\n";

        theFunction.nRequestNum = 0;
    } else {
        theFunction.nRequestNum = nRun;
    }

    // BY this point, we definitely have the request number, which means the
    // message was actually SENT. (At least.) This also means we can use nRun
    // later to query for a copy of that sent message (like if we need to
    // clawback
    // the transaction numbers from it later, once we confirm whether the server
    // actually never got it.)
    //
    return theFunction.nRequestNum;
}

std::string OTAPI_Func::SendTransaction(
    OTAPI_Func& theFunction,
    const std::string& IN_FUNCTION)
{
    std::int32_t nTotalRetries = 2;
    return SendTransaction(theFunction, IN_FUNCTION, nTotalRetries);
}

std::string OTAPI_Func::SendTransaction(
    OTAPI_Func& theFunction,
    const std::string& IN_FUNCTION,
    std::int32_t nTotalRetries) const
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation = "OTAPI_Func::SendTransaction: " + IN_FUNCTION;

    if (!MsgUtil.getIntermediaryFiles(
            String(theFunction.context_.Server()).Get(),
            String(theFunction.context_.Nym()->ID()).Get(),
            theFunction.accountID,
            false))  // bForceDownload=false))
    {
        otOut << strLocation << ", getIntermediaryFiles returned false. (It "
                                "couldn't download files that it needed.)\n";
        return "";
    }

    // GET TRANSACTION NUMBERS HERE IF NECESSARY.
    //
    std::int32_t getnym_trnsnum_count = exec_.GetNym_TransactionNumCount(
        String(theFunction.context_.Server()).Get(),
        String(theFunction.context_.Nym()->ID()).Get());
    std::int32_t configTxnCount = MsgUtil.getNbrTransactionCount();
    bool b1 = (theFunction.nTransNumsNeeded > configTxnCount);
    std::int32_t comparative = 0;

    if (b1) {
        comparative = theFunction.nTransNumsNeeded;
    } else {
        comparative = configTxnCount;
    }

    if (getnym_trnsnum_count < comparative) {
        otOut << strLocation << ", I don't have enough transaction numbers to "
                                "perform this transaction. Grabbing more "
                                "now...\n";
        MsgUtil.setNbrTransactionCount(comparative);
        MsgUtil.getTransactionNumbers(
            String(theFunction.context_.Server()).Get(),
            String(theFunction.context_.Nym()->ID()).Get());
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
        String(theFunction.context_.Server()).Get(),
        String(theFunction.context_.Nym()->ID()).Get());

    if (getnym_trnsnum_count < comparative) {
        otOut
            << strLocation
            << ", third failure: MsgUtil.getTransactionNumbers. (Giving up.)\n";
        return "";
    }

    bool bCanRetryAfterThis = false;

    std::string strResult = SendRequestOnce(
        theFunction, IN_FUNCTION, true, true, bCanRetryAfterThis);

    if (VerifyStringVal(strResult)) {
        otOut << " Getting Intermediary files.. \n";

        if (!MsgUtil.getIntermediaryFiles(
                String(theFunction.context_.Server()).Get(),
                String(theFunction.context_.Nym()->ID()).Get(),
                theFunction.accountID,
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

    std::int32_t nRetries = nTotalRetries;

    while ((nRetries > 0) && !VerifyStringVal(strResult) &&
           bCanRetryAfterThis) {
        --nRetries;

        bool bWillRetryAfterThis = true;

        if ((nRetries == 0) || !bCanRetryAfterThis) {
            bWillRetryAfterThis = false;
        }

        if (exec_.CheckConnection(
                String(theFunction.context_.Server()).Get())) {
            strResult = SendRequestOnce(
                theFunction,
                IN_FUNCTION,
                true,
                bWillRetryAfterThis,
                bCanRetryAfterThis);
        }

        // In case of failure, we want to get these before we re-try.
        // But in case of success, we also want to get these, so we can
        // see the results of our success. So we get these either way...
        //
        if (VerifyStringVal(strResult)) {
            if (!MsgUtil.getIntermediaryFiles(
                    String(theFunction.context_.Server()).Get(),
                    String(theFunction.context_.Nym()->ID()).Get(),
                    theFunction.accountID,
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

std::string OTAPI_Func::SendRequest(
    OTAPI_Func& theFunction,
    const std::string& IN_FUNCTION) const
{
    Utility MsgUtil(context_, otapi_);

    bool bCanRetryAfterThis = false;

    std::string strResult = SendRequestOnce(
        theFunction, IN_FUNCTION, false, true, bCanRetryAfterThis);

    if (!VerifyStringVal(strResult) && bCanRetryAfterThis) {
        if (exec_.CheckConnection(
                String(theFunction.context_.Server()).Get())) {
            strResult = SendRequestOnce(
                theFunction, IN_FUNCTION, false, false, bCanRetryAfterThis);
        }
    }
    return strResult;
}

std::string OTAPI_Func::SendRequestOnce(
    OTAPI_Func& theFunction,
    const std::string& IN_FUNCTION,
    const bool bIsTransaction,
    const bool bWillRetryAfterThis,
    bool& bCanRetryAfterThis) const
{
    Utility MsgUtil(context_, otapi_);
    std::string strLocation = "OTAPI_Func::SendRequestOnce: " + IN_FUNCTION;

    bCanRetryAfterThis = false;

    std::string strReply = "";
    std::int32_t nlocalRequestNum = SendRequestLowLevel(
        theFunction, IN_FUNCTION);  // <========   FIRST ATTEMPT!!!!!!;

    if ((nlocalRequestNum == -1) || (nlocalRequestNum == 0)) {
        // bCanRetryAfterThis is already false here.
        return "";
    } else  // 1 and default.
    {
        if (nlocalRequestNum < -1) {
            return "";
        }

        strReply = MsgUtil.ReceiveReplyLowLevel(
            String(theFunction.context_.Server()).Get(),
            String(theFunction.context_.Nym()->ID()).Get(),
            nlocalRequestNum,
            IN_FUNCTION);  // <==== Here we RECEIVE the REPLY...;
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
                String(theFunction.context_.Server()).Get(),
                String(theFunction.context_.Nym()->ID()).Get(),
                theFunction.accountID,
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
                        String(theFunction.context_.Server()).Get(),
                        String(theFunction.context_.Nym()->ID()).Get(),
                        theFunction.accountID,
                        strReply);

                // If it's not cancelled, then we assume it's a normal
                // transaction (versus a cancellation)
                // and we check for success/failure as normal...
                //
                if (1 != nTransCancelled) {
                    nTransSuccess = exec_.Message_GetTransactionSuccess(
                        String(theFunction.context_.Server()).Get(),
                        String(theFunction.context_.Nym()->ID()).Get(),
                        theFunction.accountID,
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
        // String(theFunction.context_.Server()).Get(),
        // String(theFunction.context_.Nym()->ID()).Get());
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
                        String(theFunction.context_.Server()).Get(),
                        String(theFunction.context_.Nym()->ID()).Get(),
                        theFunction.accountID,
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
                        String(theFunction.context_.Server()).Get(),
                        String(theFunction.context_.Nym()->ID()).Get(),
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
                        String(theFunction.context_.Server()).Get(),
                        String(theFunction.context_.Nym()->ID())
                            .Get());  // FLUSH SENT MESSAGES!!!!  (AND
                                      // HARVEST.);

                    if (VerifyStringVal(strNymbox)) {
                        exec_.FlushSentMessages(
                            false,
                            String(theFunction.context_.Server()).Get(),
                            String(theFunction.context_.Nym()->ID()).Get(),
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

OTAPI_Func::~OTAPI_Func() {}
}  // namespace opentxs
