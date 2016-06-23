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

#include "opentxs/client/OTAPI.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/core/OTStorage.hpp"

#ifndef OT_USE_CXX11
#include <cstdlib>
#endif
#include <string>

#define OT_OTAPI_OT

class the_lambda_struct;

typedef std::map<std::string, opentxs::OTDB::OfferDataNym*> SubMap;
typedef std::map<std::string, SubMap*> MapOfMaps;
typedef int32_t (*LambdaFunc)(const opentxs::OTDB::OfferDataNym& offer_data,
                              int32_t nIndex, const MapOfMaps& map_of_maps,
                              const SubMap& sub_map,
                              the_lambda_struct& extra_vals);

EXPORT OT_OTAPI_OT MapOfMaps* convert_offerlist_to_maps(
    opentxs::OTDB::OfferListNym& offerList);
EXPORT OT_OTAPI_OT int32_t find_strange_offers(
    const opentxs::OTDB::OfferDataNym& offer_data, int32_t nIndex,
    const MapOfMaps& map_of_maps, const SubMap& sub_map,
    the_lambda_struct& extra_vals); // if 10 offers are printed
                                    // for the SAME market,
                                    // nIndex will be 0..9
EXPORT OT_OTAPI_OT int32_t
    iterate_nymoffers_maps(MapOfMaps& map_of_maps,
                           LambdaFunc the_lambda); // low level. map_of_maps
                                                   // must be
                                                   // good. (assumed.)
EXPORT OT_OTAPI_OT int32_t
    iterate_nymoffers_maps(MapOfMaps& map_of_maps, LambdaFunc the_lambda,
                           the_lambda_struct& extra_vals); // low level.
                                                           // map_of_maps
                                                           // must be good.
                                                           // (assumed.)

EXPORT OT_OTAPI_OT int32_t
    iterate_nymoffers_sub_map(const MapOfMaps& map_of_maps, SubMap& sub_map,
                              LambdaFunc the_lambda);

EXPORT OT_OTAPI_OT int32_t
    iterate_nymoffers_sub_map(const MapOfMaps& map_of_maps, SubMap& sub_map,
                              LambdaFunc the_lambda,
                              the_lambda_struct& extra_vals);

EXPORT OT_OTAPI_OT opentxs::OTDB::OfferListNym* loadNymOffers(
    const std::string& notaryID, const std::string& nymID);
EXPORT OT_OTAPI_OT int32_t output_nymoffer_data(
    const opentxs::OTDB::OfferDataNym& offer_data, int32_t nIndex,
    const MapOfMaps& map_of_maps, const SubMap& sub_map,
    the_lambda_struct& extra_vals); // if 10 offers are printed
                                    // for the SAME market,
                                    // nIndex will be 0..9

extern std::string Args;
extern std::string HisAcct;
extern std::string HisNym;
extern std::string HisPurse;
extern std::string MyAcct;
extern std::string MyNym;
extern std::string MyPurse;
extern std::string Server;

typedef enum {
    NO_FUNC = 0,
    REGISTER_NYM = 1,
    DELETE_NYM = 2,
    CHECK_NYM = 3,
    SEND_USER_MESSAGE = 4,
    SEND_USER_INSTRUMENT = 5,
    ISSUE_ASSET_TYPE = 6,
    ISSUE_BASKET = 7,
    CREATE_ASSET_ACCT = 8,
    DELETE_ASSET_ACCT = 9,
    ACTIVATE_SMART_CONTRACT = 10,
    TRIGGER_CLAUSE = 11,
    PROCESS_INBOX = 12,
    EXCHANGE_BASKET = 13,
    DEPOSIT_CASH = 14,
    EXCHANGE_CASH = 15,
    DEPOSIT_CHEQUE = 16,
    WITHDRAW_VOUCHER = 17,
    PAY_DIVIDEND = 18,
    WITHDRAW_CASH = 19,
    GET_CONTRACT = 20,
    SEND_TRANSFER = 21,
    GET_MARKET_LIST = 22,
    CREATE_MARKET_OFFER = 23,
    KILL_MARKET_OFFER = 24,
    KILL_PAYMENT_PLAN = 25,
    DEPOSIT_PAYMENT_PLAN = 26,
    GET_NYM_MARKET_OFFERS = 27,
    GET_MARKET_OFFERS = 28,
    GET_MARKET_RECENT_TRADES = 29,
    GET_MINT = 30,
    GET_BOX_RECEIPT = 32,
    ADJUST_USAGE_CREDITS = 33,
} OTAPI_Func_Type;

