/************************************************************
 *
 *  CmdShowRecords.cpp
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

#include "CmdShowRecords.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdShowRecords::CmdShowRecords()
{
    command = "showrecords";
    args[0] = "[--server <server>]";
    args[1] = "[--mynym <nym>]";
    args[2] = "[--myacct <account>]";
    category = catMisc;
    help = "Show contents of record box.";
    usage = "Specify either one of --server/--mynym and --myacct.";
}

CmdShowRecords::~CmdShowRecords()
{
}

int32_t CmdShowRecords::runWithOptions()
{
    return run(getOption("server"), getOption("mynym"), getOption("myacct"));
}

int32_t CmdShowRecords::run(string server, string mynym, string myacct)
{
    if ("" == myacct) {
        if (!checkServer("server", server)) {
            return -1;
        }

        if (!checkNym("mynym", mynym)) {
            return -1;
        }
    }
    else {
        if (!checkAccount("myacct", myacct)) {
            return -1;
        }

        server = getAccountServer(myacct);
        if ("" == server) {
            return -1;
        }

        mynym = getAccountNym(myacct);
        if ("" == mynym) {
            return -1;
        }
    }

    // REMEMBER, recordbox for myacct contains the old inbox receipts.
    // Whereas recordbox for MyNym contains the old payments (in and out.)
    // In the case of the latter, the NYM must be passed as the ACCT...
    //
    // Meaning: there are TWO record boxes. So must there thus be two commands
    // for viewing them? Or do we combine them somehow?
    ///
    // ===> I say combine them, since they are for viewing only (nothing is
    // actually done with the records -- they're just there for the actual
    // client to take and store however it wishes.)

    otOut << "Archived Nym-related records (" << mynym << "):\n";
    bool success = 0 <= showRecords(server, mynym, mynym);

    if ("" != myacct) {
        dashLine();
        otOut << "Archived Account-related records (" << myacct << "):\n";
        success |= 0 <= showRecords(server, mynym, myacct);
    }

    return success ? 1 : -1;
}

int32_t CmdShowRecords::showRecords(const string& server, const string& mynym,
                                    const string& myacct)
{
    string records = OTAPI_Wrap::LoadRecordBox(server, mynym, myacct);
    if ("" == records) {
        otOut << "Error: cannot load record box.\n";
        return -1;
    }

    int32_t items = OTAPI_Wrap::Ledger_GetCount(server, mynym, myacct, records);
    if (0 > items) {
        otOut << "Error: cannot load record box item count.\n";
        return -1;
    }

    if (0 == items) {
        otOut << "The record box is empty.\n";
        return 0;
    }

    cout << "Idx  Amt  Type        Txn# InRef#|User / Acct\n";
    cout << "---------------------------------|(from or to)\n";
    for (int32_t i = 0; i < items; i++) {
        string tx = OTAPI_Wrap::Ledger_GetTransactionByIndex(
            server, mynym, myacct, records, i);
        int64_t txNum = OTAPI_Wrap::Ledger_GetTransactionIDByIndex(
            server, mynym, myacct, records, i);
        int64_t refNum = OTAPI_Wrap::Transaction_GetDisplayReferenceToNum(
            server, mynym, myacct, tx);
        int64_t amount =
            OTAPI_Wrap::Transaction_GetAmount(server, mynym, myacct, tx);
        string type =
            OTAPI_Wrap::Transaction_GetType(server, mynym, myacct, tx);
        string senderUser =
            OTAPI_Wrap::Transaction_GetSenderNymID(server, mynym, myacct, tx);
        string senderAcct =
            OTAPI_Wrap::Transaction_GetSenderAcctID(server, mynym, myacct, tx);
        string recipientUser = OTAPI_Wrap::Transaction_GetRecipientNymID(
            server, mynym, myacct, tx);
        string recipientAcct = OTAPI_Wrap::Transaction_GetRecipientAcctID(
            server, mynym, myacct, tx);

        string user = "" != senderUser ? senderUser : recipientUser;
        string acct = "" != senderAcct ? senderAcct : recipientAcct;

        string separator = ("" == user && "" == acct)
                               ? ""
                               : "\n                                 |";

        string userDenoter = "" != user ? "U:" : "";
        string acctDenoter = "" != acct ? "A:" : "";

        string assetType = "" != acct ? getAccountAssetType(acct) : "";
        string fmtAmount = formatAmount(assetType, amount);

        cout << i << "    ";
        cout << fmtAmount << (fmtAmount.size() < 3 ? "    " : "   ");
        cout << type << (type.size() > 10 ? " " : "    ");
        cout << txNum << (to_string(txNum).size() < 2 ? "    " : "   ");
        cout << refNum << (to_string(refNum).size() > 2 ? "  " : " ") << "|";
        cout << userDenoter << user << separator << acctDenoter << acct << "\n";
    }

    return 1;
}
