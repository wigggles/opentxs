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

#ifndef OPENTXS_CLIENT_OT_OTAPI_OT_HPP
#define OPENTXS_CLIENT_OT_OTAPI_OT_HPP

#include "opentxs/Version.hpp"

#include "opentxs/api/Editor.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>

namespace opentxs
{
class Message;
class OT_API;
class OTAPI_Exec;
class PeerReply;
class PeerRequest;
class ServerContext;
class the_lambda_struct;

namespace api
{
namespace client
{
class Wallet;
}  // namespace client
}  // namespace api

typedef std::map<std::string, opentxs::OTDB::OfferDataNym*> SubMap;
typedef std::map<std::string, SubMap*> MapOfMaps;
typedef std::int32_t (*LambdaFunc)(
    const opentxs::OTDB::OfferDataNym& offer_data,
    std::int32_t nIndex,
    const MapOfMaps& map_of_maps,
    const SubMap& sub_map,
    the_lambda_struct& extra_vals);

EXPORT MapOfMaps* convert_offerlist_to_maps(
    opentxs::OTDB::OfferListNym& offerList);
EXPORT std::int32_t find_strange_offers(
    const opentxs::OTDB::OfferDataNym& offer_data,
    std::int32_t nIndex,
    const MapOfMaps& map_of_maps,
    const SubMap& sub_map,
    the_lambda_struct& extra_vals);  // if 10 offers are printed
                                     // for the SAME market,
                                     // nIndex will be 0..9
EXPORT std::int32_t iterate_nymoffers_maps(
    MapOfMaps& map_of_maps,
    LambdaFunc the_lambda);  // low level. map_of_maps
                             // must be
                             // good. (assumed.)
EXPORT std::int32_t iterate_nymoffers_maps(
    MapOfMaps& map_of_maps,
    LambdaFunc the_lambda,
    the_lambda_struct& extra_vals);  // low level.
                                     // map_of_maps
                                     // must be good.
                                     // (assumed.)

EXPORT std::int32_t iterate_nymoffers_sub_map(
    const MapOfMaps& map_of_maps,
    SubMap& sub_map,
    LambdaFunc the_lambda);

EXPORT std::int32_t iterate_nymoffers_sub_map(
    const MapOfMaps& map_of_maps,
    SubMap& sub_map,
    LambdaFunc the_lambda,
    the_lambda_struct& extra_vals);

EXPORT opentxs::OTDB::OfferListNym* loadNymOffers(
    const std::string& notaryID,
    const std::string& nymID);
EXPORT std::int32_t output_nymoffer_data(
    const opentxs::OTDB::OfferDataNym& offer_data,
    std::int32_t nIndex,
    const MapOfMaps& map_of_maps,
    const SubMap& sub_map,
    the_lambda_struct& extra_vals);  // if 10 offers are printed
                                     // for the SAME market,
                                     // nIndex will be 0..9

typedef enum {
    NO_FUNC = 0,
    REMOVED1 = 1,
    REGISTER_NYM = 2,
    DELETE_NYM = 3,
    CHECK_NYM = 4,
    SEND_USER_MESSAGE = 5,
    SEND_USER_INSTRUMENT = 6,
    ISSUE_ASSET_TYPE = 7,
    ISSUE_BASKET = 8,
    CREATE_ASSET_ACCT = 9,
    DELETE_ASSET_ACCT = 10,
    ACTIVATE_SMART_CONTRACT = 11,
    TRIGGER_CLAUSE = 12,
    PROCESS_INBOX = 13,
    EXCHANGE_BASKET = 14,
    DEPOSIT_CASH = 15,
    EXCHANGE_CASH = 16,
    DEPOSIT_CHEQUE = 17,
    WITHDRAW_VOUCHER = 18,
    PAY_DIVIDEND = 19,
    WITHDRAW_CASH = 20,
    GET_CONTRACT = 21,
    SEND_TRANSFER = 22,
    GET_MARKET_LIST = 23,
    CREATE_MARKET_OFFER = 24,
    KILL_MARKET_OFFER = 25,
    KILL_PAYMENT_PLAN = 26,
    DEPOSIT_PAYMENT_PLAN = 27,
    GET_NYM_MARKET_OFFERS = 28,
    GET_MARKET_OFFERS = 29,
    GET_MARKET_RECENT_TRADES = 30,
    GET_MINT = 31,
    GET_BOX_RECEIPT = 32,
    ADJUST_USAGE_CREDITS = 33,
    INITIATE_BAILMENT = 34,
    INITIATE_OUTBAILMENT = 35,
    ACKNOWLEDGE_BAILMENT = 36,
    ACKNOWLEDGE_OUTBAILMENT = 37,
    NOTIFY_BAILMENT = 38,
    ACKNOWLEDGE_NOTICE = 39,
    REQUEST_CONNECTION = 40,
    ACKNOWLEDGE_CONNECTION = 41,
    REGISTER_CONTRACT_NYM = 42,
    REGISTER_CONTRACT_SERVER = 43,
    REGISTER_CONTRACT_UNIT = 44,
    REQUEST_ADMIN = 45,
    SERVER_ADD_CLAIM = 46,
    STORE_SECRET = 47
} OTAPI_Func_Type;

class the_lambda_struct
{
public:
    std::vector<std::string> the_vector;  // used for returning a list of
                                          // something.
    std::string the_asset_acct;     // for newoffer, we want to remove existing
                                    // offers
                                    // for the same accounts in certain cases.
    std::string the_currency_acct;  // for newoffer, we want to remove existing
                                    // offers
                                    // for the same accounts in certain cases.
    std::string the_scale;          // for newoffer as well.
    std::string the_price;          // for newoffer as well.
    bool bSelling{false};           // for newoffer as well.

