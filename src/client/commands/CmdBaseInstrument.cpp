/************************************************************
 *
 *  CmdBaseInstrument.cpp
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

#include "CmdBaseInstrument.hpp"

#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/OTLog.hpp>

using namespace opentxs;
using namespace std;

CmdBaseInstrument::CmdBaseInstrument()
{
}

CmdBaseInstrument::~CmdBaseInstrument()
{
}

int32_t CmdBaseInstrument::getTokens(vector<string>& tokens,
                                     const string& server, const string& mynym,
                                     const string& assetType, string purse,
                                     const string& indices) const
{

    if ("" == indices) {
        return 1;
    }

    int32_t items = OTAPI_Wrap::Purse_Count(server, assetType, purse);
    if (0 > items) {
        otOut << "Error: cannot load purse item count.\n";
        return -1;
    }

    if (1 > items) {
        otOut << "Error: The purse is empty.\n";
        return -1;
    }

    bool all = "all" == indices;
    for (int32_t i = 0; i < items; i++) {
        string token = OTAPI_Wrap::Purse_Peek(server, assetType, mynym, purse);
        if ("" == token) {
            otOut << "Error:cannot load token from purse.\n";
            return -1;
        }

        purse = OTAPI_Wrap::Purse_Pop(server, assetType, mynym, purse);
        if ("" == purse) {
            otOut << "Error: cannot load updated purse.\n";
            return -1;
        }

        string tokenID = OTAPI_Wrap::Token_GetID(server, assetType, token);
        if ("" == tokenID) {
            otOut << "Error: cannot get token ID.\n";
            return -1;
        }

        if (!all && OTAPI_Wrap::NumList_VerifyQuery(indices, to_string(i))) {
            tokens.push_back(tokenID);
        }
    }

    return 1;
}

int32_t CmdBaseInstrument::sendPayment(const string& cheque, string sender,
                                       const char* what) const
{
    string server = OTAPI_Wrap::Instrmnt_GetNotaryID(cheque);
    if ("" == server) {
        otOut << "Error: cannot get server.\n";
        return -1;
    }

    if ("" == sender) {
        sender = OTAPI_Wrap::Instrmnt_GetSenderNymID(cheque);
        if ("" == sender) {
            otOut << "Error: cannot get sender.\n";
            return -1;
        }
    }

    string recipient = OTAPI_Wrap::Instrmnt_GetRecipientNymID(cheque);
    if ("" == recipient) {
        otOut << "Error: cannot get recipient.\n";
        return -1;
    }

    string response =
        MadeEasy::send_user_payment(server, sender, recipient, cheque);
    return processResponse(response, what);
}

string CmdBaseInstrument::writeCheque(string myacct, string hisnym,
                                      string amount, string memo,
                                      string validfor, bool isInvoice) const
{
    if (!checkAccount("myacct", myacct)) {
        return "";
    }

    if ("" != hisnym && !checkNym("hisnym", hisnym)) {
        return "";
    }

    int64_t value = checkAmount("amount", amount, myacct);
    if (OT_ERROR_AMOUNT == value) {
        return "";
    }

    if ("" != validfor && !checkValue("validfor", validfor)) {
        return "";
    }

    string server = getAccountServer(myacct);
    if ("" == server) {
        return "";
    }

    string mynym = getAccountNym(myacct);
    if ("" == mynym) {
        return "";
    }

    // make sure we can access the public key before trying to write a cheque
    if ("" != hisnym) {
        if (MadeEasy::load_or_retrieve_encrypt_key(server, mynym, hisnym) ==
            "") {
            otOut << "Error: cannot load public key for hisnym.\n";
            return "";
        }
    }

    if (!MadeEasy::insure_enough_nums(10, server, mynym)) {
        otOut << "Error: cannot reserve transaction numbers.\n";
        return "";
    }

    int64_t oneMonth = OTTimeGetSecondsFromTime(OT_TIME_MONTH_IN_SECONDS);
    int64_t timeSpan = "" != validfor ? stoll(validfor) : oneMonth;
    time64_t from = OTAPI_Wrap::GetTime();
    time64_t until = OTTimeAddTimeInterval(from, timeSpan);

    string cheque =
        OTAPI_Wrap::WriteCheque(server, isInvoice ? -value : value, from, until,
                                myacct, mynym, memo, hisnym);
    if ("" == cheque) {
        otOut << "Error: cannot write cheque.\n";
        return "";
    }

    // Record it in the records?
    // Update: We wouldn't record that here. Instead, OTAPI_Wrap::WriteCheque
    // should drop a notice into the payments outbox, the same as it does when
    // you "sendcheque" (after all, the same resolution would be expected once
    // it is cashed.)

    return cheque;
}
