/************************************************************
 *
 *  CmdBaseAccept.cpp
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

#include "CmdBaseAccept.hpp"

#include "CmdPayInvoice.hpp"
#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/client/OT_ME.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

/*
call OTAPI_Wrap::LoadInbox() to load the inbox ledger from local storage.

During this time, your user has the opportunity to peruse the inbox, and to
decide which transactions therein he wishes to accept or reject. Usually the
inbox is displayed on the screen, then the user selects various items to accept
or reject, and then the user clicks “Process Inbox” and then you do this:
Then call OTAPI_Wrap::Ledger_CreateResponse() in order to create a ‘response’
ledger for that inbox, which will be sent to the server to signal your responses
to the various inbox transactions.
Then call OTAPI_Wrap::Ledger_GetCount() (pass it the inbox) to find out how many
transactions are inside of it. Use that count to LOOP through them…
Use OTAPI_Wrap::Ledger_GetTransactionByIndex() to grab each transaction as you
iterate through the inbox. (There are various introspection functions you can
use in the API here if you wish to display the inbox items on the screen for the
user…)
Next call OTAPI_Wrap::Transaction_CreateResponse() for each transaction in the
inbox, to create a response to it, accepting or rejecting it. This function
creates the response and adds it to the response ledger.
Next, call OTAPI_Wrap::Ledger_FinalizeResponse() which will create a Balance
Agreement for the ledger.
Finally, call OTAPI_Wrap::processInbox() to send your message to the server and
process the various items.

If the message was successful, then use
OTAPI_Wrap::Message_GetBalanceAgreementSuccess() and
OTAPI_Wrap::Message_GetTransactionSuccess() as described above in the deposit
cash instructions.
*/

// Done:  add options here for accept transfers, accept receipts, and accept
// all.
// Done:  Then basically make a version for the payments inbox for accept
// payments, accept invoices, and accept all.
//
// (Accepting payments can basically be automated, but accepting invoices
// requires user permission.)
//
// Therefore add:
//   acceptmoney    -- This accepts all incoming transfers and incoming payments
//                     (Not receipts or invoices) for any designated accounts
// and nyms.
//   acceptreceipts -- Accepts all inbox receipts (not transfers.)
//   acceptinvoices -- Accepts all invoices (not payments.)
//   acceptall      -- All of the above.
//
// Todo: Make the above functions also work with specific indices (vs "all")
//