    the_lambda_struct();
};

class OTAPI_Func
{
public:
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_strParam);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_strParam,
        std::int64_t p_lData);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_strParam,
        const std::string& p_strData);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_nymID2,
        const std::string& p_strData,
        const std::string& p_strData2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_nymID2,
        const std::string& p_strData,
        const bool p_Bool);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_accountID,
        const std::string& p_strParam,
        std::int64_t p_lData,
        const std::string& p_strData2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_accountID,
        const std::string& p_strParam,
        const std::string& p_strData,
        std::int64_t p_lData2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_accountID,
        const std::string& p_strParam,
        const std::string& p_strData,
        const std::string& p_strData2);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        bool p_bBool,
        const std::string& p_strData,
        const std::string& p_strData2,
        const std::string& p_strData3);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& p_instrumentDefinitionID,
        const std::string& p_basket,
        const std::string& p_accountID,
        bool p_bBool,
        std::int32_t p_nTransNumsNeeded);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& accountID,
        const std::string& accountID2,
        const std::string& strData,
        const std::string& strData2,
        const std::string& strData3,
        const std::string& strData4,
        bool bBool);
    explicit OTAPI_Func(
        OTAPI_Func_Type theType,
        api::client::Wallet& wallet,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const std::string& accountID,
        const std::string& accountID2,
        const std::string& strData,
        const std::string& strData2,
        const std::string& strData3,
        const std::string& strData4,
        const bool bBool,
        const time64_t time,
        const std::int64_t lData,
        const std::string& strData5);

    TransactionNumber GetTransactionNumber() const;
    SendResult LastSendResult() const;
    const std::shared_ptr<PeerRequest>& SentPeerRequest() const;
    const std::shared_ptr<PeerReply>& SentPeerReply() const;
    const std::shared_ptr<Message>& Reply() const;

    std::string Run(const std::size_t totalRetries = 2);

    ~OTAPI_Func();

private:
    static const std::map<OTAPI_Func_Type, std::string> type_name_;
    static const std::map<OTAPI_Func_Type, bool> type_type_;

    OTAPI_Func_Type type_{NO_FUNC};
    std::string nymID2;
    std::string instrumentDefinitionID;
    std::string instrumentDefinitionID2;
    std::string accountID;
    std::string accountID2;
    std::string basket;
    std::string strData;
    std::string strData2;
    std::string strData3;
    std::string strData4;
    std::string strData5;
    bool bBool{false};
    std::int32_t nData{0};
    std::int64_t lData{0};
    time64_t tData{OT_TIME_ZERO};
    std::int32_t nTransNumsNeeded{0};
    std::int32_t nRequestNum{-1};
    api::client::Wallet& wallet_;
    Editor<ServerContext> context_editor_;
    ServerContext& context_;
    const OTAPI_Exec& exec_;
    const OT_API& otapi_;
    mutable std::mutex lock_;
    CommandResult last_attempt_;
    const bool is_transaction_{false};
    std::shared_ptr<PeerReply> peer_reply_;
    std::shared_ptr<PeerRequest> peer_request_;

    void run();
    std::int32_t send();
    std::string send_once(
        const bool bIsTransaction,
        const bool bWillRetryAfterThis,
        bool& bCanRetryAfterThis);
    std::string send_request();
    std::string send_transaction(const std::size_t totalRetries);

    explicit OTAPI_Func(
        api::client::Wallet& wallet,
        const OTAPI_Exec& exec,
        const OT_API& otapi,
        const Identifier& nymID,
        const Identifier& serverID,
        const OTAPI_Func_Type type);
    OTAPI_Func() = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OT_OTAPI_OT_HPP
