/************************************************************
 *
 *  CmdPayInvoice.cpp
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

#include "CmdPayInvoice.hpp"

#include "CmdDeposit.hpp"
#include "../ot_made_easy_ot.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/OTLog.hpp>

using namespace opentxs;
using namespace std;

CmdPayInvoice::CmdPayInvoice()
{
    command = "payinvoice";
    args[0] = "--myacct <account>";
    args[1] = "[--index <index>]";
    category = catOtherUsers;
    help = "Pay an invoice.";
    usage = "If --index is omitted you must paste an invoice.";
}

CmdPayInvoice::~CmdPayInvoice()
{
}

int32_t CmdPayInvoice::runWithOptions()
{
    return run(getOption("myacct"), getOption("index"));
}

// Should I bother moving the invoice from the payments inbox to the record box?
//
// Update: Okay for now, I'm using an API call here (RecordPayment) which moves
// the invoice. HOWEVER, in the real term, we don't want to do it here. Rather,
// we want to do it inside OT while it's processesing the server reply for your
// cheque (invoice) deposit.
// For example what if there's a network problem and we don't process that reply
// here now? There'll still be a copy of the reply in the Nymbox and it will
// still get processed at a future time... and THAT's when we need to move the
// record, not here. (But this is what we'll do for now.)

// UPDATE:
// - In my Payments Inbox, there could be a cheque or invoice. Either way, when
// I deposit the cheque or pay the invoice, the chequeReceipt goes back to the
// signer's asset account's inbox.
// - When he accepts the chequeReceipt (during a processInbox) and WHEN HE GETS
// THE "SUCCESS" REPLY to that processInbox, is when the chequeReceipt should
// be moved from his inbox to his record box. It MUST be done then, inside OT,
// because the next time he downloads the inbox from the server, that
// chequeReceipt won't be in there anymore! It'll be too late to pass it on to
// the records.
// - Whereas I, being the recipient of his cheque, had it in my **payments
// inbox,** and thus upon receipt of a successful server-reply to my deposit
// transaction, need to move it from my payments inbox to my record box. (The
// record box will eventually be a callback so that client software can take
// over that functionality, which is outside the scope of OT. The actual CALL
// to store in the record box, however should occur inside OT.)
// - For now, I'm using the below API call, so it's available inside the
// scripts. This is "good enough" for now, just to get the payments inbox/outbox
// working for the scripts. But in the long term, I'll need to add the hooks
// directly into OT as described just above. (It'll be necessary in order to
// get the record box working.)
// - Since I'm only worried about Payments Inbox for now, and since I'll be
// calling the below function directly from inside the scripts, how will this
// work? Incoming cheque or invoice will be in the payments inbox, and will
// need to be moved to recordBox (below call) when the script receives a
// success reply to the depositing/paying of that cheque/invoice.
// - Whereas outoing cheque/invoice is in the Outpayments box, (fundamentally
// more similar to the outmail box than to the payments inbox.) If the
// cheque/invoice is cashed/paid by the endorsee, **I** will receive the
// chequeReceipt, in MY asset account inbox, and when I accept it during a
// processInbox transaction, the SUCCESS REPLY from the server for that
// processInbox is where I should actually process that chequeReceipt and,
// if it appears in the outpayments box, move it at that time to the record
// box. The problem is, I can NOT do this much inside the script. To do this
// part, I thus HAVE to go into OT itself as I just described.
// - Fuck!
// - Therefore I might as well comment this out, since this simply isn't going
// to work.

// - Updated plan:
// 1. DONE: Inside OT, when processing successful server reply to processInbox
// request, if a chequeReceipt was processed out successfully, and if that
// chequeReceipt is found inside the outpayments, then move it at that time to
// the record box.
// 2. DONE: Inside OT, when processing successful server reply to depositCheque
// request, if that cheque is found inside the Payments Inbox, move it to the
// record box.
// 3. As for cash: If I SENT cash, it will be in my outpayments box. But that's
// wrong. Because I can never see if the guy cashed it or not. Therefore it
// should go straight to the record box, when sent. AND it needs to be
// encrypted to MY key, not his -- so need to generate BOTH versions, when
// exporting the purse to him in the FIRST PLACE. Then my version goes straight
// into my record box and I can delete it at my leisure. (If he comes running
// the next day saying "I lost it!!" I can still recover it. But once he
// deposits it, then the cash will be no good and I might as well archive it
// or destroy it, or whatever I choose to do with my personal records.)
// If I RECEIVED cash, it will be in my payments inbox, and then when I deposit
// it, and when I process the SUCCESSFUL server REPLY to my depositCash request,
// it should be moved to my record Box.
// 4. How about vouchers? If I deposit a voucher, then the "original sender"
// should get some sort of notice. This means attaching his ID to the voucher
// --which should be optional-- and then dropping an "FYI" notice to him when
// it gets deposited. It can't be a normal chequeReceipt because that's used to
// verify the balance agreement against a balance change, whereas a "voucher
// receipt" wouldn't represent a balance change at all, since the balance was
// already changed when you originally bought the voucher.
// Instead it would probably be send to your Nymbox but it COULD NOT BE PROVEN
// that it was, since OT currently can't prove NOTICE!!
//
// All of the above needs to happen inside OT, since there are many places
// where it's the only appropriate place to take the necessary action.

int32_t CmdPayInvoice::run(string myacct, string index)
{
    if (!checkAccount("myacct", myacct)) {
        return -1;
    }

    if ("" != index && !checkValue("index", index)) {
        return -1;
    }

    return processPayment(myacct, "INVOICE", "",
                          "" == index ? -1 : stoi(index));
}

int32_t CmdPayInvoice::processPayment(const string& myacct,
                                      const string& paymentType,
                                      const string& inbox, const int32_t index)
{
    if ("" == myacct) {
        otOut << "Failure: myacct not a valid string.\n";
        return -1;
    }

    string server = getAccountServer(myacct);
    if ("" == server) {
        return -1;
    }

    string mynym = getAccountNym(myacct);
    if ("" == mynym) {
        return -1;
    }

    string instrument = "";
    if (-1 == index) {
        instrument = inputText("the instrument");
        if ("" == instrument) {
            return -1;
        }
    }
    else {
        instrument =
            MadeEasy::get_payment_instrument(server, mynym, index, inbox);
        if ("" == instrument) {
            otOut << "Error: cannot get payment instrument.\n";
            return -1;
        }
    }

    string type = OTAPI_Wrap::Instrmnt_GetType(instrument);
    if ("" == type) {
        otOut << "Error: cannot determine instrument type.\n";
        return -1;
    }

    string strIndexErrorMsg = "";
    if (-1 != index) {
        strIndexErrorMsg = "at index " + to_string(index) + " ";
    }

    // If there's a payment type,
    // and it's not "ANY", and it's the wrong type,
    // then skip this one.
    if ("" != paymentType && paymentType != "ANY" && paymentType != type) {
        if (("CHEQUE" == paymentType && "VOUCHER" == type) ||
            ("VOUCHER" == paymentType && "CHEQUE" == type)) {
            // in this case we allow it to drop through.
        }
        else {
            otOut << "Error: invalid instrument type.\n";
            return -1;
        }
    }

    // Note: I USED to check the ASSET TYPE ID here, but then I removed it,
    // since details_deposit_cheque() already verifies that (so I don't need
    // to do it twice.)

    // By this point, we know the invoice has the right instrument definition
    // for the
    // account we're trying to use (to pay it from.)
    //
    // But we need to make sure the invoice is made out to mynym (or to no
    // one.) Because if it IS endorsed to a Nym, and mynym is NOT that nym,
    // then the transaction will fail. So let's check, before we bother
    // sending it...
    string recipient = OTAPI_Wrap::Instrmnt_GetRecipientNymID(instrument);

    // Not all instruments have a specified recipient. But if they do, let's
    // make sure the Nym matches.
    if ("" != recipient && (recipient != mynym)) {
        otOut << "The instrument " << strIndexErrorMsg
              << "is endorsed to a specific recipient (" << recipient
              << ") and that doesn't match the account's owner Nym (" << mynym
              << "). (Skipping.)\nTry specifying a different "
                 "account, using --myacct ACCT_ID \n";
        return -1;
    }

    // At this point I know the invoice isn't made out to anyone, or if it is,
    // it's properly made out to the owner of the account which I'm trying to
    // use to pay the invoice from. So let's pay it!
    // P.S. recipient might be empty, but mynym is guaranteed to be good.

    string assetType =
        OTAPI_Wrap::Instrmnt_GetInstrumentDefinitionID(instrument);
    string accountAssetType = getAccountAssetType(myacct);

    if ("" != assetType && accountAssetType != assetType) {
        otOut << "The instrument at index " << index
              << " has a different instrument definition than the selected "
                 "account. "
                 "(Skipping.)\nTry specifying a different account, using "
                 "--myacct ACCT_ID \n";
        return -1;
    }

    time64_t from = OTAPI_Wrap::Instrmnt_GetValidFrom(instrument);
    time64_t until = OTAPI_Wrap::Instrmnt_GetValidTo(instrument);
    time64_t now = OTAPI_Wrap::GetTime();

    if (now < from) {
        otOut << "The instrument at index " << index
              << " is not yet within its valid date range. (Skipping.)\n";
        return -1;
    }

    if (until > OT_TIME_ZERO && now > until) {
        otOut << "The instrument at index " << index
              << " is expired. (Moving it to the record box.)\n";

        // Since this instrument is expired, remove it from the payments inbox,
        // and move to record box.
        if (0 <= index &&
            OTAPI_Wrap::RecordPayment(server, mynym, true, index, true)) {
            return 0;
        }

        return -1;
    }

    // TODO, IMPORTANT: After the below deposits are completed successfully, the
    // wallet will receive a "successful deposit" server reply. When that
    // happens, OT (internally) needs to go and see if the deposited item was a
    // payment in the payments inbox. If so, it should REMOVE it from that box
    // and move it to the record box.
    //
    // That's why you don't see me messing with the payments inbox even when
    // these are successful. They DO need to be removed from the payments inbox,
    // but just not here in the script. (Rather, internally by OT itself.)
    if ("CHEQUE" == type || "VOUCHER" == type || "INVOICE" == type) {
        CmdDeposit deposit;
        return deposit.depositCheque(server, myacct, mynym, instrument);
    }

    if ("PURSE" == type) {
        CmdDeposit deposit;
        int32_t success =
            deposit.depositPurse(server, myacct, mynym, instrument, "");

        // if index != -1, go ahead and call RecordPayment on the purse at that
        // index, to remove it from payments inbox and move it to the recordbox.
        if (index != -1 && 1 == success) {
            OTAPI_Wrap::RecordPayment(server, mynym, true, index, true);
        }

        return success;
    }

    otOut << "\nSkipping this instrument: Expected CHEQUE, VOUCHER, INVOICE, "
             "or (cash) PURSE.\n";

    return -1;
}