//
// PROCESS INBOX, ACCEPTING ALL ITEMS WITHIN...
//
// Load an asset account's inbox from local storage and iterate through
// the items inside, and fire off a server message accepting them all.
//
// itemType == 0 for all, 1 for transfers only, 2 for receipts only.
// "" == indices for "all indices"
//
int32_t CmdBaseAccept::acceptFromInbox(const string& myacct,
                                       const string& indices,
                                       const int32_t itemType) const
{
    string server = OTAPI_Wrap::GetAccountWallet_NotaryID(myacct);
    if ("" == server) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string mynym = OTAPI_Wrap::GetAccountWallet_NymID(myacct);
    if ("" == mynym) {
        otOut << "Error: cannot determine mynym from myacct.\n";
        return -1;
    }

    if (!MadeEasy::retrieve_account(server, mynym, myacct, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    // NOTE: Normally we don't have to do this, because the high-level API is
    // smart enough, when sending server transaction requests, to grab new
    // transaction numbers if it is running low. But in this case, we need the
    // numbers available BEFORE sending the transaction request, because the
    // call to OTAPI_Wrap::Ledger_CreateResponse is where the number is first
    // needed, and that call is made before the server transaction request is
    // actually sent.
    OT_ME ot_me;
    if (!ot_me.make_sure_enough_trans_nums(10, server, mynym)) {
        otOut << "Error: cannot reserve transaction numbers.\n";
        return -1;
    }

    string inbox = OTAPI_Wrap::LoadInbox(server, mynym, myacct);
    if ("" == inbox) {
        otOut << "Error: cannot load inbox.\n";
        return -1;
    }

    int32_t items = OTAPI_Wrap::Ledger_GetCount(server, mynym, myacct, inbox);
    if (0 > items) {
        otOut << "Error: cannot load inbox item count.\n";
        return -1;
    }

    if (0 == items) {
        otOut << "The inbox is empty.\n";
        return 0;
    }

    if (!checkIndicesRange("indices", indices, items)) {
        return -1;
    }

    bool all = "" == indices || "all" == indices;

    string ledger = "";
    for (int32_t i = 0; i < items; i++) {
        string tx = OTAPI_Wrap::Ledger_GetTransactionByIndex(server, mynym,
                                                             myacct, inbox, i);

        // itemType == 0 for all, 1 for transfers only, 2 for receipts only.
        if (0 != itemType) {
            string type =
                OTAPI_Wrap::Transaction_GetType(server, mynym, myacct, tx);
            bool transfer = "pending" == type;
            if (1 == itemType && !transfer) {
                // not a transfer.
                continue;
            }
            if (2 == itemType && transfer) {
                // not a receipt.
                continue;
            }
        }

        // do we want this index?
        if (!all && !OTAPI_Wrap::NumList_VerifyQuery(indices, to_string(i))) {
            continue;
        }

        // need to create response ledger?
        if ("" == ledger) {
            ledger =
                OTAPI_Wrap::Ledger_CreateResponse(server, mynym, myacct, inbox);
            if ("" == ledger) {
                otOut << "Error: cannot create response ledger.\n";
                return -1;
            }
        }

        ledger = OTAPI_Wrap::Transaction_CreateResponse(server, mynym, myacct,
                                                        ledger, tx, true);
        if ("" == ledger) {
            otOut << "Error: cannot create transaction response.\n";
            return -1;
        }
    }

    if ("" == ledger) {
        // did not process anything
        return 0;
    }

    string response =
        OTAPI_Wrap::Ledger_FinalizeResponse(server, mynym, myacct, ledger);
    if ("" == response) {
        otOut << "Error: cannot finalize response.\n";
        return -1;
    }

    response = MadeEasy::process_inbox(server, mynym, myacct, response);
    int32_t reply =
        responseReply(response, server, mynym, myacct, "process_inbox");
    if (1 != reply) {
        return reply;
    }

    if (!MadeEasy::retrieve_account(server, mynym, myacct, true)) {
        otOut << "Error retrieving intermediary files for account.\n";
        return -1;
    }

    return 1;
}

int32_t CmdBaseAccept::acceptFromPaymentbox(const string& myacct,
                                            const string& indices,
                                            const string& paymentType) const
{
    if ("" == myacct) {
        otOut << "Error: myacct is empty.\n";
        return -1;
    }

    string server = OTAPI_Wrap::GetAccountWallet_NotaryID(myacct);
    if ("" == server) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string mynym = OTAPI_Wrap::GetAccountWallet_NymID(myacct);
    if ("" == mynym) {
        otOut << "Error: cannot determine mynym from myacct.\n";
        return -1;
    }

    string inbox = OTAPI_Wrap::LoadPaymentInbox(server, mynym);
    if ("" == inbox) {
        otOut << "Error: cannot load payment inbox.\n";
        return -1;
    }

    int32_t items = OTAPI_Wrap::Ledger_GetCount(server, mynym, mynym, inbox);
    if (0 > items) {
        otOut << "Error: cannot load payment inbox item count.\n";
        return -1;
    }

    if (!checkIndicesRange("indices", indices, items)) {
        return -1;
    }

    if (0 == items) {
        otOut << "The payment inbox is empty.\n";
        return 0;
    }

    bool all = "" == indices || "all" == indices;
    for (int32_t i = items - 1; 0 <= i; i--) {
        if (all || OTAPI_Wrap::NumList_VerifyQuery(indices, to_string(i))) {
            CmdPayInvoice payInvoice;
            payInvoice.processPayment(myacct, paymentType, inbox, i);
        }
    }

    return 1;
}
