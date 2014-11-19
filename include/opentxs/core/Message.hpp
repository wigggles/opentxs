/************************************************************
 *
 *  OTMessage.hpp
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

#ifndef OPENTXS_CORE_OTMESSAGE_HPP
#define OPENTXS_CORE_OTMESSAGE_HPP

#include "crypto/OTASCIIArmor.hpp"
#include "Contract.hpp"
#include "OTNumList.hpp"

#include <unordered_map>
#include <memory>

namespace opentxs
{

class OTPasswordData;
class Nym;
class Message;

class OTMessageStrategy
{
public:
    virtual int32_t processXml(Message& message,
                               irr::io::IrrXMLReader*& xml) = 0;
    virtual String writeXml(Message& message) = 0;
    virtual ~OTMessageStrategy();

    void processXmlSuccess(Message& m, irr::io::IrrXMLReader*& xml);
};

class OTMessageStrategyManager
{
public:
    OTMessageStrategy* findStrategy(std::string name)
    {
        auto strategy = mapping.find(name);
        if (strategy == mapping.end()) return nullptr;
        return strategy->second.get();
    }
    void registerStrategy(std::string name, OTMessageStrategy* strategy)
    {
        mapping[name] = std::unique_ptr<OTMessageStrategy>(strategy);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<OTMessageStrategy>> mapping;
};

class Message : public Contract
{
protected:
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);

    virtual void UpdateContents();

    bool m_bIsSigned;

private:
    bool updateContentsByType();

    int32_t processXmlNodeAckReplies(Message& m, irr::io::IrrXMLReader*& xml);
    int32_t processXmlNodeAcknowledgedReplies(Message& m,
                                              irr::io::IrrXMLReader*& xml);
    int32_t processXmlNodeOTmessage(Message& m, irr::io::IrrXMLReader*& xml);

public:
    EXPORT Message();
    EXPORT virtual ~Message();

    virtual bool VerifyContractID() const;

    EXPORT virtual bool SignContract(const Nym& theNym,
                                     const OTPasswordData* pPWData = nullptr);
    EXPORT virtual bool VerifySignature(
        const Nym& theNym, const OTPasswordData* pPWData = nullptr) const;

    EXPORT bool HarvestTransactionNumbers(
        Nym& theNym,
        bool bHarvestingForRetry,           // false until positively asserted.
        bool bReplyWasSuccess,              // false until positively asserted.
        bool bReplyWasFailure,              // false until positively asserted.
        bool bTransactionWasSuccess,        // false until positively asserted.
        bool bTransactionWasFailure) const; // false until positively asserted.

    // So the message can get the list of numbers from the Nym, before sending,
    // that should be listed as acknowledged that the server reply has already
    // been
    // seen for those request numbers.
    // IMPORTANT NOTE: The Server ID is used to lookup the numbers from the Nym.
    // Therefore,
    // make sure that OTMessage::m_strServerID is set BEFORE calling this
    // function. (It will
    // ASSERT if you don't...)
    //
    EXPORT void SetAcknowledgments(Nym& theNym);

    EXPORT static void registerStrategy(std::string name,
                                        OTMessageStrategy* strategy);

    String m_strCommand;  // perhaps @register is the string for "reply to
                          // register" a-ha
    String m_strServerID; // This is sent with every message for security
                          // reasons.
    String m_strNymID;    // The hash of the user's public key... or x509 cert.
    String m_strNymboxHash; // Sometimes in a server reply as FYI, sometimes
                            // in user message for validation purposes.
    String m_strInboxHash;  // Sometimes in a server reply as FYI, sometimes in
                            // user message for validation purposes.
    String m_strOutboxHash; // Sometimes in a server reply as FYI, sometimes
                            // in user message for validation purposes.
    String m_strNymID2;     // If the user requests public key of another user.
                            // ALSO used for MARKET ID sometimes.
    String m_strNymPublicKey; // The user's public key... or x509 cert.
    String m_strAssetID;      // The hash of the contract for whatever digital
                              // asset is referenced.
    String m_strAcctID;       // The unique ID of an asset account.
    String m_strType;         // .
    String m_strRequestNum;   // Every user has a request number. This prevents
                              // messages from
                              // being intercepted and repeated by attackers.

    OTASCIIArmor m_ascInReferenceTo; // If the server responds to a user
                                     // command, he sends
    // it back to the user here in ascii armored format.
    OTASCIIArmor m_ascPayload; // If the reply needs to include a payload (such
                               // as a new account
    // or a message envelope or request from another user etc) then
    // it can be put here in ascii-armored format.
    OTASCIIArmor m_ascPayload2; // Sometimes one payload just isn't enough.
    OTASCIIArmor m_ascPayload3; // Sometimes two payload just isn't enough.

    // This list of request numbers is stored for optimization, so client/server
    // can communicate about
    // which messages have been received, and can avoid certain downloads, such
    // as replyNotice Box Receipts.
    //
    OTNumList m_AcknowledgedReplies; // Client request: list of server replies
                                     // client has already seen.
    // Server reply:   list of client-acknowledged replies (so client knows that
    // server knows.)

    int64_t m_lNewRequestNum; // If you are SENDING a message, you set
                              // m_strRequestNum. (For all msgs.)
    // Server Reply for all messages copies that same number into
    // m_strRequestNum;
    // But if this is a SERVER REPLY to the "getRequestNumber" MESSAGE, the
    // "request number" expected in that reply is stored HERE in
    // m_lNewRequestNum;
    int64_t m_lDepth;          // For Market-related messages... (Plus for usage
                               // credits.) Also used by getBoxReceipt
    int64_t m_lTransactionNum; // For Market-related messages... Also used by
                               // getBoxReceipt

    bool m_bSuccess; // When the server replies to the client, this may be true
                     // or false
    bool m_bBool;    // Some commands need to send a bool. This variable is for
                     // those.
    int64_t m_lTime; // Timestamp when the message was signed.

    static OTMessageStrategyManager messageStrategyManager;
};

class RegisterStrategy
{
public:
    RegisterStrategy(std::string name, OTMessageStrategy* strategy)
    {
        Message::registerStrategy(name, strategy);
    }
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTMESSAGE_HPP
