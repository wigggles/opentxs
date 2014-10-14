/************************************************************
 *
 *  Helpers.cpp
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

#include "Helpers.hpp"

#include "../ext/OTPayment.hpp"
#include "../core/crypto/OTEnvelope.hpp"
#include "../core/OTLedger.hpp"
#include "../core/OTPseudonym.hpp"
#include "../core/OTMessage.hpp"
#include "../core/OTString.hpp"
#include "../core/OTLog.hpp"

namespace opentxs
{

// For payments inbox and possibly recordbox / expired box. (Starting to write
// it now...)
//
// Caller responsible to delete.

OTPayment* GetInstrument(const OTPseudonym& theNym, const int32_t& nIndex,
                         OTLedger& ledger) // Returns financial instrument
                                           // by index.
{
    if ((0 > nIndex) || (nIndex >= ledger.GetTransactionCount())) {
        otErr << __FUNCTION__
              << ": nIndex is out of bounds (it's in the negative.)\n";
        OT_FAIL;
    }

    //    if (nIndex >= GetTransactionCount())
    //    {
    //        otErr << "%s: out of bounds: %d\n", __FUNCTION__, nIndex);
    //        return nullptr; // out of bounds. I'm saving from an
    // OT_ASSERT_MSG()
    // happening here. (Maybe I shouldn't.)
    //                   // ^^^ That's right you shouldn't! That's the client
    // developer's problem, not yours.
    //    }

    OTTransaction* pTransaction = ledger.GetTransactionByIndex(nIndex);

    if (nullptr == pTransaction) {
        otErr << __FUNCTION__
              << ": good index but uncovered nullptr pointer: " << nIndex
              << "\n";
        return nullptr; // Weird.
    }

    const int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty, since
    // it has never actually
    // been signed (in fact the whole point32_t with abbreviated transactions in
    // a ledger is that they
    // take up very little room, and have no signature of their own, but exist
    // merely as XML tags on
    // their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and if
    // so, sign it in order to
    // force the UpdateContents() call, so the programmatic user of this API
    // will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        ledger.LoadBoxReceipt(static_cast<int64_t>(
            lTransactionNum)); // I don't check return val here because I still
                               // want it to send the abbreviated form, if this
                               // fails.
        pTransaction =
            ledger.GetTransaction(static_cast<int64_t>(lTransactionNum));

        if (nullptr == pTransaction) {
            otErr << __FUNCTION__
                  << ": good index but uncovered nullptr "
                     "pointer after trying to load full version of receipt "
                     "(from abbreviated) at index: " << nIndex << "\n";
            return nullptr; // Weird. Clearly I need the full box receipt, if
                            // I'm
                            // to get the instrument out of it.
        }
    }

    /*
    TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
    -- Iterate through the transactions in the payments inbox.
    -- (They should all be "instrumentNotice" transactions.)
    -- Each transaction contains (1) OTMessage in "in ref to" field, which in
    turn contains an encrypted
    OTPayment in the payload field, which contains the actual financial
    instrument.
    -- *** Therefore, this function, based purely on ledger index (as we
    iterate):
    1. extracts the OTMessage from the Transaction "in ref to" field (for the
    transaction at that index),
    2. then decrypts the payload on that message, producing an OTPayment object,
    3. ...which contains the actual instrument.
    */

    if ((OTTransaction::instrumentNotice != pTransaction->GetType()) &&
        (OTTransaction::payDividend != pTransaction->GetType()) &&
        (OTTransaction::notice != pTransaction->GetType())) {
        otOut << __FUNCTION__
              << ": Failure: Expected OTTransaction::instrumentNotice, "
                 "::payDividend or ::notice, "
                 "but found: OTTransaction::" << pTransaction->GetTypeString()
              << "\n";
        return nullptr;
    }

    if ((OTTransaction::instrumentNotice ==
         pTransaction->GetType()) || // It's encrypted.
        (OTTransaction::payDividend == pTransaction->GetType())) {
        OTString strMsg;
        pTransaction->GetReferenceString(strMsg);

        if (!strMsg.Exists()) {
            otOut << __FUNCTION__
                  << ": Failure: Expected OTTransaction::instrumentNotice to "
                     "contain an 'in reference to' string, but it was empty. "
                     "(Returning \"\".)\n";
            return nullptr;
        }

        std::unique_ptr<OTMessage> pMsg(new OTMessage);
        if (nullptr == pMsg) {
            otErr << __FUNCTION__ << ": Null:  Assert while allocating memory "
                                     "for an OTMessage!\n";
            OT_FAIL;
        }

        if (!pMsg->LoadContractFromString(strMsg)) {
            otOut << __FUNCTION__
                  << ": Failed trying to load OTMessage from string:\n\n"
                  << strMsg << "\n\n";
            return nullptr;
        }

        // By this point, the original OTMessage has been loaded from string
        // successfully.
        // Now we need to decrypt the payment on that message (which contains
        // the instrument
        // itself that we need to return.) We decrypt it the same way as we do
        // in
        // OTAPI_Wrap::GetNym_MailContentsByIndex():
        //

        // SENDER:     pMsg->m_strNymID
        // RECIPIENT:  pMsg->m_strNymID2
        // INSTRUMENT: pMsg->m_ascPayload (in an OTEnvelope)
        //
        OTEnvelope theEnvelope;
        OTString strEnvelopeContents;

        // Decrypt the Envelope.
        if (!theEnvelope.SetAsciiArmoredData(pMsg->m_ascPayload))
            otOut << __FUNCTION__
                  << ": Failed trying to set ASCII-armored data for envelope:\n"
                  << strMsg << "\n\n";
        else if (!theEnvelope.Open(theNym, strEnvelopeContents))
            otOut << __FUNCTION__
                  << ": Failed trying to decrypt the financial instrument "
                     "that was supposedly attached as a payload to this "
                     "payment message:\n" << strMsg << "\n\n";
        else if (!strEnvelopeContents.Exists())
            otOut << __FUNCTION__
                  << ": Failed: after decryption, cleartext is empty. From:\n"
                  << strMsg << "\n\n";
        else {
            std::unique_ptr<OTPayment> pPayment(
                new OTPayment(strEnvelopeContents)); // strEnvelopeContents
                                                     // contains a PURSE or
                                                     // CHEQUE (etc) and not
                                                     // specifically a PAYMENT.
            OT_ASSERT(nullptr != pPayment);

            if (!pPayment->IsValid())
                otOut << __FUNCTION__
                      << ": Failed: after decryption, payment is invalid. "
                         "Contents:\n\n" << strEnvelopeContents << "\n\n";
            else // success.
            {
                return pPayment.release(); // Caller responsible to delete.
            }
        }
    }
    else
        otErr << __FUNCTION__ << ": This must be a notice (vs an "
                                 "instrumentNotice or payDividend). "
                                 "!!! Not yet supported !!!\n";

    return nullptr;
}

int32_t GetOutpaymentsIndexByTransNum(const OTPseudonym& nym, int64_t lTransNum)
{
    int32_t lOutpaymentsCount = nym.GetOutpaymentsCount();

    for (int32_t lOutpaymentsIndex = 0; lOutpaymentsIndex < lOutpaymentsCount;
         ++lOutpaymentsIndex) {
        OTMessage* pOutpaymentMsg =
            nym.GetOutpaymentsByIndex(lOutpaymentsIndex);
        if (nullptr != pOutpaymentMsg) {
            OTString strPayment;

            // There isn't any encrypted envelope this time, since it's my
            // outPayments box.
            //
            if (pOutpaymentMsg->m_ascPayload.Exists() &&
                pOutpaymentMsg->m_ascPayload.GetString(strPayment) &&
                strPayment.Exists()) {
                OTPayment thePayment(strPayment);

                // Let's see if it's the cheque we're looking for...
                //
                if (thePayment.IsValid()) {
                    if (thePayment.SetTempValues()) {
                        if (thePayment.HasTransactionNum(lTransNum)) {
                            return static_cast<int32_t>(lOutpaymentsIndex);
                        }
                    }
                }
            }
        }
    }

    return -1;
}

} // namespace opentxs
