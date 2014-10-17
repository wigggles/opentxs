/************************************************************
 *
 *  OTMessage.cpp
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

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/OTMessage.hpp>

#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <fstream>

#include <irrxml/irrXML.hpp>

// PROTOCOL DOCUMENT

// --- This is the file that implements the entire message protocol.
// (Transactions are in a different file.)

// true  == success (even if nothing harvested.)
// false == error.
//

namespace opentxs
{

bool OTMessage::HarvestTransactionNumbers(
    OTPseudonym& theNym,
    bool bHarvestingForRetry,          // false until positively asserted.
    bool bReplyWasSuccess,             // false until positively asserted.
    bool bReplyWasFailure,             // false until positively asserted.
    bool bTransactionWasSuccess,       // false until positively asserted.
    bool bTransactionWasFailure) const // false until positively asserted.
{

    const OTIdentifier MSG_NYM_ID(m_strNymID), SERVER_ID(m_strServerID),
        ACCOUNT_ID(m_strAcctID.Exists() ? m_strAcctID
                                        : m_strNymID); // This may be
                                                       // unnecessary, but just
                                                       // in case.

    const OTString strLedger(m_ascPayload);

    OTLedger theLedger(MSG_NYM_ID, ACCOUNT_ID, SERVER_ID); // We're going to
                                                           // load a messsage
                                                           // ledger from *this.

    if (!strLedger.Exists() || !theLedger.LoadLedgerFromString(strLedger)) {
        otErr << __FUNCTION__
              << ": ERROR: Failed trying to load message ledger:\n\n"
              << strLedger << "\n\n";
        return false;
    }
    // theLedger is loaded up!
    else {
        // Let's iterate through the transactions inside, and harvest whatever
        // we can...
        //
        for (auto& it : theLedger.GetTransactionMap()) {
            OTTransaction* pTransaction = it.second;
            OT_ASSERT(nullptr != pTransaction);

            // NOTE: You would ONLY harvest the transaction numbers if your
            // request failed.
            // Clearly you would never bother harvesting the numbers from a
            // SUCCESSFUL request,
            // because doing so would only put you out of sync. (This is the
            // same reason why
            // we DO harvest numbers from UNSUCCESSFUL requests--in order to
            // stay in sync.)
            //
            // That having been said, an important distinction must be made
            // between failed
            // requests where "the message succeeded but the TRANSACTION
            // failed", versus requests
            // where the MESSAGE ITSELF failed (meaning the transaction itself
            // never got a
            // chance to run, and thus never had a chance to fail.)
            //
            // In the first case, you don't want to harvest the opening
            // transaction number
            // (the primary transaction number for that transaction) because
            // that number was
            // already burned when the transaction failed. Instead, you want to
            // harvest "all
            // the others" (the "closing" numbers.)
            // But in the second case, you want to harvest the opening
            // transaction number as well,
            // since it is still good (because the transaction never ran.)
            //
            // (Therefore the below logic turns on whether or not the message
            // was a success.)
            //
            // UPDATE: The logic is now all inside
            // OTTransaction::Harvest...Numbers, you just have to tell it,
            // when you call it, the state of certain things (message success,
            // transaction success, etc.)
            //

            pTransaction->HarvestOpeningNumber(
                theNym, bHarvestingForRetry, bReplyWasSuccess, bReplyWasFailure,
                bTransactionWasSuccess, bTransactionWasFailure);

            // We grab the closing numbers no matter what (whether message
            // succeeded or failed.)
            // It bears mentioning one more time that you would NEVER harvest in
            // the first place unless
            // your original request somehow failed. So this is more about WHERE
            // the failure occurred (at
            // the message level or the transaction level), not WHETHER one
            // occurred.
            //
            pTransaction->HarvestClosingNumbers(
                theNym, bHarvestingForRetry, bReplyWasSuccess, bReplyWasFailure,
                bTransactionWasSuccess, bTransactionWasFailure);
        }
    } // else (ledger is loaded up.)

    return true;
}

// So the message can get the list of numbers from the Nym, before sending,
// that should be listed as acknowledged that the server reply has already been
// seen for those request numbers.
// IMPORTANT NOTE: The Server ID is used to lookup the numbers from the Nym.
// Therefore,
// make sure that OTMessage::m_strServerID is set BEFORE calling this function.
// (It will
// ASSERT if you don't...)
//
void OTMessage::SetAcknowledgments(OTPseudonym& theNym)
{
    m_AcknowledgedReplies.Release();

    const OTIdentifier theServerID(m_strServerID);

    for (auto& it : theNym.GetMapAcknowledgedNum()) {
        std::string strServerID = it.first;
        dequeOfTransNums* pDeque = it.second;
        OT_ASSERT(nullptr != pDeque);

        OTString OTstrServerID = strServerID.c_str();
        const OTIdentifier theTempID(OTstrServerID);

        if (!(pDeque->empty()) &&
            (theServerID == theTempID)) // only for the matching serverID.
        {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                const int64_t lAckRequestNumber = pDeque->at(i);

                m_AcknowledgedReplies.Add(lAckRequestNumber);
            }
            break; // We found it! Might as well break out.
        }
    } // for
}

// The framework (OTContract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts
// are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called
// just prior to the signing of the message, in OTContract::SignContract.
void OTMessage::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_lTime = OTTimeGetSecondsFromTime(OTTimeGetCurrentTime());

    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");
    m_xmlUnsigned.Concatenate(
        "<OTmessage\n version=\"%s\"\n dateSigned=\"%lld\">\n\n",
        m_strVersion.Get(), m_lTime);

    if (!updateContentsByType()) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"false\"\n"
                                  " acctID=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  " ><!-- THIS IS AN INVALID MESSAGE -->\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strAcctID.Get(), m_strNymID.Get(),
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
    }

    // ACKNOWLEDGED REQUEST NUMBERS
    //
    // (For reducing the number of box receipts for replyNotices that must be
    // downloaded.)
    //
    // Client keeps a list of server replies he's already seen.
    // Server keeps a list of numbers the client has provided on HIS list
    // (server has removed those from Nymbox).
    //
    // (Each sends his respective list in every message.)
    //
    // Client removes any number he sees on the server's list.
    // Server removes any number he sees the client has also removed.
    //

    if (m_AcknowledgedReplies.Count() > 0) {
        OTString strAck;
        if (m_AcknowledgedReplies.Output(strAck) && strAck.Exists()) {
            const OTASCIIArmor ascTemp(strAck);

            if (ascTemp.Exists())
                m_xmlUnsigned.Concatenate("<ackReplies>\n%s</ackReplies>\n\n",
                                          ascTemp.Get());
        }
    }

    m_xmlUnsigned.Concatenate("</OTmessage>\n");
}

bool OTMessage::updateContentsByType()
{
    if (m_strCommand.Compare("getMarketList")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getMarketList")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " depth=\"%lld\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_lDepth);

        if (m_bSuccess && (m_ascPayload.GetLength() > 2) && (m_lDepth > 0))
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());
        else if (!m_bSuccess && (m_ascInReferenceTo.GetLength() > 2))
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getMarketOffers")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " marketID=\"%s\"\n" // stored in NymID2
                                  " depth=\"%lld\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strNymID2.Get(), // Storing Market ID
                                  m_lDepth);

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getMarketOffers")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " marketID=\"%s\"\n" // stored in NymID2
                                  " depth=\"%lld\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strNymID2.Get(), // Storing Market ID
                                  m_lDepth);

        if (m_bSuccess && (m_ascPayload.GetLength() > 2) && (m_lDepth > 0))
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());
        else if (!m_bSuccess && (m_ascInReferenceTo.GetLength() > 2))
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getMarketRecentTrades")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " marketID=\"%s\"" // stored in NymID2
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strNymID2.Get() // Storing Market ID
                                  );

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getMarketRecentTrades")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " marketID=\"%s\"\n" // stored in NymID2
                                  " depth=\"%lld\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strNymID2.Get(), // Storing Market ID
                                  m_lDepth);

        if (m_bSuccess && (m_ascPayload.GetLength() > 2) && (m_lDepth > 0))
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());
        else if (!m_bSuccess && (m_ascInReferenceTo.GetLength() > 2))
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getNym_MarketOffers")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getNym_MarketOffers")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " depth=\"%lld\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_lDepth);

        if (m_bSuccess && (m_ascPayload.GetLength() > 2) && (m_lDepth > 0))
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());
        else if (!m_bSuccess && (m_ascInReferenceTo.GetLength() > 2))
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("checkServerID")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get());

        m_xmlUnsigned.Concatenate(
            "<publicAuthentKey>\n%s</publicAuthentKey>\n\n",
            m_strNymPublicKey.Get());
        m_xmlUnsigned.Concatenate(
            "<publicEncryptionKey>\n%s</publicEncryptionKey>\n\n",
            m_strNymID2.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@checkServerID")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("createUserAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get());
        if (m_ascPayload.Exists())
            m_xmlUnsigned.Concatenate(
                "<credentialList>\n%s</credentialList>\n\n",
                m_ascPayload.Get());
        if (m_ascPayload2.Exists())
            m_xmlUnsigned.Concatenate("<credentials>\n%s</credentials>\n\n",
                                      m_ascPayload2.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@createUserAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        if (m_bSuccess && (m_ascPayload.GetLength() > 2))
            m_xmlUnsigned.Concatenate("<nymfile>\n%s</nymfile>\n\n",
                                      m_ascPayload.Get());

        if (m_ascInReferenceTo.GetLength() > 2)
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("deleteUserAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  m_strNymID.Get(), m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@deleteUserAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength() > 2)
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("checkUser")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymID2.Get(), m_strRequestNum.Get(),
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@checkUser")) {
        // This means new-style credentials are being sent, not just the public
        // key as before.
        const bool bCredentials =
            (m_ascPayload.Exists() && m_ascPayload2.Exists());

        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " hasCredentials=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymID2.Get(),
                                  (bCredentials ? "true" : "false"),
                                  m_strServerID.Get());

        if (m_bSuccess) {
            // Old style. (Deprecated.)
            if (m_strNymPublicKey.Exists())
                m_xmlUnsigned.Concatenate(
                    "<nymPublicKey>\n%s</nymPublicKey>\n\n",
                    m_strNymPublicKey.Get());

            // New style:
            if (bCredentials) {
                m_xmlUnsigned.Concatenate(
                    "<credentialList>\n%s</credentialList>\n\n",
                    m_ascPayload.Get());
                m_xmlUnsigned.Concatenate("<credentials>\n%s</credentials>\n\n",
                                          m_ascPayload2.Get());
            }
        }
        else
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());

        return true;
    }

    if (m_strCommand.Compare("usageCredits")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " adjustment=\"%lld\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymID2.Get(), m_strRequestNum.Get(),
                                  m_lDepth, m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@usageCredits")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " totalCredits=\"%lld\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymID2.Get(), m_lDepth,
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // This one isn't part of the message protocol, but is used for outmail
    // storage.
    // (Because outmail isn't encrypted like the inmail is, since the Nymfile
    // itself
    // will soon be encrypted, and there's no need to be redundant also as well
    // in addition on top of that.
    //
    if (m_strCommand.Compare("outmailMessage") ||
        m_strCommand.Compare("outpaymentsMessage")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymID2.Get(), m_strRequestNum.Get(),
                                  m_strServerID.Get());

        if (m_ascPayload.GetLength() > 2)
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("sendUserMessage")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymID2.Get(), m_strRequestNum.Get(),
                                  m_strServerID.Get());

        if (m_ascPayload.GetLength() > 2)
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@sendUserMessage")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymID2.Get(),
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare(
            "sendUserInstrument") || // sendUserInstrument is sent from one user
                                     // to the server, which then attaches that
                                     // message as a payment, onto a transaction
                                     // on the Nymbox of the recipient.
        m_strCommand.Compare("payDividend")) // payDividend is not a normal user
                                             // message. Rather, the sender uses
                                             // notarizeTransactions to do a
                                             // payDividend transaction. On the
                                             // server side, this creates a new
                                             // message of type "payDividend"
                                             // for each recipient, in order to
                                             // attach a voucher to it (for each
                                             // recipient) and then that
                                             // (artificially created
                                             // payDividend msg) is added to the
                                             // Nymbox of each recipient.
    {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymID2.Get(), m_strRequestNum.Get(),
                                  m_strServerID.Get());

        if (m_ascPayload.GetLength() > 2)
            m_xmlUnsigned.Concatenate(
                "<messagePayload>\n%s</messagePayload>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@sendUserInstrument")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymID2=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymID2.Get(),
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getRequest")) {
        m_xmlUnsigned.Concatenate("<%s\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // This is the ONE command where you see a request number coming back from
    // the server.
    // In all the other commands, it should be SENT to the server, not received
    // from the server.
    if (m_strCommand.Compare("@getRequest")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n"             // command
            " success=\"%s\"\n" // m_bSuccess
            " nymID=\"%s\"\n"
            " nymboxHash=\"%s\"\n"
            " serverID=\"%s\"\n"
            " newRequestNum=\"%lld\"\n"
            " requestNum=\"%s\""
            ">\n\n",
            m_strCommand.Get(), (m_bSuccess ? "true" : "false"),
            m_strNymID.Get(), m_strNymboxHash.Get(), m_strServerID.Get(),
            m_lNewRequestNum, m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("issueAssetType")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " assetType=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get(),
                                  m_strAssetID.Get());

        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<assetContract>\n%s</assetContract>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@issueAssetType")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n" // Command
            " requestNum=\"%s\"\n"
            " success=\"%s\"\n"
            " accountID=\"%s\"\n" // the new issuer account ID
            " nymID=\"%s\"\n"
            " assetType=\"%s\"\n"
            " serverID=\"%s\""
            ">\n\n",
            m_strCommand.Get(), m_strRequestNum.Get(),
            (m_bSuccess ? "true" : "false"), m_strAcctID.Get(),
            m_strNymID.Get(), m_strAssetID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<issuerAccount>\n%s</issuerAccount>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("queryAssetTypes")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get());

        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<stringMap>\n%s</stringMap>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@queryAssetTypes")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<stringMap>\n%s</stringMap>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("issueBasket")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get());

        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate(
                "<currencyBasket>\n%s</currencyBasket>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@issueBasket")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n" // Command
            " requestNum=\"%s\"\n"
            " success=\"%s\"\n"
            " accountID=\"%s\"\n" // the new basket issuer account ID
            " nymID=\"%s\"\n"
            " assetType=\"%s\"\n" // the new Asset Type
            " serverID=\"%s\""
            ">\n\n",
            m_strCommand.Get(), m_strRequestNum.Get(),
            (m_bSuccess ? "true" : "false"), m_strAcctID.Get(),
            m_strNymID.Get(), m_strAssetID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("createAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " assetType=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get(),
                                  m_strAssetID.Get());

        //        otErr << "DEBUG: Asset Type length: %d, Value:\n%s\n",
        // m_strAssetID.GetLength(),  m_strAssetID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@createAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strAcctID.Get(), m_strNymID.Get(),
                                  m_strServerID.Get());

        if (m_ascInReferenceTo.Exists())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        if (m_bSuccess && m_ascPayload.Exists())
            m_xmlUnsigned.Concatenate("<newAccount>\n%s</newAccount>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getBoxReceipt")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " serverID=\"%s\"\n"
            " requestNum=\"%s\"\n"
            " transactionNum=\"%lld\"\n"
            " boxType=\"%s\"\n"
            " accountID=\"%s\"" // If retrieving box receipt for Nymbox, NymID
                                // will appear in this variable.
            ">\n\n",
            m_strCommand.Get(), m_strNymID.Get(), m_strServerID.Get(),
            m_strRequestNum.Get(), m_lTransactionNum,
            (m_lDepth == 0)
                ? "nymbox"
                : ((m_lDepth == 1) ? "inbox" : "outbox"), // outbox is 2.
            m_strAcctID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getBoxReceipt")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n" // Command
            " requestNum=\"%s\"\n"
            " success=\"%s\"\n"
            " accountID=\"%s\"\n"
            " transactionNum=\"%lld\"\n"
            " boxType=\"%s\"\n"
            " nymID=\"%s\"\n"
            " serverID=\"%s\""
            ">\n\n",
            m_strCommand.Get(), m_strRequestNum.Get(),
            (m_bSuccess ? "true" : "false"), m_strAcctID.Get(),
            m_lTransactionNum,
            (m_lDepth == 0)
                ? "nymbox"
                : ((m_lDepth == 1) ? "inbox" : "outbox"), // outbox is 2.
            m_strNymID.Get(),
            m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<boxReceipt>\n%s</boxReceipt>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("deleteAssetAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\"\n"
                                  " accountID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get(),
                                  m_strAcctID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@deleteAssetAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  ">\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strAcctID.Get(), m_strNymID.Get(),
                                  m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("notarizeTransactions")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymboxHash.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get(), m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<accountLedger>\n%s</accountLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@notarizeTransactions")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate(
                "<responseLedger>\n%s</responseLedger>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getTransactionNum")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymboxHash.Get(), m_strServerID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("@getTransactionNum")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymboxHash.Get(),
                                  m_strServerID.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getNymbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@getNymbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strNymboxHash.Get(),
                                  m_strServerID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<nymboxLedger>\n%s</nymboxLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("getInbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@getInbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " inboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strInboxHash.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<inboxLedger>\n%s</inboxLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("getOutbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@getOutbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " outboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strOutboxHash.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<outboxLedger>\n%s</outboxLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTAccount object.
    if (m_strCommand.Compare("@getAccount")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<assetAccount>\n%s</assetAccount>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getAccountFiles")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAcctID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains a STRING_MAP containing the OTAccount,
    // plus the inbox and outbox for that acct..
    //
    if (m_strCommand.Compare("@getAccountFiles")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " inboxHash=\"%s\"\n"
                                  " outboxHash=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strInboxHash.Get(), m_strOutboxHash.Get(),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // Famous last words.
        //
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<acctFiles>\n%s</acctFiles>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getContract")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " assetType=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAssetID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTAssetContract object.
    if (m_strCommand.Compare("@getContract")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " assetType=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAssetID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<assetContract>\n%s</assetContract>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("getMint")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " assetType=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strServerID.Get(), m_strAssetID.Get(),
                                  m_strRequestNum.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTMint object.
    if (m_strCommand.Compare("@getMint")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " assetType=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAssetID.Get());

        if (!m_bSuccess && m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_bSuccess && m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<mint>\n%s</mint>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("processInbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymboxHash.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get(), m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<processLedger>\n%s</processLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@processInbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " accountID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get(),
                                  m_strAcctID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate(
                "<responseLedger>\n%s</responseLedger>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("processNymbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " nymID=\"%s\"\n"
                                  " nymboxHash=\"%s\"\n"
                                  " serverID=\"%s\"\n"
                                  " requestNum=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strNymID.Get(),
                                  m_strNymboxHash.Get(), m_strServerID.Get(),
                                  m_strRequestNum.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate("<processLedger>\n%s</processLedger>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTLedger object.
    if (m_strCommand.Compare("@processNymbox")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        // I would check if this was empty, but it should never be empty...
        // famous last words.
        if (m_ascPayload.GetLength())
            m_xmlUnsigned.Concatenate(
                "<responseLedger>\n%s</responseLedger>\n\n",
                m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    if (m_strCommand.Compare("triggerClause")) {
        m_xmlUnsigned.Concatenate(
            "<%s\n" // Command
            " nymID=\"%s\"\n"
            " nymboxHash=\"%s\"\n"
            " serverID=\"%s\"\n"
            " smartContractID=\"%lld\"\n" // <===
            " clauseName=\"%s\"\n"        // <===
            " hasParam=\"%s\"\n"          // <===
            " requestNum=\"%s\""
            " >\n\n",
            m_strCommand.Get(), m_strNymID.Get(), m_strNymboxHash.Get(),
            m_strServerID.Get(), m_lTransactionNum,
            m_strNymID2.Get(), // clause name is stored here for this message.
            (m_ascPayload.Exists()) ? "true" : "false", m_strRequestNum.Get());

        if (m_ascPayload.Exists())
            m_xmlUnsigned.Concatenate("<parameter>\n%s</parameter>\n\n",
                                      m_ascPayload.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }

    // the Payload contains an ascii-armored OTMint object.
    if (m_strCommand.Compare("@triggerClause")) {
        m_xmlUnsigned.Concatenate("<%s\n" // Command
                                  " requestNum=\"%s\"\n"
                                  " success=\"%s\"\n"
                                  " nymID=\"%s\"\n"
                                  " serverID=\"%s\""
                                  " >\n\n",
                                  m_strCommand.Get(), m_strRequestNum.Get(),
                                  (m_bSuccess ? "true" : "false"),
                                  m_strNymID.Get(), m_strServerID.Get());

        if (m_ascInReferenceTo.GetLength())
            m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                      m_ascInReferenceTo.Get());

        m_xmlUnsigned.Concatenate("</%s>\n\n", m_strCommand.Get());
        return true;
    }
    return false;
}

void OTMessage::processXmlSuccess(irr::io::IrrXMLReader*& xml)
{
    m_bSuccess = OTString(xml->getAttributeValue("success")).Compare("true");
}

// Todo: consider leaving the request # inside all the server REPLIES, so they
// are easier to match up to the requests. (Duh.)

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTMessage::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    //
    // if (nReturnVal = OTContract::ProcessXMLNode(xml))
    //      return nReturnVal;

    const OTString strNodeName(xml->getNodeName());

    if (strNodeName.Compare("ackReplies")) {
        return processXmlNodeAckReplies(xml);
    }
    else if (strNodeName.Compare("acknowledgedReplies")) {
        return processXmlNodeAcknowledgedReplies(xml);
    }
    else if (strNodeName.Compare("OTmessage")) {
        return processXmlNodeOTmessage(xml);
    }
    else if (strNodeName.Compare("getMarketList")) {
        return processXmlNodeGetMarketList(xml);
    }
    else if (strNodeName.Compare("@getMarketList")) {
        return processXmlNodeAtGetMarketList(xml);
    }
    else if (strNodeName.Compare("getMarketOffers")) {
        return processXmlNodeGetMarketOffers(xml);
    }
    else if (strNodeName.Compare("@getMarketOffers")) {
        return processXmlNodeAtGetMarketOffers(xml);
    }
    else if (strNodeName.Compare("getMarketRecentTrades")) {
        return processXmlNodeGetMarketRecentTrades(xml);
    }
    else if (strNodeName.Compare("@getMarketRecentTrades")) {
        return processXmlNodeAtGetMarketRecentTrades(xml);
    }
    else if (strNodeName.Compare("getNym_MarketOffers")) {
        return processXmlNodeGetNymMarketOffers(xml);
    }
    else if (strNodeName.Compare("@getNym_MarketOffers")) {
        return processXmlNodeAtGetNymMarketOffers(xml);
    }
    else if (strNodeName.Compare("checkServerID")) {
        return processXmlNodeCheckServerID(xml);
    }
    else if (strNodeName.Compare("@checkServerID")) {
        return processXmlNodeAtCheckServerID(xml);
    }
    else if (strNodeName.Compare("createUserAccount")) {
        return processXmlNodeCreateUserAccount(xml);
    }
    else if (strNodeName.Compare("@createUserAccount")) {
        return processXmlNodeAtcreateUserAccount(xml);
    }
    else if (strNodeName.Compare("deleteUserAccount")) {
        return processXmlNodeDeleteUserAccount(xml);
    }
    else if (strNodeName.Compare("@deleteUserAccount")) {
        return processXmlNodeAtDeleteUserAccount(xml);
    }
    else if (strNodeName.Compare("getRequest")) {
        return processXmlNodeGetRequest(xml);
    }
    else if (strNodeName.Compare("@getRequest")) {
        return processXmlNodeAtGetRequest(xml);
    }
    else if (strNodeName.Compare("outmailMessage") ||
               strNodeName.Compare("outpaymentsMessage")) {
        return processXmlNodeOutmailMessageOrOutpaymentsMessage(xml);
    }
    else if (strNodeName.Compare("sendUserMessage")) {
        return processXmlNodeSendUserMessage(xml);
    }
    else if (strNodeName.Compare("@sendUserMessage")) {
        return processXmlNodeAtSendUserMessage(xml);
    }
    else if (strNodeName.Compare("sendUserInstrument") ||
               strNodeName.Compare("payDividend")) // not a real message. Used
                                                   // by server when sending
                                                   // vouchers to people.
    {
        return processXmlNodeSendUserInstrumentOrPayDivident(xml);
    }
    else if (strNodeName.Compare("@sendUserInstrument")) {
        return processXmlNodeAtSendUserInstrument(xml);
    }
    else if (strNodeName.Compare("usageCredits")) {
        return processXmlNodeUsageCredits(xml);
    }
    else if (strNodeName.Compare("@usageCredits")) {
        return processXmlNodeAtUsageCredits(xml);
    }
    else if (strNodeName.Compare("checkUser")) {
        return processXmlNodeCheckUser(xml);
    }
    else if (strNodeName.Compare("@checkUser")) {
        return processXmlNodeAtCheckUser(xml);
    }
    else if (strNodeName.Compare("issueAssetType")) {
        return processXmlNodeIssueAssetType(xml);
    }
    else if (strNodeName.Compare("@issueAssetType")) {
        return processXmlNodeAtIssueAssetType(xml);
    }
    else if (strNodeName.Compare("queryAssetTypes")) {
        return processXmlNodeQueryAssetTypes(xml);
    }
    else if (strNodeName.Compare("@queryAssetTypes")) {
        return processXmlNodeAtQueryAssetTypes(xml);
    }
    else if (strNodeName.Compare("createAccount")) {
        return processXmlNodeCreateAccount(xml);
    }
    else if (strNodeName.Compare("@createAccount")) {
        return processXmlNodeAtCreateAccount(xml);
    }
    else if (strNodeName.Compare("getBoxReceipt")) {
        return processXmlNodeGetBoxReceipt(xml);
    }
    else if (strNodeName.Compare("@getBoxReceipt")) {
        return processXmlNodeAtGetBoxReceipt(xml);
    }
    else if (strNodeName.Compare("deleteAssetAccount")) {
        return processXmlNodeDeleteAssetAccount(xml);
    }
    else if (strNodeName.Compare("@deleteAssetAccount")) {
        return processXmlNodeAtDeleteAssetAccount(xml);
    }
    else if (strNodeName.Compare("issueBasket")) {
        return processXmlNodeIssueBasket(xml);
    }
    else if (strNodeName.Compare("@issueBasket")) {
        return processXmlNodeAtIssueBasket(xml);
    }
    else if (strNodeName.Compare("getTransactionNum")) {
        return processXmlNodeGetTransactionNum(xml);
    }
    else if (strNodeName.Compare("@getTransactionNum")) {
        return processXmlNodeAtGetTransactionNum(xml);
    }
    else if (strNodeName.Compare("notarizeTransactions")) {
        return processXmlNodeNotarizeTransactions(xml);
    }
    else if (strNodeName.Compare("@notarizeTransactions")) {
        return processXmlNodeAtNotarizeTransactions(xml);
    }
    else if (strNodeName.Compare("getInbox")) {
        return processXmlNodeGetInbox(xml);
    }
    else if (strNodeName.Compare("getNymbox")) {
        return processXmlNodeGetNymbox(xml);
    }
    else if (strNodeName.Compare("@getInbox")) {
        return processXmlNodeAtGetInbox(xml);
    }
    else if (strNodeName.Compare("@getNymbox")) {
        return processXmlNodeAtGetNymbox(xml);
    }
    else if (strNodeName.Compare("getOutbox")) {
        return processXmlNodeGetOutbox(xml);
    }
    else if (strNodeName.Compare("@getOutbox")) {
        return processXmlNodeAtGetOutbox(xml);
    }
    else if (strNodeName.Compare("getAccount")) {
        return processXmlNodeGetAccount(xml);
    }
    else if (strNodeName.Compare("@getAccount")) {
        return processXmlNodeAtGetAccount(xml);
    }
    else if (strNodeName.Compare("getAccountFiles")) {
        return processXmlNodeGetAccountFiles(xml);
    }
    else if (strNodeName.Compare("@getAccountFiles")) {
        return processXmlNodeAtGetAccountFiles(xml);
    }
    else if (strNodeName.Compare("getContract")) {
        return processXmlNodeGetContract(xml);
    }
    else if (strNodeName.Compare("@getContract")) {
        return processXmlNodeAtGetContract(xml);
    }
    else if (strNodeName.Compare("getMint")) {
        return processXmlNodeGetMint(xml);
    }
    // the Payload contains an ascii-armored OTMint object.
    else if (strNodeName.Compare("@getMint")) {
        return processXmlNodeAtGetMint(xml);
    }
    else if (strNodeName.Compare("triggerClause")) {
        return processXmlNodeTriggerClause(xml);
    }
    else if (strNodeName.Compare("@triggerClause")) {
        return processXmlNodeAtTriggerClause(xml);
    }
    else if (strNodeName.Compare("processInbox")) {
        return processXmlNodeProcessInbox(xml);
    }
    else if (strNodeName.Compare("processNymbox")) {
        return processXmlNodeProcessNymbox(xml);
    }
    else if (strNodeName.Compare("@processInbox")) {
        return processXmlNodeAtProcessInbox(xml);
    }
    else if (strNodeName.Compare("@processNymbox")) {
        return processXmlNodeAtProcessNymbox(xml);
    }

    return 0;
}

int32_t OTMessage::processXmlNodeAckReplies(irr::io::IrrXMLReader*& xml)
{
    OTString strDepth;
    if (!OTContract::LoadEncodedTextField(xml, strDepth)) {
        otErr << "Error in OTMessage::ProcessXMLNode: ackReplies field "
                 "without value.\n";
        return (-1); // error condition
    }

    m_AcknowledgedReplies.Release();

    if (strDepth.Exists()) m_AcknowledgedReplies.Add(strDepth);

    return 1;
}

int32_t OTMessage::processXmlNodeAcknowledgedReplies(
    irr::io::IrrXMLReader*& xml)
{
    otErr << "OTMessage::ProcessXMLNode: SKIPPING DEPRECATED FIELD: "
             "acknowledgedReplies\n";

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) {
        xml->read();
    }

    return 1;
}

int32_t OTMessage::processXmlNodeOTmessage(irr::io::IrrXMLReader*& xml)
{
    m_strVersion = xml->getAttributeValue("version");

    OTString strDateSigned = xml->getAttributeValue("dateSigned");

    if (strDateSigned.Exists()) m_lTime = atol(strDateSigned.Get());

    otInfo << "\n===> Loading XML for Message into memory structures...\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetMarketList(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetMarketList(irr::io::IrrXMLReader*& xml)
{
    //      std::cerr << m_xmlUnsigned << std::endl;
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    OTString strDepth = xml->getAttributeValue("depth");

    if (strDepth.GetLength() > 0) m_lDepth = atol(strDepth.Get());

    const char* pElementExpected = nullptr;
    if (m_bSuccess && (m_lDepth > 0))
        pElementExpected = "messagePayload";
    else if (!m_bSuccess)
        pElementExpected = "inReferenceTo";

    if (nullptr != pElementExpected) {
        OTASCIIArmor ascTextExpected;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }

        if (m_bSuccess)
            m_ascPayload.Set(ascTextExpected);
        else
            m_ascInReferenceTo.Set(ascTextExpected);
    }

    if (m_bSuccess)
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n\n"; // m_ascPayload.Get()
    else
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n\n"; // m_ascInReferenceTo.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeGetMarketOffers(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID2 = xml->getAttributeValue("marketID");

    OTString strDepth = xml->getAttributeValue("depth");

    if (strDepth.GetLength() > 0) m_lDepth = atol(strDepth.Get());

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n Market ID: " << m_strNymID2
           << "\n Request #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetMarketOffers(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strNymID2 = xml->getAttributeValue("marketID");

    OTString strDepth = xml->getAttributeValue("depth");

    if (strDepth.GetLength() > 0) m_lDepth = atol(strDepth.Get());

    const char* pElementExpected = nullptr;
    if (m_bSuccess && (m_lDepth > 0))
        pElementExpected = "messagePayload";
    else if (!m_bSuccess)
        pElementExpected = "inReferenceTo";

    if (nullptr != pElementExpected) {
        OTASCIIArmor ascTextExpected;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }

        if (m_bSuccess)
            m_ascPayload.Set(ascTextExpected);
        else
            m_ascInReferenceTo = ascTextExpected;
    }

    if (m_bSuccess)
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n MarketID: " << m_strNymID2
               << "\n\n"; // m_ascPayload.Get()
    else
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n MarketID: " << m_strNymID2
               << "\n\n"; // m_ascInReferenceTo.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeGetMarketRecentTrades(
    irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID2 = xml->getAttributeValue("marketID");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n Market ID: " << m_strNymID2
           << "\n Request #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetMarketRecentTrades(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strNymID2 = xml->getAttributeValue("marketID");

    OTString strDepth = xml->getAttributeValue("depth");

    if (strDepth.GetLength() > 0) m_lDepth = atol(strDepth.Get());

    const char* pElementExpected = nullptr;
    if (m_bSuccess && (m_lDepth > 0))
        pElementExpected = "messagePayload";
    else if (!m_bSuccess)
        pElementExpected = "inReferenceTo";

    if (nullptr != pElementExpected) {
        OTASCIIArmor ascTextExpected;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }

        if (m_bSuccess)
            m_ascPayload.Set(ascTextExpected);
        else
            m_ascInReferenceTo = ascTextExpected;
    }

    if (m_bSuccess)
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n MarketID: " << m_strNymID2
               << "\n\n"; // m_ascPayload.Get()
    else
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n MarketID: " << m_strNymID2
               << "\n\n"; // m_ascInReferenceTo.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeGetNymMarketOffers(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetNymMarketOffers(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    OTString strDepth = xml->getAttributeValue("depth");

    if (strDepth.GetLength() > 0) m_lDepth = atol(strDepth.Get());

    const char* pElementExpected = nullptr;
    if (m_bSuccess && (m_lDepth > 0))
        pElementExpected = "messagePayload";
    else if (!m_bSuccess)
        pElementExpected = "inReferenceTo";

    if (nullptr != pElementExpected) {
        OTASCIIArmor ascTextExpected;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }

        if (m_bSuccess)
            m_ascPayload.Set(ascTextExpected);
        else
            m_ascInReferenceTo = ascTextExpected;
    }

    if (m_bSuccess)
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n\n"; // m_ascPayload.Get()
    else
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID
               << "\n ServerID: " << m_strServerID
               << "\n\n"; // m_ascInReferenceTo.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeCheckServerID(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    const char* pElementExpected = "publicAuthentKey";
    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    m_strNymPublicKey.Set(ascTextExpected);

    pElementExpected = "publicEncryptionKey";
    ascTextExpected.Release();

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    m_strNymID2.Set(ascTextExpected);

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n\n Public signing key:\n"
           << m_strNymPublicKey << "\nPublic encryption key:\n" << m_strNymID2
           << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtCheckServerID(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    otWarn << "\nCommand: " << m_strCommand
           << "\nSuccess: " << (m_bSuccess ? "true" : "false")
           << "\nNymID:    " << m_strNymID << "\n"
                                              "ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeCreateUserAccount(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    const char* pElementExpected = "credentialList";

    if (!OTContract::LoadEncodedTextFieldByName(xml, m_ascPayload,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    pElementExpected = "credentials";

    if (!OTContract::LoadEncodedTextFieldByName(xml, m_ascPayload2,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtcreateUserAccount(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    if (m_bSuccess) {
        const char* pElementExpected = "nymfile";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    const char* pElementExpected = "inReferenceTo";
    OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "  "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nServerID: " << m_strServerID
           << "\n\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeDeleteUserAccount(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtDeleteUserAccount(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    const char* pElementExpected = "inReferenceTo";
    OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "  "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nServerID: " << m_strServerID
           << "\n\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetRequest(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetRequest(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");

    const OTString strNewRequestNum = xml->getAttributeValue("newRequestNum");
    m_lNewRequestNum =
        strNewRequestNum.Exists() ? atol(strNewRequestNum.Get()) : 0;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\n"
                                              "ServerID: " << m_strServerID
           << "\nRequest Number:    " << m_strRequestNum
           << "  New Number: " << m_lNewRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeOutmailMessageOrOutpaymentsMessage(
    irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    const char* pElementExpected = "messagePayload";
    OTASCIIArmor& ascTextExpected = m_ascPayload;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nNymID2:    " << m_strNymID2 << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeSendUserMessage(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    const char* pElementExpected = "messagePayload";
    OTASCIIArmor& ascTextExpected = m_ascPayload;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nNymID2:    " << m_strNymID2 << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtSendUserMessage(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nNymID2:    " << m_strNymID2
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeSendUserInstrumentOrPayDivident(
    irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    const char* pElementExpected = "messagePayload";
    OTASCIIArmor& ascTextExpected = m_ascPayload;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nNymID2:    " << m_strNymID2 << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtSendUserInstrument(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nNymID2:    " << m_strNymID2
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeUsageCredits(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    OTString strAdjustment = xml->getAttributeValue("adjustment");

    if (strAdjustment.GetLength() > 0) m_lDepth = atol(strAdjustment.Get());

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nNymID2:    " << m_strNymID2 << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\nAdjustment: " << m_lDepth
           << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtUsageCredits(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");

    OTString strTotalCredits = xml->getAttributeValue("totalCredits");

    if (strTotalCredits.GetLength() > 0) m_lDepth = atol(strTotalCredits.Get());

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nNymID2:    " << m_strNymID2
           << "\n"
              "ServerID: " << m_strServerID << "\nTotal Credits: " << m_lDepth
           << " \n\n";
    return 1;
}

int32_t OTMessage::processXmlNodeCheckUser(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nNymID2:    " << m_strNymID2 << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtCheckUser(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymID2 = xml->getAttributeValue("nymID2");
    m_strServerID = xml->getAttributeValue("serverID");

    const OTString strHasCredentials(xml->getAttributeValue("hasCredentials"));
    const bool bHasCredentials = strHasCredentials.Compare("true");

    const char* pElementExpected = nullptr;
    if (m_bSuccess)
        pElementExpected = "nymPublicKey";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;
    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }
    if (m_bSuccess)
        m_strNymPublicKey.Set(ascTextExpected);
    else
        m_ascInReferenceTo = ascTextExpected;

    if (bHasCredentials) {
        pElementExpected = "credentialList";
        ascTextExpected.Release();

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
        m_ascPayload = ascTextExpected;

        pElementExpected = "credentials";
        ascTextExpected.Release();

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
        m_ascPayload2 = ascTextExpected;
    }

    if (m_bSuccess)
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID << "\nNymID2:    " << m_strNymID2
               << "\n"
                  "ServerID: " << m_strServerID << "\nNym2 Public Key:\n"
               << m_strNymPublicKey << "\n\n";
    else
        otWarn << "\nCommand: " << m_strCommand << "   "
               << (m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m_strNymID << "\nNymID2:    " << m_strNymID2
               << "\n"
                  "ServerID: " << m_strServerID
               << "\n\n"; // m_ascInReferenceTo.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeIssueAssetType(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    const char* pElementExpected = "assetContract";
    OTASCIIArmor& ascTextExpected = m_ascPayload;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << " \nNymID:    " << m_strNymID
           << "\n"
              "ServerID: " << m_strServerID << "\nRequest#: " << m_strRequestNum
           << "\nAsset Type:\n" << m_strAssetID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtIssueAssetType(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strAcctID = xml->getAttributeValue("accountID");

    // If successful, we need to read 2 more things: inReferenceTo and
    // issuerAccount payload.
    // If failure, then we only need to read 1 thing: inReferenceTo
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    if (m_bSuccess) {
        const char* pElementExpected = "issuerAccount";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there,
    // OR if it was successful but the Payload isn't there, then failure.
    if (!m_ascInReferenceTo.GetLength() ||
        (m_bSuccess && !m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected issuerAccount and/or inReferenceTo elements "
                 "with text fields in "
                 "@issueAssetType reply\n";
        return (-1); // error condition
    }

    OTString acctContents(m_ascPayload);
    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID: " << m_strAcctID
           << "\nAsset Type ID: " << m_strAssetID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",
    //    m_ascInReferenceTo.Get(),
    // acctContents.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeQueryAssetTypes(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    const char* pElementExpected = "stringMap";
    OTASCIIArmor& ascTextExpected = m_ascPayload;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << " \nNymID:    " << m_strNymID
           << "\n"
              "ServerID: " << m_strServerID << "\nRequest#: " << m_strRequestNum
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtQueryAssetTypes(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strServerID = xml->getAttributeValue("serverID");

    // If successful, we need to read 2 more things: inReferenceTo and
    // issuerAccount payload.
    // If failure, then we only need to read 1 thing: inReferenceTo
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    if (m_bSuccess) {
        const char* pElementExpected = "stringMap";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there,
    // OR if it was successful but the Payload isn't there, then failure.
    if (!m_ascInReferenceTo.GetLength() ||
        (m_bSuccess && !m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected stringMap and/or inReferenceTo elements with "
                 "text fields in "
                 "@queryAssetTypes reply\n";
        return (-1); // error condition
    }

    otWarn << "\n Command: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\n NymID:    " << m_strNymID << "\n ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeCreateAccount(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << " \nNymID:    " << m_strNymID
           << "\n"
              "ServerID: " << m_strServerID << "\nRequest#: " << m_strRequestNum
           << "\nAsset Type:\n" << m_strAssetID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtCreateAccount(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    // If successful, we need to read 2 more things: inReferenceTo and
    // issuerAccount payload.
    // If failure, then we only need to read 1 thing: inReferenceTo
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            //                return (-1); // error condition
        }
    }

    if (m_bSuccess) {
        const char* pElementExpected = "newAccount";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there,
    // OR if it was successful but the Payload isn't there, then failure.
    //
    if (m_bSuccess && !m_ascPayload.GetLength()) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected newAccount element with text field, in "
                 "@createAccount reply\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID: " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",
    //    m_ascInReferenceTo.Get(),
    // acctContents.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeGetBoxReceipt(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    OTString strTransactionNum = xml->getAttributeValue("transactionNum");
    m_lTransactionNum =
        strTransactionNum.Exists() ? atol(strTransactionNum.Get()) : 0;

    const OTString strBoxType = xml->getAttributeValue("boxType");

    if (strBoxType.Compare("nymbox"))
        m_lDepth = 0;
    else if (strBoxType.Compare("inbox"))
        m_lDepth = 1;
    else if (strBoxType.Compare("outbox"))
        m_lDepth = 2;
    else {
        m_lDepth = 0;
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected boxType to be inbox, outbox, or nymbox, in "
                 "getBoxReceipt\n";
        return (-1);
    }

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n AccountID:    " << m_strAcctID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum
           << "  Transaction#: " << m_lTransactionNum << "   boxType: "
           << ((m_lDepth == 0) ? "nymbox" : (m_lDepth == 1) ? "inbox"
                                                            : "outbox")
           << "\n\n"; // outbox is 2.);

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetBoxReceipt(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    OTString strTransactionNum = xml->getAttributeValue("transactionNum");
    m_lTransactionNum =
        strTransactionNum.Exists() ? atol(strTransactionNum.Get()) : 0;

    const OTString strBoxType = xml->getAttributeValue("boxType");

    if (strBoxType.Compare("nymbox"))
        m_lDepth = 0;
    else if (strBoxType.Compare("inbox"))
        m_lDepth = 1;
    else if (strBoxType.Compare("outbox"))
        m_lDepth = 2;
    else {
        m_lDepth = 0;
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected boxType to be inbox, outbox, or nymbox, in "
                 "@getBoxReceipt reply\n";
        return (-1);
    }

    // inReferenceTo contains the getBoxReceipt (original request)
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    if (m_bSuccess) {
        const char* pElementExpected = "boxReceipt";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there,
    // OR if it was successful but the Payload isn't there, then failure.
    if (!m_ascInReferenceTo.GetLength() ||
        (m_bSuccess && !m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected boxReceipt and/or inReferenceTo elements with "
                 "text fields in "
                 "@getBoxReceipt reply\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID: " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",

    return 1;
}

int32_t OTMessage::processXmlNodeDeleteAssetAccount(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n AccountID:    " << m_strAcctID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtDeleteAssetAccount(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    // inReferenceTo contains the deleteAssetAccount (original request)
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there, then failure.
    if (!m_ascInReferenceTo.GetLength()) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected inReferenceTo element with text fields in "
                 "@deleteAssetAccount reply\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID: " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",
    //    m_ascInReferenceTo.Get(),
    // acctContents.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeIssueBasket(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    {
        const char* pElementExpected = "currencyBasket";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the Payload isn't there, then failure.
    if (!m_ascPayload.GetLength()) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected currencyBasket element with text fields in "
                 "issueBasket message\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << " \nNymID:    " << m_strNymID
           << "\n"
              "ServerID: " << m_strServerID << "\nRequest#: " << m_strRequestNum
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtIssueBasket(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there,
    // OR if it was successful but the Payload isn't there, then failure.
    if (!m_ascInReferenceTo.GetLength()) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected inReferenceTo element with text fields in "
                 "@issueBasket reply\n";
        return (-1); // error condition
    }

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID: " << m_strAcctID
           << "\nAssetTypeID: " << m_strAssetID << "\n"
                                                   "ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetTransactionNum(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetTransactionNum(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");

    otWarn << "\n Command: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\n NymID:    " << m_strNymID << "\n"
                                               " ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeNotarizeTransactions(
    irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    {
        const char* pElementExpected = "accountLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n AccountID:    " << m_strAcctID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtNotarizeTransactions(
    irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    // If successful or failure, we need to read 2 more things:
    // inReferenceTo and the responseLedger payload.
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    {
        const char* pElementExpected = "responseLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there, or the Payload isn't
    // there, then failure.
    if (!m_ascInReferenceTo.GetLength() || (!m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected responseLedger and/or inReferenceTo elements "
                 "with text fields in "
                 "@notarizeTransactions reply\n";
        return (-1); // error condition
    }

    //        OTString acctContents(m_ascPayload);
    otWarn << "\n Command: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\n NymID:    " << m_strNymID << "\n AccountID: " << m_strAcctID
           << "\n ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",
    //    m_ascInReferenceTo.Get(),
    // acctContents.Get()

    return 1;
}

int32_t OTMessage::processXmlNodeGetInbox(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAccountID:    " << m_strAcctID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetNymbox(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetInbox(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strInboxHash = xml->getAttributeValue("inboxHash");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "inboxLedger";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID:    " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetNymbox(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "nymboxLedger";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\n"
                                              "ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetOutbox(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAccountID:    " << m_strAcctID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetOutbox(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strOutboxHash = xml->getAttributeValue("outboxHash");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "outboxLedger";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID:    " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetAccount(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAccountID:    " << m_strAcctID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetAccount(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "assetAccount";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID:    " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetAccountFiles(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAccountID:    " << m_strAcctID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetAccountFiles(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strInboxHash = xml->getAttributeValue("inboxHash");
    m_strOutboxHash = xml->getAttributeValue("outboxHash");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "acctFiles";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "\nAccountID:    " << m_strAcctID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetContract(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAsset Type:    " << m_strAssetID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetContract(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "assetContract";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID
           << "\nAsset Type ID:    " << m_strAssetID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeGetMint(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nAsset Type:    " << m_strAssetID
           << "\nRequest #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtGetMint(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAssetID = xml->getAttributeValue("assetType");

    const char* pElementExpected;
    if (m_bSuccess)
        pElementExpected = "mint";
    else
        pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    if (m_bSuccess)
        m_ascPayload = ascTextExpected;
    else
        m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID
           << "\nAsset Type ID:    " << m_strAssetID
           << "\n"
              "ServerID: " << m_strServerID << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeTriggerClause(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strNymID2 = xml->getAttributeValue("clauseName");
    m_strRequestNum = xml->getAttributeValue("requestNum");
    const OTString strHasParam = xml->getAttributeValue("hasParam");

    OTString strTransactionNum = xml->getAttributeValue("smartContractID");
    if (strTransactionNum.Exists())
        m_lTransactionNum = atol(strTransactionNum.Get());

    if (strHasParam.Compare("true")) {
        const char* pElementExpected = "parameter";
        OTASCIIArmor ascTextExpected;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
        else
            m_ascPayload = ascTextExpected;
    }

    otWarn << "\nCommand: " << m_strCommand << "\nNymID:    " << m_strNymID
           << "\nServerID: " << m_strServerID
           << "\nClause TransNum and Name:  " << m_lTransactionNum << "  /  "
           << m_strNymID2 << " \n"
                             "Request #: " << m_strRequestNum << "\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtTriggerClause(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    const char* pElementExpected = "inReferenceTo";

    OTASCIIArmor ascTextExpected;

    if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected)) {
        otErr << "Error in OTMessage::ProcessXMLNode: "
                 "Expected " << pElementExpected
              << " element with text field, for " << m_strCommand << ".\n";
        return (-1); // error condition
    }

    m_ascInReferenceTo = ascTextExpected;

    otWarn << "\nCommand: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\nNymID:    " << m_strNymID << "   ServerID: " << m_strServerID
           << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeProcessInbox(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    {
        const char* pElementExpected = "processLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n AccountID:    " << m_strAcctID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeProcessNymbox(irr::io::IrrXMLReader*& xml)
{
    m_strCommand = xml->getNodeName(); // Command
    m_strNymID = xml->getAttributeValue("nymID");
    m_strNymboxHash = xml->getAttributeValue("nymboxHash");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strRequestNum = xml->getAttributeValue("requestNum");

    {
        const char* pElementExpected = "processLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    otWarn << "\n Command: " << m_strCommand << " \n NymID:    " << m_strNymID
           << "\n"
              " ServerID: " << m_strServerID
           << "\n Request#: " << m_strRequestNum << "\n\n";

    return 1;
}

int32_t OTMessage::processXmlNodeAtProcessInbox(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");
    m_strAcctID = xml->getAttributeValue("accountID");

    // If successful or failure, we need to read 2 more things:
    // inReferenceTo and the responseLedger payload.
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    {
        const char* pElementExpected = "responseLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there, or the Payload isn't
    // there, then failure.
    if (!m_ascInReferenceTo.GetLength() || (!m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected responseLedger and/or inReferenceTo elements "
                 "with text fields in "
                 "@processInbox reply\n";
        return (-1); // error condition
    }

    otWarn << "\n Command: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\n NymID:    " << m_strNymID << "\n AccountID: " << m_strAcctID
           << "\n ServerID: " << m_strServerID << "\n\n";
    //    "****New Account****:\n%s\n",

    return 1;
}

int32_t OTMessage::processXmlNodeAtProcessNymbox(irr::io::IrrXMLReader*& xml)
{
    processXmlSuccess(xml);

    m_strCommand = xml->getNodeName(); // Command
    m_strRequestNum = xml->getAttributeValue("requestNum");
    m_strNymID = xml->getAttributeValue("nymID");
    m_strServerID = xml->getAttributeValue("serverID");

    // If successful or failure, we need to read 2 more things:
    // inReferenceTo and the responseLedger payload.
    // At this point, we do not send the REASON WHY if it failed.

    {
        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m_ascInReferenceTo;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    {
        const char* pElementExpected = "responseLedger";
        OTASCIIArmor& ascTextExpected = m_ascPayload;

        if (!OTContract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                    pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m_strCommand << ".\n";
            return (-1); // error condition
        }
    }

    // Did we find everything we were looking for?
    // If the "command responding to" isn't there, or the Payload isn't
    // there, then failure.
    if (!m_ascInReferenceTo.GetLength() || (!m_ascPayload.GetLength())) {
        otErr << "Error in OTMessage::ProcessXMLNode:\n"
                 "Expected responseLedger and/or inReferenceTo elements "
                 "with text fields in "
                 "@processNymbox reply\n";
        return (-1); // error condition
    }

    otWarn << "\n Command: " << m_strCommand << "   "
           << (m_bSuccess ? "SUCCESS" : "FAILED")
           << "\n NymID:    " << m_strNymID << "\n"
                                               " ServerID: " << m_strServerID
           << "\n\n";
    //    "****New Account****:\n%s\n",

    return 1;
}

// Most contracts do not override this function...
// But OTMessage does, because every request sent to the server needs to be
// signed.
// And each new request is a new message, that requires a new signature, unlike
// most
// contracts, (that always stay the same after they are signed.)
//
// We need to update the m_xmlUnsigned member with the message members before
// the
// actual signing occurs. (Presumably this is the whole reason why the account
// is being re-signed.)
//
// Normally, in other OTContract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change.
// But as new messages are sent, they must be signed. This function insures that
// the most up-to-date member contents are included in the request before it is
// signed.
//
// Note: Above comment is slightly old. This override is now here only for the
// purpose
// of releasing the signatures.  The other functionality is now handled by the
// UpdateContents member, which is called by the framework, and otherwise empty
// in
// default, but child classes such as OTMessage and OTAccount override it to
// save
// their contents just before signing.
// See OTMessage::UpdateContents near the top of this file for an example.
//
bool OTMessage::SignContract(const OTPseudonym& theNym,
                             const OTPasswordData* pPWData)
{
    // I release these, I assume, because a message only has one signer.
    ReleaseSignatures(); // Note: this might change with credentials. We might
                         // require multiple signatures.

    // Use the authentication key instead of the signing key.
    //
    m_bIsSigned = OTContract::SignContractAuthent(theNym, pPWData);

    if (m_bIsSigned) {
        //        otErr <<
        // "\n******************************************************\n"
        //                "Contents of signed
        // message:\n\n%s******************************************************\n\n",
        // m_xmlUnsigned.Get());
    }
    else
        otWarn << "Failure signing message:\n" << m_xmlUnsigned << "";

    return m_bIsSigned;
}

// virtual (OTContract)
bool OTMessage::VerifySignature(const OTPseudonym& theNym,
                                const OTPasswordData* pPWData) const
{
    // Messages, unlike many contracts, use the authentication key instead of
    // the signing key. This is because signing keys are meant for signing
    // legally
    // binding agreements, whereas authentication keys are used for message
    // transport
    // and for file storage. Since this is OTMessage specifically, used for
    // transport,
    // we have overridden sign and verify contract methods, to explicitly use
    // the
    // authentication key instead of the signing key. OTSignedFile should
    // probably be
    // the same way. (Maybe it already is, by the time you are reading this.)
    //
    return VerifySigAuthent(theNym, pPWData);
}

// Unlike other contracts, which do not change over time, and thus calculate
// their ID
// from a hash of the file itself, OTMessage objects are different every time.
// Thus, we
// cannot use a hash of the file to produce the Message ID.
//
// Message ID will probably become an important part of the protocol (to prevent
// replay attacks..)
// So I will end up using it. But for now, VerifyContractID will always return
// true.
//
bool OTMessage::VerifyContractID() const
{
    return true;
}

OTMessage::OTMessage()
    : OTContract()
    , m_bIsSigned(false)
    , m_lNewRequestNum(0)
    , m_lDepth(0)
    , m_lTransactionNum(0)
    , m_bSuccess(false)
    , m_bBool(false)
    , m_lTime(0)

{
    OTContract::m_strContractType.Set("MESSAGE");
}

OTMessage::~OTMessage()
{
}

} // namespace opentxs
