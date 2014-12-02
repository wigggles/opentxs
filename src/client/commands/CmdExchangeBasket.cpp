/************************************************************
 *
 *  CmdExchangeBasket.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include "CmdExchangeBasket.hpp"

#include "CmdShowBasket.hpp"
#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdExchangeBasket::CmdExchangeBasket()
{
    command = "exchangebasket";
    args[0] = "--myacct <basketaccount>";
    args[1] = "[--direction <in|out> (default in)]";
    args[2] = "[--multiple <multiple> (default 1)]";
    category = catBaskets;
    help = "Exchange in or out of a basket currency.";
}

CmdExchangeBasket::~CmdExchangeBasket()
{
}

int32_t CmdExchangeBasket::runWithOptions()
{
    return run(getOption("myacct"), getOption("direction"),
               getOption("multiple"));
}

int32_t CmdExchangeBasket::run(string myacct, string direction, string multiple)
{
    if (!checkAccount("myacct", myacct)) {
        return -1;
    }

    if ("" != direction && "in" != direction && "out" != direction) {
        otOut << "Error: direction: expected 'in' or 'out'.\n";
        return -1;
    }

    if ("" != multiple && !checkValue("multiple", multiple)) {
        return -1;
    }

    int32_t multiplier = "" != multiple ? stol(multiple) : 1;
    if (1 > multiplier) {
        otOut << "Error: multiple: invalid value: " << multiple << "\n";
        return -1;
    }

    string server = OTAPI_Wrap::GetAccountWallet_NotaryID(myacct);
    if ("" == server) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string mynym = getAccountNym(myacct);
    if ("" == mynym) {
        return -1;
    }

    string assetType = getAccountAssetType(myacct);
    if ("" == assetType) {
        return -1;
    }

    if (!OTAPI_Wrap::IsBasketCurrency(assetType)) {
        otOut << "Error: account is not a basket currency.\n";
        return -1;
    }

    int32_t members = OTAPI_Wrap::Basket_GetMemberCount(assetType);
    if (2 > members) {
        otOut << "Error: cannot load basket member count.\n";
        return -1;
    }

    int64_t minAmount = OTAPI_Wrap::Basket_GetMinimumTransferAmount(assetType);
    if (0 > minAmount) {
        otOut << "Error: cannot load minimum transfer amount for basket.\n";
        return -1;
    }

    int64_t balance = OTAPI_Wrap::GetAccountWallet_Balance(myacct);
    if (OT_ERROR_AMOUNT == balance) {
        otOut << "Error: cannot retrieve balance for basket account.\n";
        return -1;
    }

    int64_t amount = multiplier * minAmount;

    bool bExchangingIn = "out" != direction;
    if (!bExchangingIn) {
        if (balance < minAmount) {
            otOut << "Error: balance (" << balance
                  << ") is less than minimum amount (" << minAmount << ").\n";
            return -1;
        }

        if (amount > balance) {
            otOut << "Error: balance (" << balance
                  << ") is insufficient for transfer amount (" << amount
                  << ").\n";
            return -1;
        }
    }

    if (!MadeEasy::insure_enough_nums(20, server, mynym)) {
        otOut << "Error: cannot reserve transaction numbers.\n";
        return -1;
    }

    string basket = OTAPI_Wrap::GenerateBasketExchange(server, mynym, assetType,
                                                       myacct, multiplier);
    if ("" == basket) {
        otOut << "Error: cannot generate basket exchange.\n";
        return -1;
    }

    // Sub-currencies!
    for (int32_t member = 0; member < members; member++) {
        string memberType = OTAPI_Wrap::Basket_GetMemberType(assetType, member);
        if ("" == memberType) {
            otOut << "Error: cannot load basket member type.\n";
            return harvestTxNumbers(basket, mynym);
        }

        int64_t memberAmount =
            OTAPI_Wrap::Basket_GetMemberMinimumTransferAmount(assetType,
                                                              member);
        if (0 > memberAmount) {
            otOut << "Error: cannot load basket member minimum transfer "
                     "amount.\n";
            return harvestTxNumbers(basket, mynym);
        }

        amount = multiplier * memberAmount;

        if (0 > showBasketAccounts(server, mynym, memberType, true)) {
            return harvestTxNumbers(basket, mynym);
        }

        string memberTypeName = OTAPI_Wrap::GetAssetType_Name(memberType);
        otOut << "\nThere are " << (members - member)
              << " accounts remaining to be selected.\n\n";
        otOut << "Currently we need to select an account with the instrument "
                 "definition:\n" << memberType << " (" << memberTypeName
              << ")\n";
        otOut << "Above are all the accounts in the wallet, for the relevant "
                 "server and nym, of that instrument definition.\n";

        if (bExchangingIn) {
            otOut << "\nKeep in mind, with a transfer multiple of "
                  << multiplier << " and a minimum transfer amount of "
                  << memberAmount
                  << "\n(for this sub-currency), you must therefore select an "
                     "account with a minimum\nbalance of: " << amount << "\n";
        }

        otOut << "\nPlease PASTE an account ID from the above list: ";
        string account = inputLine();
        if ("" == account) {
            otOut << "Error: invalid account ID.\n";
            return harvestTxNumbers(basket, mynym);
        }

        string subAssetType =
            OTAPI_Wrap::GetAccountWallet_InstrumentDefinitionID(account);
        if ("" == subAssetType) {
            otOut << "Error: cannot load account instrument definition.\n";
            return harvestTxNumbers(basket, mynym);
        }

        if (memberType != subAssetType) {
            otOut << "Error: incorrect account instrument definition.\n";
            return harvestTxNumbers(basket, mynym);
        }

        balance = OTAPI_Wrap::GetAccountWallet_Balance(account);
        if (OT_ERROR_AMOUNT == balance) {
            otOut << "Error: cannot load account balance.\n";
            return harvestTxNumbers(basket, mynym);
        }

        if (bExchangingIn && amount > balance) {
            otOut << "Error: account balance (" << balance
                  << ") is insufficient for transfer amount (" << amount
                  << ").\n";
            return harvestTxNumbers(basket, mynym);
        }

        string newBasket = OTAPI_Wrap::AddBasketExchangeItem(
            server, mynym, basket, subAssetType, account);
        if ("" == newBasket) {
            otOut << "Error: cannot add basket exchange item.\n";
            return harvestTxNumbers(basket, mynym);
        }

        basket = newBasket;
    }

    string response = MadeEasy::exchange_basket_currency(
        server, mynym, assetType, basket, myacct, bExchangingIn);
    int32_t reply =
        responseReply(response, server, mynym, myacct, "exchange_basket");
    if (1 != reply) {
        return reply;
    }

    if (!MadeEasy::retrieve_account(server, mynym, myacct, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    return 1;
}

// Used by exchange_basket for displaying certain types of accounts.
//
// if assetType doesn't exist, it will ONLY show accounts that are basket
// currencies.
// if assetType exists, and bFilter is TRUE, it will ONLY show accounts of
// that type.
// if assetType exists, and bFilter is FALSE, it will only show accounts
// that are NOT of that type.
//
// Also: if server exists, the accounts are filtered by that server.
// Also: if mynym exists, the accounts are filtered by that Nym.
//
int32_t CmdExchangeBasket::showBasketAccounts(const string& server,
                                              const string& mynym,
                                              const string& assetType,
                                              bool bFilter)
{
    int32_t items = OTAPI_Wrap::GetAccountCount();
    if (0 > items) {
        otOut << "Error: cannot load account count.\n";
        return -1;
    }

    dashLine();
    cout << " ** ACCOUNTS:\n\n";

    for (int32_t i = 0; i < items; i++) {
        string acct = OTAPI_Wrap::GetAccountWallet_ID(i);
        if ("" == acct) {
            otOut << "Error: cannot load account ID.\n";
            return -1;
        }

        string accountServer = OTAPI_Wrap::GetAccountWallet_NotaryID(acct);
        if ("" == accountServer) {
            otOut << "Error: cannot determine server from myacct.\n";
            return -1;
        }

        if ("" == server || server == accountServer) {
            string accountNym = getAccountNym(acct);
            if ("" == accountNym) {
                return -1;
            }

            if ("" == mynym || mynym == accountNym) {
                string asset = getAccountAssetType(acct);
                if ("" == asset) {
                    return -1;
                }

                if (("" == assetType && OTAPI_Wrap::IsBasketCurrency(asset)) ||
                    ("" != assetType && bFilter && assetType == asset) ||
                    ("" != assetType && !bFilter && assetType != asset)) {
                    string statAccount = MadeEasy::stat_asset_account(acct);
                    if ("" == statAccount) {
                        otOut << "Error: cannot stat account.\n";
                        return -1;
                    }

                    if (0 < i) {
                        cout << "-------------------------------------\n";
                    }
                    cout << statAccount << "\n";
                }
            }
        }
    }
    dashLine();

    return 1;
}