class the_lambda_struct
{
public:
    std::vector<std::string> the_vector; // used for returning a list of
                                         // something.
    std::string the_asset_acct;    // for newoffer, we want to remove existing
                                   // offers
                                   // for the same accounts in certain cases.
    std::string the_currency_acct; // for newoffer, we want to remove existing
                                   // offers
                                   // for the same accounts in certain cases.
    std::string the_scale;         // for newoffer as well.
    std::string the_price;         // for newoffer as well.
    bool bSelling;                 // for newoffer as well.

    the_lambda_struct();
};

class OTAPI_Func
{
public:
    OTAPI_Func_Type funcType;
    std::string notaryID;
    std::string nymID;
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
    bool bBool;
    int32_t nData;
    int64_t lData;
    time64_t tData;
    int32_t nTransNumsNeeded;
    int32_t nRequestNum;

    OTAPI_Func();
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID); // 3 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID,
               const std::string& p_strParam); // 4 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_strParam,
               int64_t p_lData); // 5 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_strParam,
               const std::string& p_strData); // 5 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_nymID2,
               const std::string& p_strData,
               const std::string& p_strData2); // 6 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_accountID,
               const std::string& p_strParam, int64_t p_lData,
               const std::string& p_strData2); // 7 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_accountID,
               const std::string& p_strParam, const std::string& p_strData,
               int64_t p_lData2); // 7 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& p_accountID,
               const std::string& p_strParam, const std::string& p_strData,
               const std::string& p_strData2); // 7 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID,
               const std::string& p_instrumentDefinitionID,
               const std::string& p_basket, const std::string& p_accountID,
               bool p_bBool, int32_t p_nTransNumsNeeded); // 8 args
    OTAPI_Func(OTAPI_Func_Type theType, const std::string& p_notaryID,
               const std::string& p_nymID, const std::string& assetAccountID,
               const std::string& currencyAcctID, const std::string& scale,
               const std::string& minIncrement, const std::string& quantity,
               const std::string& price, bool bSelling); // 10 args
    ~OTAPI_Func();

    EXPORT OT_OTAPI_OT static void CopyVariables();
    OT_OTAPI_OT void InitCustom();
    OT_OTAPI_OT int32_t Run() const;
    OT_OTAPI_OT std::string SendRequest(OTAPI_Func& theFunction,
                                        const std::string& IN_FUNCTION) const;
    OT_OTAPI_OT int32_t
        SendRequestLowLevel(OTAPI_Func& theFunction,
                            const std::string& IN_FUNCTION) const;
    OT_OTAPI_OT std::string SendRequestOnce(OTAPI_Func& theFunction,
                                            const std::string& IN_FUNCTION,
                                            bool bIsTransaction,
                                            bool bWillRetryAfterThis,
                                            bool& bCanRetryAfterThis) const;
    OT_OTAPI_OT std::string SendTransaction(OTAPI_Func& theFunction,
                                            const std::string& IN_FUNCTION);
    OT_OTAPI_OT std::string SendTransaction(OTAPI_Func& theFunction,
                                            const std::string& IN_FUNCTION,
                                            int32_t nTotalRetries) const;

private:
    static std::string GetVariable(const char* name);
};

#endif // OPENTXS_CLIENT_OT_OTAPI_OT_HPP
