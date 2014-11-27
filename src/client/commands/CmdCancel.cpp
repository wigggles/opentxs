/************************************************************
 *
 *  CmdCancel.cpp
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

#include "CmdCancel.hpp"

#include "CmdDeposit.hpp"
#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdCancel::CmdCancel()
{
    command = "cancel";
    args[0] = "--mynym <nym>";
    args[1] = "[--myacct <account>]";
    args[2] = "--indices <indices|all>";
    category = catInstruments;
    help = "Cancel an uncashed outgoing instrument from outpayment box.";
    usage = "Specify --myacct when canceling a smart contract.";
}

CmdCancel::~CmdCancel()
{
}

int32_t CmdCancel::runWithOptions()
{
    return run(getOption("mynym"), getOption("myacct"), getOption("indices"));
}

// Also TODO:
// Update the "cancel" command, for outgoing cash, to give you a choice to
// deposit the cash instead
// of discarding it.
// CmdDeposit::depositPurse(strNotaryID, strMyAcctID, strToNymID, strInstrument,
// "") // strIndices is left blank in this case

// NOTE: You can't just discard a sent cheque  from your outpayment box.
// Why not? Just because you remove your record of the outgoing cheque,
// doesn't mean you didn't already send it. (The recipient still received
// it, and still has it, whether you remove it from your outbox or not.)
// If you really want to cancel the cheque, then you need to do it in such
// a way that it will fail-as-cancelled when the recipient tries to deposit
// it. Otherwise, he would get that money whether you kept your own outgoing
// record or not. Therefore SOME server message must be performed here,
// which actually cancels the transaction number itself that appears on the
// cheque. This is the only way to insure that the cheque can't be used by
// the recipient (and even this will only work if you beat him to the punch.
// If he deposits it before you cancel it, then it's already too late and he
// has the money.) THIS is why RecordPayment, regarding outpayments, only
// works on expired instruments -- because if it's not expired, you don't
// just want to remove your record of it. You want to cancel the transaction
// number itself -- and that requires server communication.

int32_t CmdCancel::run(string mynym, string myacct, string indices)
{
    if (!checkNym("mynym", mynym)) {
        return -1;
    }

    if ("" != myacct && !checkAccount("myacct", myacct)) {
        return -1;
    }

    if (!checkIndices("indices", indices)) {
        return -1;
    }

    int32_t items = OTAPI_Wrap::GetNym_OutpaymentsCount(mynym);
    if (0 > items) {
        otOut << "Error: cannot load payment outbox item count.\n";
        return -1;
    }

    if (0 == items) {
        otOut << "The payment outbox is empty.\n";
        return 0;
    }

    bool all = "all" == indices;

    // Loop from back to front, in case any are removed.
    int32_t retVal = 1;
    for (int32_t i = items - 1; 0 <= i; i--) {
        if (!all && !OTAPI_Wrap::NumList_VerifyQuery(indices, to_string(i))) {
            continue;
        }

        string payment =
            OTAPI_Wrap::GetNym_OutpaymentsContentsByIndex(mynym, i);
        if ("" == payment) {
            otOut << "Error: cannot load payment " << i << ".\n";
            retVal = -1;
            continue;
        }

        string server = OTAPI_Wrap::GetNym_OutpaymentsNotaryIDByIndex(mynym, i);
        if ("" == server) {
            otOut << "Error: cannot load server for payment " << i << ".\n";
            retVal = -1;
            continue;
        }

        // Done: Put the code here where we message the server to cancel all
        // relevant transaction numbers for the instrument. If it's a cheque,
        // there's only one number. But if it's a smart contract, there could
        // be many numbers. Seems like best thing is to just activate it, but
        // have a "rejected" flag which causes the activation to fail. (That
        // way, all the other parties will be properly notified, which the
        // server already does.) We don't even need to remove it from the
        // outpayment box, because the failure notification from the server
        // will already cause the OTClient to remove it from the outpayment box.
        //
        // Ah-ha! ANY outgoing payment plan or smart contract is necessarily
        // incomplete: it's outgoing because it was sent to the next party so
        // he could sign it, too, and probably activate it. Presumably he has
        // not done so yet (if I am 'beating him to the punch' by cancelling it
        // before he can activate it) and therefore the plan or smart contract
        // still is missing at least one signer, so it is GUARANTEED to fail
        // verification if I try to activate it myself. (Good.)
        //
        // This means I can just take whatever instrument appears outgoing,
        // and try to activate it myself. It will definitely fail activation,
        // and then the failure notice will already be sent from that, to all
        // the parties, and they can harvest back their numbers automatically
        // as necessary.
        //
        // The one problem is, though this works for payment plans and smart
        // contracts, it will not work for cheques. The cheque is made out to
        // someone else, and he is the one normally who needs to deposit it.
        // Plus, I can't deposit a cheque into the same account it's drawn on.
        //
        // UPDATE: Now when a cheque is deposited into the same account it was
        // drawn on, that will be interpreted by the server as a request to
        // CANCEL the cheque.

        string type = OTAPI_Wrap::Instrmnt_GetType(payment);

        if ("SMARTCONTRACT" == type) {
            // Just take the smart contract from the outpayment box, and try to
            // activate it. It WILL fail, and then the failure message will be
            // propagated to all the other parties to the contract. (Which will
            // result in its automatic removal from the outpayment box.)

            // FIX: take myacct from smart contract instead of --myacct
            if ("" == myacct) {
                otOut << "You MUST provide --myacct for smart contracts.\n";
                retVal = -1;
                continue;
            }

            // Try to activate the smart contract. (As a way of  cancelling it.)
            // So while we expect this 'activation' to fail, it should have the
            // desired effect of cancelling the smart contract and sending
            // failure notices to all the parties.
            string response = MadeEasy::activate_smart_contract(
                server, mynym, myacct, "acct_agent_name", payment);
            if ("" == response) {
                otOut << "Error: cannot cancel smart contract.\n";
                retVal = -1;
                continue;
            }

            otOut << "Server reply: \n" << response << "\n";

            if (1 != OTAPI_Wrap::Message_IsTransactionCanceled(
                         server, mynym, myacct, response)) {
                otOut << "Error: cancel smart contract failed.\n";
                retVal = -1;
                continue;
            }

            otOut << "Success canceling smart contract!\n";
            continue;
        }

        if ("PAYMENT PLAN" == type) {
            // Just take the payment plan from the outpayment box, and try to
            // activate it. It WILL fail, and then the failure message will be
            // propagated to the other party to the contract. (Which will result
            // in its automatic removal from the outpayment box.)

            string response =
                MadeEasy::cancel_payment_plan(server, mynym, payment);
            if ("" == response) {
                otOut << "Error: cannot cancel payment plan.\n";
                retVal = -1;
                continue;
            }

            otOut << "Server reply: \n" << response << "\n";

            if (1 != OTAPI_Wrap::Message_IsTransactionCanceled(
                         server, mynym, myacct, response)) {
                otOut << "Error: cancel payment plan failed.\n";
                retVal = -1;
                continue;
            }

            otOut << "Success canceling payment plant!\n";
            continue;
        }

        if ("PURSE" == type) {
            // This is a tricky one -- why would anyone EVER want to discard
            // outgoing cash? Normally your incentive would be to do the
            // opposite: Keep a copy of all outgoing cash until the copy
            // itself expires (when the cash expires.) This way it's always
            // recoverable in the event of a "worst case" situation.
            //
            // So what do we do in this case? Nevertheless, the user has
            // explicitly just instructed the client to DISCARD OUTGOING CASH.
            //
            // Perhaps we should just ask the user to CONFIRM that he wants to
            // erase the cash, and make SURE that he understands the
            // consequences of that choice.

            // removes payment instrument (from payments in or out box)
            if (!OTAPI_Wrap::RecordPayment(server, mynym, false, i, false)) {
                otOut << "Error: cannot cancel cash purse.\n";
                retVal = -1;
                continue;
            }

            otOut << "Success canceling cash purse!\n";
            continue;
        }

        // CHEQUE VOUCHER INVOICE

        bool isVoucher = ("VOUCHER" == type);

        // Get the nym and account IDs from the cheque itself.
        string acctID = isVoucher
                            ? OTAPI_Wrap::Instrmnt_GetRemitterAcctID(payment)
                            : OTAPI_Wrap::Instrmnt_GetSenderAcctID(payment);
        if ("" == acctID) {
            otOut << "Error: cannot retrieve asset account ID.\n";
            retVal = -1;
            continue;
        }

        string nymID = isVoucher
                           ? OTAPI_Wrap::Instrmnt_GetRemitterNymID(payment)
                           : OTAPI_Wrap::Instrmnt_GetSenderNymID(payment);
        if ("" == nymID) {
            otOut << "Error: cannot retrieve sender nym.\n";
            retVal = -1;
            continue;
        }

        if (nymID != mynym) {
            otOut << "Error: unexpected sender nym.\n";
            retVal = -1;
            continue;
        }

        CmdDeposit deposit;
        if (1 != deposit.depositCheque(server, acctID, nymID, payment)) {
            otOut << "Error: cannot cancel " << type << ".\n";
            retVal = -1;
            continue;
        }

        otOut << "Success canceling " << type << "!\n";
    }
    return retVal;
}
