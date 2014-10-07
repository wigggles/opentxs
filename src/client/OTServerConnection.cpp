/************************************************************
*
*  OTServerConnection.cpp
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

#include "../core/stdafx.hpp"

#include "OTServerConnection.hpp"
#include "OpenTransactions.hpp"
#include "OTClient.hpp"
#include "OTWallet.hpp"
#include "TransportCallback.hpp"

#include "../core/OTAssetContract.hpp"
#include "../core/util/OTDataCheck.hpp"
#include "../core/crypto/OTEnvelope.hpp"
#include "../core/OTLog.hpp"
#include "../core/OTMessage.hpp"
#include "../core/OTPayload.hpp"
#include "../core/OTPseudonym.hpp"
#include "../core/OTServerContract.hpp"

extern "C" {
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif
}

namespace opentxs
{

int32_t allow_debug = 1;

void SetupHeader(u_header& theCMD, int32_t nTypeID, int32_t nCmdID,
                 OTPayload& thePayload)
{
    uint32_t lSize = thePayload.GetSize(); // outputting in normal byte order,
                                           // but sent to network in network
                                           // byte order.

    theCMD.fields.type_id = (nTypeID > 0) ? static_cast<BYTE>(nTypeID) : '\0';
    theCMD.fields.command_id = (nCmdID > 0) ? static_cast<BYTE>(nCmdID) : '\0';
    ;
    theCMD.fields.size =
        htonl(lSize); // think this is causing problems.. maybe not...
    theCMD.fields.checksum = CalcChecksum(theCMD.buf, OT_CMD_HEADER_SIZE - 1);

    BYTE byChecksum = (BYTE)theCMD.fields.checksum;
    int32_t nChecksum = byChecksum;

    otLog4 << "OT_CMD_HEADER_SIZE: " << OT_CMD_HEADER_SIZE
           << " -- CMD TYPE: " << nTypeID << " -- CMD NUM: " << nCmdID
           << " -- (followed by 2 bytes of filler)\n"
              "PAYLOAD SIZE: " << lSize << " -- CHECKSUM: " << nChecksum
           << "\n";

    otLog5 << "First 9 bytes are:";
    for (int i = 0; i < 9; i++) {
        otLog5 << " " << int(theCMD.buf[i]);
    }
    otLog5 << "\n";
}

bool OTServerConnection::s_bInitialized = false;

void OTServerConnection::Initialize()
{
    // We've already been initialized. We can just return
    if (s_bInitialized) {
        return;
    }

    // This is the first time this function has run.
    s_bInitialized = true; // set this to true so the function can't run again.
                           // It only runs the first time.

    // Initialize SSL -- MUST happen before any Private keys are loaded, if you
    // want it to work.
    // Update: this no longer initializes OpenSSL, which I do in
    // OTLog::OT_Init() now.
    // Just make sure you call that early on.
    //
    //    SFSocketGlobalInit();
}

// Connect is used for SSL mode, but SetFocus is used for XmlRpc mode.
// Everytime you send a message, it is a new connection -- and could be to
// a different server! So it's important to switch focus each time so we
// have the right server contract, etc.
//
bool OTServerConnection::SetFocus(OTPseudonym& theNym,
                                  OTServerContract& theServerContract,
                                  TransportCallback* pCallback)
{
    if (nullptr == pCallback) {
        otErr << __FUNCTION__ << ": pCallback is nullptr";
        OT_FAIL;
    };

    // We're already connected! You can't use SetFocus if you're in connection
    // mode (TCP instead of HTTP.)
    // if (IsConnected())
    //    return false;
    //
    // This call initializes OpenSSL (only the first time it's called.)
    Initialize();

    // The Client keeps an internal ServerConnection at all times.
    // In SSL/TCP mode, you connect, and stay connected. But in RPC
    // mode, you must call SetFocus before each time you prepare or
    // process any server-related messages. Why? Since the Client normally
    // expects the connection to already be made, and therefore expects
    // to have access to the Nym and Server Contract (and server ID, etc)
    // available since those pointers are normally set during Connect().
    // Since we no longer connect in RPC mode, we must still make sure those
    // pointers are ready by calling SetFocus before they might end up being
    // used.
    // Each time you send a new message, it might be to a different server or
    // from a different nym. That's fine -- just call SetFocus() before you do
    // it.
    m_pNym = &theNym;
    m_pServerContract = &theServerContract;
    m_pCallback = pCallback; // This is what we get instead of a socket, when
                             // we're in RPC mode.
    m_bFocused = true;

    return true;
}

// When the server sends a reply back with our new request number, we
// need to update our records accordingly.
//
// This function is meant to be called when that happens, so that we
// can do just that.
//
void OTServerConnection::OnServerResponseToGetRequestNumber(
    int64_t lNewRequestNumber) const
{
    if (m_pNym && m_pServerContract) {
        otOut << "Received new request number from the server: "
              << lNewRequestNumber << ". Updating Nym records...\n";

        OTString strServerID;
        m_pServerContract->GetIdentifier(strServerID);
        m_pNym->OnUpdateRequestNum(*m_pNym, strServerID, lNewRequestNumber);
    }
    else {
        otErr << "Expected m_pNym or m_pServerContract to be not null in "
                 "OTServerConnection::OnServerResponseToGetRequestNumber.\n";
    }
}

bool OTServerConnection::GetServerID(OTIdentifier& theID) const
{
    if (m_pServerContract) {
        m_pServerContract->GetIdentifier(theID);
        return true;
    }
    return false;
}

// When a certain Nym opens a certain account on a certain server,
// that account is put onto a list of accounts inside the wallet.
// Therefore, a certain Nym's connection to a certain server will
// occasionally require access to those accounts. Therefore the
// server connection object needs to have a pointer to the wallet.
// There might be MORE THAN ONE connection per wallet, or only one,
// but either way the connections need a pointer to the wallet
// they are associated with, so they can access those accounts.
OTServerConnection::OTServerConnection(OTWallet& theWallet, OTClient& theClient)
{
    m_pCallback = nullptr;
    m_bFocused = false;
    m_pNym = nullptr;
    m_pServerContract = nullptr;
    m_pWallet = &theWallet;
    m_pClient = &theClient;
}

bool OTServerConnection::SignAndSend(OTMessage& theMessage) const
{
    if (m_pNym && m_pWallet && theMessage.SignContract(*m_pNym) &&
        theMessage.SaveContract()) {
        ProcessMessageOut(theMessage);
        return true;
    }

    return false;
}

void OTServerConnection::ProcessMessageOut(const OTMessage& theMessage) const
{
    u_header theCMD;
    OTPayload thePayload;

    // clear the header
    memset((void*)theCMD.buf, 0, OT_CMD_HEADER_SIZE);

    // Here is where we set up the Payload (so we have the size ready before the
    // header goes out)
    // This is also where we have turned on the encrypted envelopes  }:-)
    OTEnvelope theEnvelope; // All comms should be encrypted in one of these
                            // envelopes.

    // Testing encrypted envelopes...
    const OTPseudonym* pServerNym = nullptr;

    if (m_pServerContract &&
        (nullptr != (pServerNym = m_pServerContract->GetContractPublicNym()))) {
        OTString strEnvelopeContents;
        // Save the ready-to-go message into a string.
        theMessage.SaveContractRaw(strEnvelopeContents);

        // Seal the string up into an encrypted Envelope
        theEnvelope.Seal(*pServerNym, strEnvelopeContents);

        // From here on out, theMessage is disposable. OTPayload takes over.
        // OTMessage doesn't care about checksums and headers.
        thePayload.SetEnvelope(theEnvelope);

        // Now that the payload is ready, we'll set up the header.
        SetupHeader(theCMD, CMD_TYPE_1, TYPE_1_CMD_2, thePayload);
    }
    // else, for whatever reason, we just send an UNencrypted message... (This
    // shouldn't happen anymore...) TODO remove.
    else {
        thePayload.SetMessagePayload(theMessage);

        // Now that the payload is ready, we'll set up the header.
        SetupHeader(theCMD, CMD_TYPE_1, TYPE_1_CMD_1, thePayload);
    }

    OT_ASSERT(IsFocused())
    OT_ASSERT(nullptr != m_pCallback);
    OT_ASSERT(nullptr != m_pServerContract);

    // Call the callback here.
    otOut << "\n=====>BEGIN Sending " << theMessage.m_strCommand
          << " message via ZMQ... Request number: "
          << theMessage.m_strRequestNum << "\n";

    (*m_pCallback)(*m_pServerContract, theEnvelope);

    otWarn << "<=====END Finished sending " << theMessage.m_strCommand
           << " message (and hopefully receiving "
              "a reply.)\nRequest number: " << theMessage.m_strRequestNum
           << "\n\n";

    // At this point, we have sent the envelope to the server.
}

// This function interprets test input (so should have been in test client?)
// then it uses that to send a message to server.
// The buf passed in is simply data collected by fgets from stdin.
void OTServerConnection::ProcessMessageOut(const char* buf, const int32_t*)
{

    OT_ASSERT(nullptr != buf);

    bool bSendCommand = false;
    bool bSendPayload = false;

    OTMessage theMessage;

    bool bHandledIt = false;

    u_header theCMD;

    // clear the header
    memset((void*)theCMD.buf, 0, OT_CMD_HEADER_SIZE);

    // Simple rule here: In each of the below if statements,
    // YOU MUST set up the header bytes HERE!
    // OR you must set the boolean bSendCommand TO TRUE!!
    // It must be one or the other in each block.
    // If you set to true, code at the bottom will calculate header
    // for you. If you fail to do this, the header is now uncalculated either
    // way.
    // Don't send uncalculated headers to the server unless doing it on purpose
    // for TESTING.

    if (buf[0] == '1') {
        bHandledIt = true;

        theCMD.fields.type_id = CMD_TYPE_1;
        theCMD.fields.command_id = TYPE_1_CMD_1;
        theCMD.fields.size = 0;
        theCMD.fields.checksum =
            CalcChecksum(theCMD.buf, OT_CMD_HEADER_SIZE - 1);

        int32_t nChecksum = theCMD.fields.checksum;

        otOut << "(User has instructed to send a size " << OT_CMD_HEADER_SIZE
              << ", TYPE 1 "
                 "COMMAND to the server...)\n CHECKSUM: " << nChecksum << "\n";
        bSendCommand = true;
    }
    else if (buf[0] == '2') {
        bHandledIt = true;

        theCMD.fields.type_id = 12;
        theCMD.fields.command_id = 3;
        theCMD.fields.size = 98;
        theCMD.fields.checksum =
            CalcChecksum(theCMD.buf, OT_CMD_HEADER_SIZE - 1);

        otOut << "(User has instructed to send a size "
              << (OT_CMD_HEADER_SIZE + 3) << ", **malformed "
                                             "command** to the server...)\n";
        bSendCommand = true;
    }
    // Empty OTMessage including signed XML, but no other commands
    else if (buf[0] == '3') {
        otOut << "(User has instructed to create a signed XML message "
                 "and send it to the server...)\n";
        bHandledIt = true;

        // Normally you'd update the member variables here, before signing it.
        // But this is just an empty OTMessage.

        // When a message is signed, it updates its m_xmlUnsigned contents to
        // the values in the members variables
        m_pWallet->SignContractWithFirstNymOnList(theMessage);

        // SaveContract takes m_xmlUnsigned and wraps it with the signatures and
        // ------- BEGIN  bookends
        // If you don't pass a string in, then SaveContract saves the new
        // version to its member, m_strRawFile
        theMessage.SaveContract();

        bSendCommand = true;
        bSendPayload = true;
    }

    // Above are various test messages.

    // This section for commands that involve building full XML messages,
    // that is, most of the real implementation of the transaction protocol.

    // If we can match the user's request to a client command,
    // AND theClient object is able to process that request into
    // a payload, THEN we create the header and send it all down the pipe.

    if (!bHandledIt && m_pNym && m_pServerContract) {
        // check server ID command
        if (buf[0] == 'c') {
            otOut << "(User has instructed to send a checkServerID "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::checkServerID,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing checkServerID command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // register new user account
        else if (buf[0] == 'r') {
            otOut << "(User has instructed to send a createUserAccount "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::createUserAccount,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing createUserAccount command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // ALL MESSAGES BELOW THIS POINT SHOULD ATTACH A REQUEST NUMBER IF THEY
        // EXPECT THE SERVER TO PROCESS THEM.
        // (Handled inside ProcessUserCommand)

        // checkUser
        else if (buf[0] == 'u') {
            otOut << "(User has instructed to send a checkUser command "
                     "to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::checkUser, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr)) // nullptr pAccount on
                                                        // this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing checkUser command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // register new asset account
        else if (buf[0] == 'a') {
            otOut << "(User has instructed to send a createAccount "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(
                    OTClient::createAccount, theMessage, *m_pNym,
                    *m_pServerContract,
                    nullptr)) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing createAccount command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // issue a new asset type
        else if (!strcmp(buf, "issue\n")) {
            otOut << "(User has instructed to send an issueAssetType "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::issueAssetType,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing issueAssetType command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // issue a new basket asset type
        else if (!strcmp(buf, "basket\n")) {
            otOut << "(User has instructed to send an issueBasket "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::issueBasket, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing issueBasket command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // exchange in/out of a basket currency
        else if (!strcmp(buf, "exchange\n")) {
            otOut << "(User has instructed to send an exchangeBasket "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::exchangeBasket,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing exchangeBasket command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // make an offer and put it onto a market.
        else if (!strcmp(buf, "offer\n")) {
            otOut << "(User has instructed to send a marketOffer "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::marketOffer, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing marketOffer command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // set asset contract's name
        else if (!strcmp(buf, "setassetname\n")) {
            otOut << "(User has instructed to set an Asset Contract's "
                     "client-side name...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(
                    OTClient::setAssetName, theMessage, *m_pNym,
                    *m_pServerContract,
                    nullptr)) // nullptr pAccount on this command.
            {
                //                bSendCommand = true; // No message sent.
                //                bSendPayload = true;
            }
        }

        // set server contract's name
        else if (!strcmp(buf, "setservername\n")) {
            otOut << "(User has instructed to set a Server Contract's "
                     "client-side name...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::setServerName,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                //                bSendCommand = true; // No message sent.
                //                bSendPayload = true;
            }
        }

        // set nym name
        else if (!strcmp(buf, "setnymname\n")) {
            otOut
                << "(User has instructed to set a Nym's client-side name...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::setNymName, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                //                bSendCommand = true; // No message sent.
                //                bSendPayload = true;
            }
        }

        // set account name
        else if (!strcmp(buf, "setaccountname\n")) {
            otOut << "(User wants to set an Asset Account's client-side "
                     "name...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::setAccountName,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                //                bSendCommand = true; // No message sent.
                //                bSendPayload = true;
            }
        }

        // sendUserMessage
        else if (buf[0] == 's') {
            otOut << "(User has instructed to send a sendUserMessage "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::sendUserMessage,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing sendUserMessage command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // get nymbox
        else if (buf[0] == 'y') {
            otOut << "(User has instructed to send a getNymbox command "
                     "to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::getNymbox, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getNymbox command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // get inbox
        else if (buf[0] == 'i') {
            otOut << "(User has instructed to send a getInbox command "
                     "to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(
                    OTClient::getInbox, theMessage, *m_pNym, *m_pServerContract,
                    nullptr) > 0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getInbox command in ProcessMessage: "
                      << buf[0] << "\n";
        }

        // get outbox
        else if (buf[0] == 'o') {
            otOut << "(User has instructed to send a getOutbox command "
                     "to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::getOutbox, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getOutbox command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // deposit cheque
        else if (buf[0] == 'q') {
            otOut << "User has instructed to deposit a cheque...\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::notarizeCheque,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing deposit cheque command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // withdraw voucher
        else if (buf[0] == 'v') {
            otOut << "User has instructed to withdraw a voucher (like "
                     "a cashier's cheque)...\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::withdrawVoucher,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing withdraw voucher command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // withdraw cash
        else if (buf[0] == 'w') {
            otOut << "(User has instructed to withdraw cash...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::notarizeWithdrawal,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing withdraw command in ProcessMessage: "
                      << buf[0] << "\n";
        }

        // deposit tokens
        else if (buf[0] == 'd') {
            otOut << "(User has instructed to deposit cash tokens...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::notarizeDeposit,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing deposit command in ProcessMessage: "
                      << buf[0] << "\n";
        }

        // activate payment plan
        else if (!strcmp(buf, "plan\n")) {
            otOut << "User has instructed to activate a payment plan...\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::paymentPlan, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing payment plan command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // deposit purse
        else if (buf[0] == 'p') {
            otOut << "(User has instructed to deposit a purse "
                     "containing cash...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::notarizePurse,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing deposit command in ProcessMessage: "
                      << buf[0] << "\n";
        }

        // get account
        else if (!strcmp(buf, "test\n")) {
            otOut << "(User has instructed to perform a test...)\n";

            // if successful setting up the command payload...

            if (m_pNym) {
                OTString strMessage(
                    "Well well well, this is just a little bit of "
                    "plaintext.\nNotice there are NO NEWLINES at the start.\n"
                    "I'm just trying to make it as long as i can, so that\nI "
                    "can test the envelope and armor functionality.\n");

                otOut << "MESSAGE:\n------>" << strMessage << "<--------\n";

                OTASCIIArmor ascMessage(strMessage);

                otOut << "ASCII ARMOR:\n------>" << ascMessage << "<--------\n";

                OTEnvelope theEnvelope;
                theEnvelope.Seal(*m_pNym, strMessage);

                ascMessage.Release();

                theEnvelope.GetAsciiArmoredData(ascMessage);

                otOut << "ENCRYPTED PLAIN TEXT AND THEN ASCII "
                         "ARMOR:\n------>" << ascMessage << "<--------\n";

                strMessage.Release();

                OTEnvelope the2Envelope(ascMessage);
                the2Envelope.Open(*m_pNym, strMessage);

                otOut << "DECRYPTED PLAIN TEXT:\n------>" << strMessage
                      << "<--------\n";

                OTEnvelope the3Envelope;
                the3Envelope.Seal(*m_pNym, strMessage.Get());

                ascMessage.Release();

                the3Envelope.GetAsciiArmoredData(ascMessage);

                otOut << "RE-ENCRYPTED PLAIN TEXT AND THEN ASCII "
                         "ARMOR:\n------>" << ascMessage << "<--------\n";

                strMessage.Release();

                OTEnvelope the4Envelope(ascMessage);
                the4Envelope.Open(*m_pNym, strMessage);

                otOut << "RE-DECRYPTED PLAIN TEXT:\n------>" << strMessage
                      << "<--------\n";

                OTEnvelope the5Envelope;
                the5Envelope.Seal(*m_pNym, strMessage.Get());

                ascMessage.Release();

                the3Envelope.GetAsciiArmoredData(ascMessage);

                otOut << "RE-RE-ENCRYPTED PLAIN TEXT AND THEN ASCII "
                         "ARMOR:\n------>" << ascMessage << "<--------\n";

                strMessage.Release();

                OTEnvelope the6Envelope(ascMessage);
                the6Envelope.Open(*m_pNym, strMessage);

                otOut << "RE-RE-DECRYPTED PLAIN TEXT:\n------>" << strMessage
                      << "<--------\n";
            }

        }

        // get account
        else if (!strcmp(buf, "get\n")) {
            otOut << "(User has instructed to send a getAccount "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::getAccount, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getAccount command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // get contract
        else if (!strcmp(buf, "getcontract\n")) {
            otOut << "(User has instructed to send a getContract "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::getContract, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getContract command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // sign contract
        else if (!strcmp(buf, "signcontract\n")) {
            otOut << "Is the contract a server contract, or an asset "
                     "contract [s/a]: ";
            OTString strContractType;
            strContractType.OTfgets(std::cin);

            char cContractType = 's';
            bool bIsAssetContract = strContractType.At(0, cContractType);

            if (bIsAssetContract) {
                if ('S' == cContractType || 's' == cContractType)
                    bIsAssetContract = false;
            }
            otOut << "Is the contract properly escaped already? (If "
                     "escaped, all lines beginning with ----- will "
                     "instead appear as - ----- ) [y\n]: ";
            // User input.
            // I need a from account, Yes even in a deposit, it's still the
            // "From" account.
            // The "To" account is only used for a transfer. (And perhaps for a
            // 2-way trade.)
            OTString strEscape;
            strEscape.OTfgets(std::cin);

            char cEscape = 'n';
            bool bEscaped = strEscape.At(0, cEscape);

            if (bEscaped) {
                if ('N' == cEscape || 'n' == cEscape) bEscaped = false;
            }

            otOut << "Please enter an unsigned asset contract; "
                     "terminate with ~ on a new line:\n> ";
            OTString strContract;
            char decode_buffer[200]; // Safe since we only read
                                     // sizeof(decode_buffer)-1

            do {
                decode_buffer[0] = 0; // Make it fresh.

                if ((nullptr !=
                     fgets(decode_buffer, sizeof(decode_buffer) - 1, stdin)) &&
                    (decode_buffer[0] != '~')) {
                    if (!bEscaped && decode_buffer[0] == '-') {
                        strContract.Concatenate("- ");
                    }
                    strContract.Concatenate("%s", decode_buffer);
                    otOut << "> ";
                }
                else {
                    break;
                }

            } while (decode_buffer[0] != '~');

            OTServerContract theServerContract;
            OTAssetContract theAssetContract;

            OTContract* pContract =
                bIsAssetContract
                    ? dynamic_cast<OTContract*>(&theAssetContract)
                    : dynamic_cast<OTContract*>(&theServerContract);

            pContract->CreateContract(strContract, *m_pNym);

            // re-using strContract here for output this time.
            strContract.Release();
            pContract->SaveContractRaw(strContract);

            otOut << ".\n..\n...\n....\n.....\n......\n.......\n........\n....."
                     "....\n\nNEW CONTRACT:\n\n" << strContract << "\n";
        }

        // get mint
        else if (!strcmp(buf, "getmint\n")) {
            otOut << "(User has instructed to send a getMint command "
                     "to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(
                    OTClient::getMint, theMessage, *m_pNym, *m_pServerContract,
                    nullptr) > 0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getMint command in ProcessMessage: "
                      << buf[0] << "\n";
        }

        // notarize transfer
        else if (buf[0] == 't') {
            otOut << "(User has instructed to send a Transfer command "
                     "(Notarize Transactions) to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::notarizeTransfer,
                                              theMessage, *m_pNym,
                                              *m_pServerContract, nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing notarizeTransactions command "
                         "in ProcessMessage: " << buf[0] << "\n";
        }

        // getRequest
        else if (buf[0] == 'g') {
            otOut << "(User has instructed to send a getRequest "
                     "command to the server...)\n";

            // if successful setting up the command payload...

            if (m_pClient->ProcessUserCommand(OTClient::getRequest, theMessage,
                                              *m_pNym, *m_pServerContract,
                                              nullptr) >
                0) // nullptr pAccount on this command.
            {
                bSendCommand = true;
                bSendPayload = true;
            }
            else
                otErr << "Error processing getRequest command in "
                         "ProcessMessage: " << buf[0] << "\n";
        }

        // getTransactionNum
        else if (buf[0] == 'n') {
            // I just coded (here) for myself a secret option (for testing)...
            // Optionally instead of JUST 'n', I can put n <number>, (without
            // brackets) and
            // this code will add that number to my list of issued and
            // transaction numbers.
            // I already have the ability to clear the list, so now I can add
            // numbers to it as well.
            // (Which adds to both lists.)
            // I can also remove a number from the transaction list but LEAVE it
            // on the issued list,
            // for example by writing a cheque and throwing it away.
            //
            // This code is for testing and allows me to find and patch any
            // problems without
            // having to re-create my data each time -- speeds up debugging.
            //
            int64_t lTransactionNumber =
                ((strlen(buf) > 2) ? atol(&(buf[2])) : 0);

            if (lTransactionNumber > 0) {
                OTString strServerID;
                m_pServerContract->GetIdentifier(strServerID);

                m_pNym->AddTransactionNum(*m_pNym, strServerID,
                                          lTransactionNumber,
                                          true); // bool bSave=true

                otOut << "Transaction number " << lTransactionNumber
                      << " added to both lists "
                         "(on client side.)\n";
            }
            else {
                otOut << "(User has instructed to send a "
                         "getTransactionNum command to the "
                         "server...)\n";

                // if successful setting up the command payload...

                if (m_pClient->ProcessUserCommand(OTClient::getTransactionNum,
                                                  theMessage, *m_pNym,
                                                  *m_pServerContract, nullptr) >
                    0) // nullptr pAccount on this command.
                {
                    bSendCommand = true;
                    bSendPayload = true;
                }
                else
                    otErr << "Error processing getTransactionNum command "
                             "in ProcessMessage: " << buf[0] << "\n";
            }

        }
        else {
            if (allow_debug) {
                otOut << "\n";
            }
            return;
        }
    }
    else if (!bHandledIt) {
        otOut << "\n";
    }

    if (bSendCommand && bSendPayload) {
        // Voila -- it's sent. (If there was a payload involved.)
        ProcessMessageOut(theMessage);
    } // Otherwise... if it's a "header only" ...
}

} // namespace opentxs
