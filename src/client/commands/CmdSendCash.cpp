/************************************************************
 *
 *  CmdSendCash.cpp
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

#include "CmdSendCash.hpp"

#include "CmdExportCash.hpp"
#include "CmdWithdrawCash.hpp"
#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdSendCash::CmdSendCash()
{
    command = "sendcash";
    args[0] = "[--server <server>]";
    args[1] = "[--mynym <nym>]";
    args[2] = "[--myacct <account>]";
    args[3] = "[--mypurse <purse>]";
    args[4] = "--hisnym <nym>";
    args[5] = "--amount <amount>";
    args[6] = "[--indices <indices|all>]";
    args[7] = "[--password <true|false>]";
    category = catOtherUsers;
    help = "Send cash from mypurse to recipient, withdraw if necessary.";
    usage = "Specify either myacct OR mypurse.\n"
            "When mypurse is specified server and mynym are mandatory.";
}

CmdSendCash::~CmdSendCash()
{
}

int32_t CmdSendCash::runWithOptions()
{
    return run(getOption("server"), getOption("mynym"), getOption("myacct"),
               getOption("mypurse"), getOption("hisnym"), getOption("amount"),
               getOption("indices"), getOption("password"));
}

int32_t CmdSendCash::run(string server, string mynym, string myacct,
                         string mypurse, string hisnym, string amount,
                         string indices, string password)
{
    if ("" != myacct) {
        if (!checkAccount("myacct", myacct)) {
            return -1;
        }

        // myacct specified: server and mynym are implied
        server = OTAPI_Wrap::GetAccountWallet_NotaryID(myacct);
        if ("" == server) {
            otOut << "Error: cannot determine server from myacct.\n";
            return -1;
        }

        mynym = OTAPI_Wrap::GetAccountWallet_NymID(myacct);
        if ("" == mynym) {
            otOut << "Error: cannot determine mynym from myacct.\n";
            return -1;
        }

        string assetType = getAccountAssetType(myacct);
        if ("" == assetType) {
            return -1;
        }

        if ("" != mypurse && mypurse != assetType) {
            otOut << "Error: myacct instrument definition does not match "
                     "mypurse.\n";
            return -1;
        }

        mypurse = assetType;
    }
    else {
        // we want either ONE OF myacct OR mypurse to be specified
        if (!checkMandatory("myacct or mypurse", mypurse)) {
            return -1;
        }

        if (!checkPurse("mypurse", mypurse)) {
            return -1;
        }

        // mypurse specified: server and mynym are mandatory
        if (!checkServer("server", server)) {
            return -1;
        }

        if (!checkNym("mynym", mynym)) {
            return -1;
        }
    }

    if (!checkNym("hisnym", hisnym)) {
        return -1;
    }

    if ("" != password && !checkFlag("password", password)) {
        return -1;
    }

    int64_t value = checkAmount("amount", amount, myacct);
    if (OT_ERROR_AMOUNT == value) {
        return -1;
    }

    // Below this point we can just try to pay it from the purse, and if unable
    // to, try to get the remaining funds from the account, IF that's available.

    // make sure we can access the public key before trying to send cash
    if ("" == MadeEasy::load_or_retrieve_encrypt_key(server, mynym, hisnym)) {
        otOut << "Error: cannot load public key for hisnym.\n";
        return -1;
    }

    string response = "";
    if (1 != sendCash(response, server, mynym, mypurse, myacct, hisnym, amount,
                      indices, password == "true")) {
        return -1;
    }

    cout << response << "\n";

    return 1;
}

int32_t CmdSendCash::sendCash(string& response, const string& server,
                              const string& mynym, const string& assetType,
                              const string& myacct, string& hisnym,
                              const string& amount, string& indices,
                              bool hasPassword) const
{
    int64_t startAmount = "" == amount ? 0 : stoll(amount);

    // What we want to do from here is, see if we can send the cash purely using
    // cash we already have in the local purse. If so, we just package it up and
    // send it off using send_user_payment.
    //
    // But if we do NOT have the proper cash tokens in the local purse to send,
    // then we need to withdraw enough tokens until we do, and then try sending
    // again.

    int64_t remain = startAmount;
    if (!getPurseIndicesOrAmount(server, mynym, assetType, remain, indices)) {
        if ("" != indices) {
            otOut << "Error: invalid purse indices.\n";
            return -1;
        }

        // Not enough cash found in existing purse to match the amount
        CmdWithdrawCash cmd;
        if (1 != cmd.withdrawCash(myacct, remain)) {
            otOut << "Error: cannot withdraw cash.\n";
            return -1;
        }

        remain = startAmount;
        if (!getPurseIndicesOrAmount(server, mynym, assetType, remain,
                                     indices)) {
            otOut << "Error: cannot retrieve purse indices.\n";
            return -1;
        }
    }

    CmdExportCash cmd;
    string retainedCopy = "";
    string exportedCash = cmd.exportCash(server, mynym, assetType, hisnym,
                                         indices, hasPassword, retainedCopy);
    if ("" == exportedCash) {
        otOut << "Error: cannot export cash.\n";
        return -1;
    }

    response = MadeEasy::send_user_cash(server, mynym, hisnym, exportedCash,
                                        retainedCopy);
    if (1 != responseStatus(response)) {
        // cannot send cash so try to re-import into sender's purse
        if (!OTAPI_Wrap::Wallet_ImportPurse(server, assetType, mynym,
                                            retainedCopy)) {
            otOut << "Error: cannot send cash AND failed re-importing purse."
                  << "\nServer: " << server << "\nAsset Type: " << assetType
                  << "\nNym: " << mynym
                  << "\n\nPurse (SAVE THIS SOMEWHERE!):\n\n" << retainedCopy
                  << "\n";
            return -1;
        }

        // at least re-importing succeeeded
        otOut << "Error: cannot send cash.\n";
        return -1;
    }

    return 1;
}

// If you pass the indices, this function returns true if those exact indices
// exist. In that case, this function will also set remain to the total.
//
// If, instead, you pass remain and a blank indices, this function will try to
// determine the indices that would create remain, if they were selected.

bool CmdSendCash::getPurseIndicesOrAmount(const string& server,
                                          const string& mynym,
                                          const string& assetType,
                                          int64_t& remain,
                                          string& indices) const
{
    bool findAmountFromIndices = "" != indices && 0 == remain;
    bool findIndicesFromAmount = "" == indices && 0 != remain;
    if (!findAmountFromIndices && !findIndicesFromAmount) {
        otOut << "Error: invalid parameter combination.\n";
        return false;
    }

    string purse = OTAPI_Wrap::LoadPurse(server, assetType, mynym);
    if ("" == purse) {
        otOut << "Error: cannot load purse.\n";
        return false;
    }

    int32_t items = OTAPI_Wrap::Purse_Count(server, assetType, purse);
    if (0 > items) {
        otOut << "Error: cannot load purse item count.\n\n";
        return false;
    }

    if (0 == items) {
        otOut << "Error: the purse is empty.\n\n";
        return false;
    }

    for (int32_t i = 0; i < items; i++) {
        string token = OTAPI_Wrap::Purse_Peek(server, assetType, mynym, purse);
        if ("" == token) {
            otOut << "Error:cannot load token from purse.\n";
            return false;
        }

        purse = OTAPI_Wrap::Purse_Pop(server, assetType, mynym, purse);
        if ("" == purse) {
            otOut << "Error: cannot load updated purse.\n";
            return false;
        }

        int64_t denomination =
            OTAPI_Wrap::Token_GetDenomination(server, assetType, token);
        if (0 >= denomination) {
            otOut << "Error: cannot get token denomination.\n";
            return false;
        }

        time64_t validTo =
            OTAPI_Wrap::Token_GetValidTo(server, assetType, token);
        if (OT_TIME_ZERO > validTo) {
            otOut << "Error: cannot get token validTo.\n";
            return false;
        }

        time64_t time = OTAPI_Wrap::GetTime();
        if (OT_TIME_ZERO > time) {
            otOut << "Error: cannot get token time.\n";
            return false;
        }

        if (time > validTo) {
            otOut << "Skipping: token is expired.\n";
            continue;
        }

        if (findAmountFromIndices) {
            if ("all" == indices ||
                OTAPI_Wrap::NumList_VerifyQuery(indices, to_string(i))) {
                remain += denomination;
            }
            continue;
        }

        // TODO: There could be a denomination order that will cause this
        // function to fail, even though there is a denomination combination
        // that would make it succeeed. Example: try to find 6 when the
        // denominations are: 5, 2, 2, and 2. This will not succeed since it
        // will use the 5 first and then cannot satisfy the remaining 1 even
        // though the three 2's would satisfy the 6...

        if (denomination <= remain) {
            indices = OTAPI_Wrap::NumList_Add(indices, to_string(i));
            remain -= denomination;
            if (0 == remain) {
                return true;
            }
        }
    }

    return findAmountFromIndices ? true : false;
}
