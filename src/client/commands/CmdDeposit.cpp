/************************************************************
 *
 *  CmdDeposit.cpp
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

#include "CmdDeposit.hpp"

#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/OTLog.hpp>

using namespace opentxs;
using namespace std;

CmdDeposit::CmdDeposit()
{
    command = "deposit";
    args[0] = "--myacct <account>";
    args[1] = "[--mynym <nym>]";
    args[2] = "[--indices <indices|all>]";
    category = catAccounts;
    help = "Deposit cash, cheque, voucher, or invoice.";
    usage =
        "Any supplied indices must correspond to tokens in your cash purse.";
}

CmdDeposit::~CmdDeposit()
{
}

int32_t CmdDeposit::runWithOptions()
{
    return run(getOption("mynym"), getOption("myacct"), getOption("indices"));
}

// A bit complicated:
//
// If I specify MyPurse and MyAcct, then they MUST have the same instrument
// definition.
// If I specify MyNym and MyPurse, that is where we will look for the purse.
// If I specify MyAcct, and it's owned by a different Nym than MyNym, then
// the cash tokens will be reassigned from MyNym to MyAcct's Nym, before
// depositing.
// Basically ALWAYS look up MyAcct's owner, and set HIM as the recipient Nym.
// (But still use MyNym, independently, to find the purse being deposited.)
//
// Must ALWAYS specify MyAcct because otherwise, where are you depositing to?
//
// If MyNym isn't available, should use MyAcct's Nym.
//
// Shouldn't need to specify MyPurse, since we can ONLY deposit into MyAcct of
// the same type as MyAcct. Thus we should ignore any other instrument
// definitions or
// purses since they couldn't possibly be deposited into MyAcct anyway.

int32_t CmdDeposit::run(string mynym, string myacct, string indices)
{
    if (!checkAccount("myacct", myacct)) {
        return -1;
    }

    string server = getAccountServer(myacct);
    if ("" == server) {
        return -1;
    }

    string toNym = getAccountNym(myacct);
    if ("" == toNym) {
        return -1;
    }

    if ("" != indices) {
        // Only in the case of cash, it's possible you have some cash in Nym A's
        // purse, but you want to deposit it into Nym B's account. So we have a
        // "to" Nym and a "from" Nym even though they will often be the same.
        string fromNym = toNym;
        if ("" != mynym) {
            if (!checkNym("mynym", mynym)) {
                return -1;
            }
            fromNym = mynym;
        }

        // In this case, instrument is blank. That's how the callee knows that
        // we're working with the local purse. Then indices tells him either to
        // use "all" tokens in that purse, or the selected indices.
        return depositPurse(server, myacct, fromNym, "", indices);
    }

    otOut << "You can deposit a PURSE (containing cash tokens) or "
             "a CHEQUE / INVOICE / VOUCHER.\n";

    string instrument = inputText("your financial instrument");
    if ("" == instrument) {
        return -1;
    }

    string type = OTAPI_Wrap::Instrmnt_GetType(instrument);
    if ("PURSE" == type) {
        return depositPurse(server, myacct, toNym, instrument, "");
    }

    if ("CHEQUE" == type || "INVOICE" == type || "VOUCHER" == type) {
        return depositCheque(server, myacct, toNym, instrument);
    }

    otOut << "Error: cannot determine instrument type.\n"
             "Expected CHEQUE, VOUCHER, INVOICE, or (cash) PURSE.\n";
    return -1;
}

// THESE FUNCTIONS were added for the PAYMENTS screen. (They are fairly new.)
//
// Basically there was a need to have DIFFERENT instruments, but to be able to
// treat them as though they are a single type.
//
// In keeping with that, the below functions will work with disparate types.
// You can pass [ CHEQUES / VOUCHERS / INVOICES ] and PAYMENT PLANS, and
// SMART CONTRACTS, and PURSEs into these functions, and they should be able
// to handle any of those types.

int32_t CmdDeposit::depositCheque(const string& server, const string& myacct,
                                  const string& mynym,
                                  const string& instrument) const
{
    string assetType = getAccountAssetType(myacct);
    if ("" == assetType) {
        return -1;
    }

    if (assetType !=
        OTAPI_Wrap::Instrmnt_GetInstrumentDefinitionID(instrument)) {
        otOut << "Error: instrument definitions of instrument and myacct do "
                 "not match.\n";
        return -1;
    }

    string response =
        MadeEasy::deposit_cheque(server, mynym, myacct, instrument);
    int32_t reply =
        responseReply(response, server, mynym, myacct, "deposit_cheque");
    if (1 != reply) {
        return reply;
    }

    if (!MadeEasy::retrieve_account(server, mynym, myacct, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    return 1;
}

int32_t CmdDeposit::depositPurse(const string& server, const string& myacct,
                                 const string& mynym, string instrument,
                                 const string& indices) const
{
    string assetType = getAccountAssetType(myacct);
    if ("" == assetType) {
        return -1;
    }

    if ("" != instrument) {
        vector<string> tokens;
        return MadeEasy::depositCashPurse(server, assetType, mynym, instrument,
                                          tokens, myacct, false);
    }

    // we have to load the purse ourselves
    instrument = OTAPI_Wrap::LoadPurse(server, assetType, mynym);
    if ("" == instrument) {
        otOut << "Error: cannot load purse.\n";
        return -1;
    }

    vector<string> tokens;
    if (0 > getTokens(tokens, server, mynym, assetType, instrument, indices)) {
        return -1;
    }

    return MadeEasy::depositCashPurse(server, assetType, mynym, instrument,
                                      tokens, myacct, true);
}
