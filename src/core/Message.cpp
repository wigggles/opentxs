/************************************************************
 *
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
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/Message.hpp>

#include <opentxs/core/Ledger.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/Tag.hpp>

#include <fstream>
#include <cstring>

#include <irrxml/irrXML.hpp>

#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

// PROTOCOL DOCUMENT

// --- This is the file that implements the entire message protocol.
// (Transactions are in a different file.)

// true  == success (even if nothing harvested.)
// false == error.
//

namespace opentxs
{

OTMessageStrategyManager Message::messageStrategyManager;

bool Message::HarvestTransactionNumbers(
    Nym& theNym,
    bool bHarvestingForRetry,          // false until positively asserted.
    bool bReplyWasSuccess,             // false until positively asserted.
    bool bReplyWasFailure,             // false until positively asserted.
    bool bTransactionWasSuccess,       // false until positively asserted.
    bool bTransactionWasFailure) const // false until positively asserted.
{

    const Identifier MSG_NYM_ID(m_strNymID), NOTARY_ID(m_strNotaryID),
        ACCOUNT_ID(m_strAcctID.Exists() ? m_strAcctID
                                        : m_strNymID); // This may be
                                                       // unnecessary, but just
                                                       // in case.

    const String strLedger(m_ascPayload);

    Ledger theLedger(MSG_NYM_ID, ACCOUNT_ID, NOTARY_ID); // We're going to
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
// IMPORTANT NOTE: The Notary ID is used to lookup the numbers from the Nym.
// Therefore,
// make sure that OTMessage::m_strNotaryID is set BEFORE calling this function.
// (It will
// ASSERT if you don't...)
//
void Message::SetAcknowledgments(Nym& theNym)
{
    m_AcknowledgedReplies.Release();

    const Identifier theNotaryID(m_strNotaryID);

    for (auto& it : theNym.GetMapAcknowledgedNum()) {
        std::string strNotaryID = it.first;
        dequeOfTransNums* pDeque = it.second;
        OT_ASSERT(nullptr != pDeque);

        String OTstrNotaryID = strNotaryID.c_str();
        const Identifier theTempID(OTstrNotaryID);

        if (!(pDeque->empty()) &&
            (theNotaryID == theTempID)) // only for the matching notaryID.
        {
            for (uint32_t i = 0; i < pDeque->size(); i++) {
                const int64_t lAckRequestNumber = pDeque->at(i);

                m_AcknowledgedReplies.Add(lAckRequestNumber);
            }
            break; // We found it! Might as well break out.
        }
    } // for
}

// The framework (Contract) will call this function at the appropriate time.
// OTMessage is special because it actually does something here, when most
// contracts are read-only and thus never update their contents.
// Messages, obviously, are different every time, and this function will be
// called just prior to the signing of the message, in Contract::SignContract.
void Message::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    m_lTime = OTTimeGetCurrentTime();

    Tag tag("notaryMessage");

    tag.add_attribute("version", m_strVersion.Get());
    tag.add_attribute("dateSigned", formatTimestamp(m_lTime));

    if (!updateContentsByType(tag)) {
        TagPtr pTag(new Tag(m_strCommand.Get()));
        pTag->add_attribute("requestNum", m_strRequestNum.Get());
        pTag->add_attribute("success", formatBool(false));
        pTag->add_attribute("acctID", m_strAcctID.Get());
        pTag->add_attribute("nymID", m_strNymID.Get());
        pTag->add_attribute("notaryID", m_strNotaryID.Get());
        // The below was an XML comment in the previous version
        // of this code. It's unused.
        pTag->add_attribute("infoInvalid", "THIS IS AN INVALID MESSAGE");
        tag.add_tag(pTag);
    }

    // ACKNOWLEDGED REQUEST NUMBERS
    //
    // (For reducing the number of box receipts for replyNotices that
    // must be downloaded.)
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
        String strAck;
        if (m_AcknowledgedReplies.Output(strAck) && strAck.Exists()) {
            const OTASCIIArmor ascTemp(strAck);
            if (ascTemp.Exists()) {
                tag.add_tag("ackReplies", ascTemp.Get());
            }
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

bool Message::updateContentsByType(Tag& parent)
{
    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(m_strCommand.Get());
    if (!strategy) return false;
    strategy->writeXml(*this, parent);
    return true;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t Message::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //      return nReturnVal;

    const String strNodeName(xml->getNodeName());
    if (strNodeName.Compare("ackReplies")) {
        return processXmlNodeAckReplies(*this, xml);
    }
    else if (strNodeName.Compare("acknowledgedReplies")) {
        return processXmlNodeAcknowledgedReplies(*this, xml);
    }
    else if (strNodeName.Compare("notaryMessage")) {
        return processXmlNodeNotaryMessage(*this, xml);
    }

    OTMessageStrategy* strategy =
        messageStrategyManager.findStrategy(xml->getNodeName());
    if (!strategy) return 0;
    return strategy->processXml(*this, xml);
}

int32_t Message::processXmlNodeAckReplies(Message& m,
                                          irr::io::IrrXMLReader*& xml)
{
    String strDepth;
    if (!Contract::LoadEncodedTextField(xml, strDepth)) {
        otErr << "Error in OTMessage::ProcessXMLNode: ackReplies field "
                 "without value.\n";
        return (-1); // error condition
    }

    m_AcknowledgedReplies.Release();

    if (strDepth.Exists()) m_AcknowledgedReplies.Add(strDepth);

    return 1;
}

int32_t Message::processXmlNodeAcknowledgedReplies(Message& m,
                                                   irr::io::IrrXMLReader*& xml)
{
    otErr << "OTMessage::ProcessXMLNode: SKIPPING DEPRECATED FIELD: "
             "acknowledgedReplies\n";

    while (xml->getNodeType() != irr::io::EXN_ELEMENT_END) {
        xml->read();
    }

    return 1;
}

int32_t Message::processXmlNodeNotaryMessage(Message& m,
                                             irr::io::IrrXMLReader*& xml)
{
    m_strVersion = xml->getAttributeValue("version");

    String strDateSigned = xml->getAttributeValue("dateSigned");

    if (strDateSigned.Exists()) m_lTime = parseTimestamp(strDateSigned.Get());

    otInfo << "\n===> Loading XML for Message into memory structures...\n";

    return 1;
}

// OTString StrategyGetMarketListResponse::writeXml(OTMessage &message)

// int32_t StrategyGetMarketListResponse::processXml(OTMessage &message,
// irr::io::IrrXMLReader*& xml)

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
// Normally, in other Contract and derived classes, m_xmlUnsigned is read
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
bool Message::SignContract(const Nym& theNym, const OTPasswordData* pPWData)
{
    // I release these, I assume, because a message only has one signer.
    ReleaseSignatures(); // Note: this might change with credentials. We might
                         // require multiple signatures.

    // Use the authentication key instead of the signing key.
    //
    m_bIsSigned = Contract::SignContractAuthent(theNym, pPWData);

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

// virtual (Contract)
bool Message::VerifySignature(const Nym& theNym,
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
bool Message::VerifyContractID() const
{
    return true;
}

Message::Message()
    : Contract()
    , m_bIsSigned(false)
    , m_lNewRequestNum(0)
    , m_lDepth(0)
    , m_lTransactionNum(0)
    , m_bSuccess(false)
    , m_bBool(false)
    , m_lTime(0)

{
    Contract::m_strContractType.Set("MESSAGE");
}

Message::~Message()
{
}

void OTMessageStrategy::processXmlSuccess(Message& m,
                                          irr::io::IrrXMLReader*& xml)
{
    m.m_bSuccess = String(xml->getAttributeValue("success")).Compare("true");
}

void Message::registerStrategy(std::string name, OTMessageStrategy* strategy)
{
    messageStrategyManager.registerStrategy(name, strategy);
}

OTMessageStrategy::~OTMessageStrategy()
{
}

class StrategyGetMarketOffers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("marketID", m.m_strNymID2.Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n Market ID: " << m.m_strNymID2
               << "\n Request #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffers::reg("getMarketOffers",
                                              new StrategyGetMarketOffers());

class StrategyGetMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2.Get());

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketOffersResponse::reg(
    "getMarketOffersResponse", new StrategyGetMarketOffersResponse());

class StrategyGetMarketRecentTrades : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("marketID", m.m_strNymID2.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n Market ID: " << m.m_strNymID2
               << "\n Request #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTrades::reg(
    "getMarketRecentTrades", new StrategyGetMarketRecentTrades());

class StrategyGetMarketRecentTradesResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));
        pTag->add_attribute("marketID", m.m_strNymID2.Get());

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("marketID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n MarketID: " << m.m_strNymID2
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketRecentTradesResponse::reg(
    "getMarketRecentTradesResponse",
    new StrategyGetMarketRecentTradesResponse());

class StrategyGetNymMarketOffers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffers::reg(
    "getNymMarketOffers", new StrategyGetNymMarketOffers());

class StrategyGetNymMarketOffersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymMarketOffersResponse::reg(
    "getNymMarketOffersResponse", new StrategyGetNymMarketOffersResponse());

class StrategyPingNotary : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        TagPtr pAuthentKeyTag(new Tag("publicAuthentKey",    m.m_strNymPublicKey.Get()));
        TagPtr pEncryptKeyTag(new Tag("publicEncryptionKey", m.m_strNymID2.Get()));

        pAuthentKeyTag->add_attribute("type",
                                      OTAsymmetricKey::KeyTypeToString(static_cast<OTAsymmetricKey::KeyType>(m.keytypeAuthent_)).Get());
        pEncryptKeyTag->add_attribute("type",
                                      OTAsymmetricKey::KeyTypeToString(static_cast<OTAsymmetricKey::KeyType>(m.keytypeEncrypt_)).Get());

        pTag->add_tag(pAuthentKeyTag);
        pTag->add_tag(pEncryptKeyTag);

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        // -------------------------------------------------
        const char* pElementExpected = "publicAuthentKey";
        OTASCIIArmor ascTextExpected;

        String::Map temp_MapAttributesAuthent;
        temp_MapAttributesAuthent.insert(std::pair<std::string, std::string>(
            "type",
            "")); // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected,
                                                  &temp_MapAttributesAuthent)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }
        {
            // -----------------------------------------------
            auto it = temp_MapAttributesAuthent.find("type");

            if ((it != temp_MapAttributesAuthent.end())) // We expected this much.
            {
                std::string& str_type = it->second;

                if (str_type.size() >
                    0) // Success finding key type.
                {
                    m.keytypeAuthent_ = OTAsymmetricKey::StringToKeyType(str_type);
                }
            }
            // -----------------------------------------------
        }
        m.m_strNymPublicKey.Set(ascTextExpected);
        // -------------------------------------------------
        pElementExpected = "publicEncryptionKey";
        ascTextExpected.Release();

        String::Map temp_MapAttributesEncrypt;
        temp_MapAttributesEncrypt.insert(std::pair<std::string, std::string>(
            "type",
            "")); // Value should be "RSA" after reading.
        // -----------------------------------------------
        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                pElementExpected,
                                                &temp_MapAttributesEncrypt)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                    "Expected " << pElementExpected
                << " element with text field, for " << m.m_strCommand
                << ".\n";
            return (-1); // error condition
        }
        {
            // -----------------------------------------------
            auto it = temp_MapAttributesEncrypt.find("type");

            if ((it != temp_MapAttributesEncrypt.end())) // We expected this much.
            {
                std::string& str_type = it->second;

                if (str_type.size() >
                    0) // Success finding key type.
                {
                    m.keytypeEncrypt_ = OTAsymmetricKey::StringToKeyType(str_type);
                }
            }
        }
        // -----------------------------------------------
        m.m_strNymID2.Set(ascTextExpected);
        // -------------------------------------------------
        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\n\n Public signing key:\n" << m.m_strNymPublicKey
               << "\nPublic encryption key:\n" << m.m_strNymID2 << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotary::reg("pingNotary",
                                         new StrategyPingNotary());

class StrategyPingNotaryResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nSuccess: " << (m.m_bSuccess ? "true" : "false")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyPingNotaryResponse::reg(
    "pingNotaryResponse", new StrategyPingNotaryResponse());

class StrategyRegisterNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("publicnym", m.m_ascPayload.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_ascPayload.Set(xml->getAttributeValue("publicnym"));

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNym::reg("registerNym",
                                          new StrategyRegisterNym());

class StrategyRegisterNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2)) {
            pTag->add_tag("nymfile", m.m_ascPayload.Get());
        }

        if (m.m_ascInReferenceTo.GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        if (m.m_bSuccess) {
            const char* pElementExpected = "nymfile";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterNymResponse::reg(
    "registerNymResponse", new StrategyRegisterNymResponse());

class StrategyUnregisterNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNym::reg("unregisterNym",
                                            new StrategyUnregisterNym());

class StrategyUnregisterNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength() > 2) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected = "inReferenceTo";
        OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "  "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterNymResponse::reg(
    "unregisterNymResponse", new StrategyUnregisterNymResponse());

class StrategyCheckNym : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNym::reg("checkNym", new StrategyCheckNym());

class StrategyCheckNymResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        // This means new-style credentials are being sent, not just the public
        // key as before.
        const bool bCredentials = (m.m_ascPayload.Exists());
        OT_ASSERT(!m.m_bSuccess || bCredentials);

        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_bSuccess && bCredentials) {
            pTag->add_tag("publicnym", m.m_ascPayload.Get());
        }
        else {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        OTASCIIArmor ascTextExpected;
        const char* pElementExpected = nullptr;

        if (!m.m_bSuccess) {
            pElementExpected = "inReferenceTo";
            m.m_ascInReferenceTo = ascTextExpected;
            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                "Expected " << pElementExpected
                << " element with text field, for " << m.m_strCommand
                << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            pElementExpected = "publicnym";
            ascTextExpected.Release();

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
            m.m_ascPayload = ascTextExpected;
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: " << m.m_strNotaryID << "\nNym2 Public Key:\n"
                   << m.m_strNymPublicKey << "\n\n";
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\nNymID2:    " << m.m_strNymID2
                   << "\n"
                      "NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m.m_ascInReferenceTo.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyCheckNymResponse::reg("checkNymResponse",
                                               new StrategyCheckNymResponse());

class StrategyUsageCredits : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("adjustment", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        String strAdjustment = xml->getAttributeValue("adjustment");

        if (strAdjustment.GetLength() > 0) m.m_lDepth = strAdjustment.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum
               << "\nAdjustment: " << m.m_lDepth << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCredits::reg("usageCredits",
                                           new StrategyUsageCredits());

class StrategyUsageCreditsResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("totalCredits", formatLong(m.m_lDepth));

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strTotalCredits = xml->getAttributeValue("totalCredits");

        if (strTotalCredits.GetLength() > 0)
            m.m_lDepth = strTotalCredits.ToLong();

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nTotal Credits: " << m.m_lDepth << " \n\n";
        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUsageCreditsResponse::reg(
    "usageCreditsResponse", new StrategyUsageCreditsResponse());

// This one isn't part of the message protocol, but is used for
// outmail storage.
// (Because outmail isn't encrypted like the inmail is, since the
// Nymfile itself will soon be encrypted, and there's no need to
// be redundant also as well in addition on top of that.
//
class StrategyOutpaymentsMessageOrOutmailMessage : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg(
    "outpaymentsMessage", new StrategyOutpaymentsMessageOrOutmailMessage());
RegisterStrategy StrategyOutpaymentsMessageOrOutmailMessage::reg2(
    "outmailMessage", new StrategyOutpaymentsMessageOrOutmailMessage());

class StrategySendNymMessage : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessage::reg("sendNymMessage",
                                             new StrategySendNymMessage());

class StrategySendNymMessageResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymMessageResponse::reg(
    "sendNymMessageResponse", new StrategySendNymMessageResponse());

// sendNymInstrument is sent from one user
// to the server, which then attaches that
// message as a payment, onto a transaction
// on the Nymbox of the recipient.
//
// payDividend is not a normal user
// message. Rather, the sender uses
// notarizeTransaction to do a
// payDividend transaction. On the
// server side, this creates a new
// message of type "payDividend"
// for each recipient, in order to
// attach a voucher to it (for each
// recipient) and then that
// (artificially created
// payDividend msg) is added to the
// Nymbox of each recipient.
class StrategySendNymInstrumentOrPayDividend : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength() > 2) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "messagePayload";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
    static RegisterStrategy reg2;
};
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg(
    "sendNymInstrument", new StrategySendNymInstrumentOrPayDividend());
RegisterStrategy StrategySendNymInstrumentOrPayDividend::reg2(
    "payDividend", new StrategySendNymInstrumentOrPayDividend());

class StrategySendNymInstrumentResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("nymID2", m.m_strNymID2.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymID2 = xml->getAttributeValue("nymID2");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nNymID2:    " << m.m_strNymID2
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategySendNymInstrumentResponse::reg(
    "sendNymInstrumentResponse", new StrategySendNymInstrumentResponse());

class StrategyGetRequestNumber : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestNumber::reg("getRequestNumber",
                                               new StrategyGetRequestNumber());

// This is the ONE command where you see a request number coming
// back from the server.
// In all the other commands, it should be SENT to the server, not
// received from the server.
class StrategyGetRequestResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("newRequestNum", formatLong(m.m_lNewRequestNum));
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const String strNewRequestNum = xml->getAttributeValue("newRequestNum");
        m.m_lNewRequestNum =
            strNewRequestNum.Exists() ? strNewRequestNum.ToLong() : 0;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest Number:    " << m.m_strRequestNum
               << "  New Number: " << m.m_lNewRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetRequestResponse::reg(
    "getRequestNumberResponse", new StrategyGetRequestResponse());

class StrategyRegisterInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("instrumentDefinition", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "instrumentDefinition";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinition::reg(
    "registerInstrumentDefinition", new StrategyRegisterInstrumentDefinition());

class StrategyRegisterInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());
        // the new issuer account ID
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("issuerAccount", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "issuerAccount";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected issuerAccount and/or inReferenceTo elements "
                     "with text fields in "
                     "registerInstrumentDefinitionResponse reply\n";
            return (-1); // error condition
        }

        String acctContents(m.m_ascPayload);
        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrument Definition ID: "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterInstrumentDefinitionResponse::reg(
    "registerInstrumentDefinitionResponse",
    new StrategyRegisterInstrumentDefinitionResponse());

class StrategyQueryInstrumentDefinitions : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("stringMap", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        const char* pElementExpected = "stringMap";
        OTASCIIArmor& ascTextExpected = m.m_ascPayload;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitions::reg(
    "queryInstrumentDefinitions", new StrategyQueryInstrumentDefinitions());

class StrategyQueryInstrumentDefinitionsResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("stringMap", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "stringMap";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected stringMap and/or inReferenceTo elements with "
                     "text fields in "
                     "queryInstrumentDefinitionsResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyQueryInstrumentDefinitionsResponse::reg(
    "queryInstrumentDefinitionsResponse",
    new StrategyQueryInstrumentDefinitionsResponse());

class StrategyIssueBasket : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("currencyBasket", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "currencyBasket";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the Payload isn't there, then failure.
        if (!m.m_ascPayload.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected currencyBasket element with text fields in "
                     "issueBasket message\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasket::reg("issueBasket",
                                          new StrategyIssueBasket());

class StrategyIssueBasketResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected inReferenceTo element with text fields in "
                     "issueBasketResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\nInstrumentDefinitionID: " << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyIssueBasketResponse::reg(
    "issueBasketResponse", new StrategyIssueBasketResponse());

class StrategyRegisterAccount : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << " \nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID
               << "\nRequest#: " << m.m_strRequestNum << "\nAsset Type:\n"
               << m.m_strInstrumentDefinitionID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccount::reg("registerAccount",
                                              new StrategyRegisterAccount());

class StrategyRegisterAccountResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.Exists()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.Exists()) {
            pTag->add_tag("newAccount", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful, we need to read 2 more things: inReferenceTo and
        // issuerAccount payload.
        // If failure, then we only need to read 1 thing: inReferenceTo
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                //                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "newAccount";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        //
        if (m.m_bSuccess && !m.m_ascPayload.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected newAccount element with text field, in "
                     "registerAccountResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyRegisterAccountResponse::reg(
    "registerAccountResponse", new StrategyRegisterAccountResponse());

class StrategyGetBoxReceipt : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        // If retrieving box receipt for Nymbox, NymID
        // will appear in this variable.
        pTag->add_attribute("accountID", m.m_strAcctID.Get());
        pTag->add_attribute("boxType", // outbox is 2.
                            (m.m_lDepth == 0)
                                ? "nymbox"
                                : ((m.m_lDepth == 1) ? "inbox" : "outbox"));
        pTag->add_attribute("transactionNum", formatLong(m.m_lTransactionNum));

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        String strTransactionNum = xml->getAttributeValue("transactionNum");
        m.m_lTransactionNum =
            strTransactionNum.Exists() ? strTransactionNum.ToLong() : 0;

        const String strBoxType = xml->getAttributeValue("boxType");

        if (strBoxType.Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType.Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType.Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceipt\n";
            return (-1);
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum
               << "  Transaction#: " << m.m_lTransactionNum << "   boxType: "
               << ((m.m_lDepth == 0) ? "nymbox" : (m.m_lDepth == 1) ? "inbox"
                                                                    : "outbox")
               << "\n\n"; // outbox is 2.);

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceipt::reg("getBoxReceipt",
                                            new StrategyGetBoxReceipt());

class StrategyGetBoxReceiptResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());
        pTag->add_attribute("boxType", // outbox is 2.
                            (m.m_lDepth == 0)
                                ? "nymbox"
                                : ((m.m_lDepth == 1) ? "inbox" : "outbox"));
        pTag->add_attribute("transactionNum", formatLong(m.m_lTransactionNum));

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("boxReceipt", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        String strTransactionNum = xml->getAttributeValue("transactionNum");
        m.m_lTransactionNum =
            strTransactionNum.Exists() ? strTransactionNum.ToLong() : 0;

        const String strBoxType = xml->getAttributeValue("boxType");

        if (strBoxType.Compare("nymbox"))
            m.m_lDepth = 0;
        else if (strBoxType.Compare("inbox"))
            m.m_lDepth = 1;
        else if (strBoxType.Compare("outbox"))
            m.m_lDepth = 2;
        else {
            m.m_lDepth = 0;
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxType to be inbox, outbox, or nymbox, in "
                     "getBoxReceiptResponse reply\n";
            return (-1);
        }

        // inReferenceTo contains the getBoxReceipt (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (m.m_bSuccess) {
            const char* pElementExpected = "boxReceipt";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there,
        // OR if it was successful but the Payload isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (m.m_bSuccess && !m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected boxReceipt and/or inReferenceTo elements with "
                     "text fields in "
                     "getBoxReceiptResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetBoxReceiptResponse::reg(
    "getBoxReceiptResponse", new StrategyGetBoxReceiptResponse());

class StrategyUnregisterAccount : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccount::reg(
    "unregisterAccount", new StrategyUnregisterAccount());

class StrategyUnregisterAccountResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // inReferenceTo contains the unregisterAccount (original request)
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, then failure.
        if (!m.m_ascInReferenceTo.GetLength()) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected inReferenceTo element with text fields in "
                     "unregisterAccountResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID: " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyUnregisterAccountResponse::reg(
    "unregisterAccountResponse", new StrategyUnregisterAccountResponse());

class StrategyNotarizeTransaction : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("accountLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "accountLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransaction::reg(
    "notarizeTransaction", new StrategyNotarizeTransaction());

class StrategyNotarizeTransactionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "notarizeTransactionResponse reply\n";
            return (-1); // error condition
        }

        //        OTString acctContents(m.m_ascPayload);
        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",
        //    m.m_ascInReferenceTo.Get(),
        // acctContents.Get()

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyNotarizeTransactionResponse::reg(
    "notarizeTransactionResponse", new StrategyNotarizeTransactionResponse());

class StrategyGetTransactionNumbers : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbers::reg(
    "getTransactionNumbers", new StrategyGetTransactionNumbers());

class StrategyGetTransactionNumbersResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetTransactionNumbersResponse::reg(
    "getTransactionNumbersResponse",
    new StrategyGetTransactionNumbersResponse());

class StrategyGetNymbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymbox::reg("getNymbox", new StrategyGetNymbox());

class StrategyGetNymboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("nymboxLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "nymboxLedger";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetNymboxResponse::reg(
    "getNymboxResponse", new StrategyGetNymboxResponse());

class StrategyGetAccountData : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAccountID:    " << m.m_strAcctID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountData::reg("getAccountData",
                                             new StrategyGetAccountData());

class StrategyGetAccountDataResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());
        pTag->add_attribute("inboxHash", m.m_strInboxHash.Get());
        pTag->add_attribute("outboxHash", m.m_strOutboxHash.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess) {
            if (m.m_ascPayload.GetLength()) {
                pTag->add_tag("account", m.m_ascPayload.Get());
            }
            if (m.m_ascPayload2.GetLength()) {
                pTag->add_tag("inbox", m.m_ascPayload2.Get());
            }
            if (m.m_ascPayload3.GetLength()) {
                pTag->add_tag("outbox", m.m_ascPayload3.Get());
            }
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strInboxHash = xml->getAttributeValue("inboxHash");
        m.m_strOutboxHash = xml->getAttributeValue("outboxHash");

        if (m.m_bSuccess) {
            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload,
                                                      "account")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected account "
                         "element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload2,
                                                      "inbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected inbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascPayload3,
                                                      "outbox")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected outbox"
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        if (!m.m_bSuccess) {
            if (!Contract::LoadEncodedTextFieldByName(xml, m.m_ascInReferenceTo,
                                                      "inReferenceTo")) {
                otErr << "Error in OTMessage::ProcessXMLNode: Expected "
                         "inReferenceTo element with text field, for "
                      << m.m_strCommand << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nAccountID:    " << m.m_strAcctID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetAccountDataResponse::reg(
    "getAccountDataResponse", new StrategyGetAccountDataResponse());

class StrategyGetInstrumentDefinition : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinition::reg(
    "getInstrumentDefinition", new StrategyGetInstrumentDefinition());

class StrategyGetInstrumentDefinitionResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("instrumentDefinition", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "instrumentDefinition";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetInstrumentDefinitionResponse::reg(
    "getInstrumentDefinitionResponse",
    new StrategyGetInstrumentDefinitionResponse());

class StrategyGetMint : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nAsset Type:    " << m.m_strInstrumentDefinitionID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMint::reg("getMint", new StrategyGetMint());

class StrategyGetMintResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("instrumentDefinitionID",
                            m.m_strInstrumentDefinitionID.Get());

        if (!m.m_bSuccess && m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_bSuccess && m.m_ascPayload.GetLength()) {
            pTag->add_tag("mint", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strInstrumentDefinitionID =
            xml->getAttributeValue("instrumentDefinitionID");

        const char* pElementExpected;
        if (m.m_bSuccess)
            pElementExpected = "mint";
        else
            pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        if (m.m_bSuccess)
            m.m_ascPayload = ascTextExpected;
        else
            m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "\nInstrument Definition ID:    "
               << m.m_strInstrumentDefinitionID
               << "\n"
                  "NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMintResponse::reg("getMintResponse",
                                              new StrategyGetMintResponse());

class StrategyProcessInbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("processLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "processLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n AccountID:    " << m.m_strAcctID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInbox::reg("processInbox",
                                           new StrategyProcessInbox());

class StrategyProcessInboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("accountID", m.m_strAcctID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strAcctID = xml->getAttributeValue("accountID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processInboxResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n AccountID: " << m.m_strAcctID
               << "\n NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessInboxResponse::reg(
    "processInboxResponse", new StrategyProcessInboxResponse());

class StrategyProcessNymbox : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("processLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        {
            const char* pElementExpected = "processLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        otWarn << "\n Command: " << m.m_strCommand
               << " \n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID
               << "\n Request#: " << m.m_strRequestNum << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymbox::reg("processNymbox",
                                            new StrategyProcessNymbox());

class StrategyProcessNymboxResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        if (m.m_ascPayload.GetLength()) {
            pTag->add_tag("responseLedger", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        // If successful or failure, we need to read 2 more things:
        // inReferenceTo and the responseLedger payload.
        // At this point, we do not send the REASON WHY if it failed.

        {
            const char* pElementExpected = "inReferenceTo";
            OTASCIIArmor& ascTextExpected = m.m_ascInReferenceTo;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        {
            const char* pElementExpected = "responseLedger";
            OTASCIIArmor& ascTextExpected = m.m_ascPayload;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
        }

        // Did we find everything we were looking for?
        // If the "command responding to" isn't there, or the Payload isn't
        // there, then failure.
        if (!m.m_ascInReferenceTo.GetLength() ||
            (!m.m_ascPayload.GetLength())) {
            otErr << "Error in OTMessage::ProcessXMLNode:\n"
                     "Expected responseLedger and/or inReferenceTo elements "
                     "with text fields in "
                     "processNymboxResponse reply\n";
            return (-1); // error condition
        }

        otWarn << "\n Command: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\n NymID:    " << m.m_strNymID
               << "\n"
                  " NotaryID: " << m.m_strNotaryID << "\n\n";
        //    "****New Account****:\n%s\n",

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyProcessNymboxResponse::reg(
    "processNymboxResponse", new StrategyProcessNymboxResponse());

class StrategyTriggerClause : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("nymboxHash", m.m_strNymboxHash.Get());
        pTag->add_attribute("smartContractID", formatLong(m.m_lTransactionNum));
        pTag->add_attribute("clauseName", m.m_strNymID2.Get());
        pTag->add_attribute("hasParam", formatBool(m.m_ascPayload.Exists()));

        if (m.m_ascPayload.Exists()) {
            pTag->add_tag("parameter", m.m_ascPayload.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNymboxHash = xml->getAttributeValue("nymboxHash");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strNymID2 = xml->getAttributeValue("clauseName");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        const String strHasParam = xml->getAttributeValue("hasParam");

        String strTransactionNum = xml->getAttributeValue("smartContractID");
        if (strTransactionNum.Exists())
            m.m_lTransactionNum = strTransactionNum.ToLong();

        if (strHasParam.Compare("true")) {
            const char* pElementExpected = "parameter";
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }
            else
                m.m_ascPayload = ascTextExpected;
        }

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nClause TransNum and Name:  " << m.m_lTransactionNum
               << "  /  " << m.m_strNymID2 << " \n"
                                              "Request #: " << m.m_strRequestNum
               << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClause::reg("triggerClause",
                                            new StrategyTriggerClause());

class StrategyTriggerClauseResponse : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        if (m.m_ascInReferenceTo.GetLength()) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        const char* pElementExpected = "inReferenceTo";

        OTASCIIArmor ascTextExpected;

        if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                  pElementExpected)) {
            otErr << "Error in OTMessage::ProcessXMLNode: "
                     "Expected " << pElementExpected
                  << " element with text field, for " << m.m_strCommand
                  << ".\n";
            return (-1); // error condition
        }

        m.m_ascInReferenceTo = ascTextExpected;

        otWarn << "\nCommand: " << m.m_strCommand << "   "
               << (m.m_bSuccess ? "SUCCESS" : "FAILED")
               << "\nNymID:    " << m.m_strNymID
               << "   NotaryID: " << m.m_strNotaryID << "\n\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyTriggerClauseResponse::reg(
    "triggerClauseResponse", new StrategyTriggerClauseResponse());

class StrategyGetMarketList : public OTMessageStrategy
{
public:
    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());

        parent.add_tag(pTag);
    }

    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");
        m.m_strRequestNum = xml->getAttributeValue("requestNum");

        otWarn << "\nCommand: " << m.m_strCommand
               << "\nNymID:    " << m.m_strNymID
               << "\nNotaryID: " << m.m_strNotaryID
               << "\nRequest #: " << m.m_strRequestNum << "\n";

        return 1;
    }
    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketList::reg("getMarketList",
                                            new StrategyGetMarketList());

class StrategyGetMarketListResponse : public OTMessageStrategy
{
public:
    virtual int32_t processXml(Message& m, irr::io::IrrXMLReader*& xml)
    {
        processXmlSuccess(m, xml);

        m.m_strCommand = xml->getNodeName(); // Command
        m.m_strRequestNum = xml->getAttributeValue("requestNum");
        m.m_strNymID = xml->getAttributeValue("nymID");
        m.m_strNotaryID = xml->getAttributeValue("notaryID");

        String strDepth = xml->getAttributeValue("depth");

        if (strDepth.GetLength() > 0) m.m_lDepth = strDepth.ToLong();

        const char* pElementExpected = nullptr;
        if (m.m_bSuccess && (m.m_lDepth > 0))
            pElementExpected = "messagePayload";
        else if (!m.m_bSuccess)
            pElementExpected = "inReferenceTo";

        if (nullptr != pElementExpected) {
            OTASCIIArmor ascTextExpected;

            if (!Contract::LoadEncodedTextFieldByName(xml, ascTextExpected,
                                                      pElementExpected)) {
                otErr << "Error in OTMessage::ProcessXMLNode: "
                         "Expected " << pElementExpected
                      << " element with text field, for " << m.m_strCommand
                      << ".\n";
                return (-1); // error condition
            }

            if (m.m_bSuccess)
                m.m_ascPayload.Set(ascTextExpected);
            else
                m.m_ascInReferenceTo.Set(ascTextExpected);
        }

        if (m.m_bSuccess)
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascPayload.Get()
        else
            otWarn << "\nCommand: " << m.m_strCommand << "   "
                   << (m.m_bSuccess ? "SUCCESS" : "FAILED")
                   << "\nNymID:    " << m.m_strNymID
                   << "\n NotaryID: " << m.m_strNotaryID
                   << "\n\n"; // m_ascInReferenceTo.Get()

        return 1;
    }

    virtual void writeXml(Message& m, Tag& parent)
    {
        TagPtr pTag(new Tag(m.m_strCommand.Get()));

        pTag->add_attribute("success", formatBool(m.m_bSuccess));
        pTag->add_attribute("requestNum", m.m_strRequestNum.Get());
        pTag->add_attribute("nymID", m.m_strNymID.Get());
        pTag->add_attribute("notaryID", m.m_strNotaryID.Get());
        pTag->add_attribute("depth", formatLong(m.m_lDepth));

        if (m.m_bSuccess && (m.m_ascPayload.GetLength() > 2) &&
            (m.m_lDepth > 0)) {
            pTag->add_tag("messagePayload", m.m_ascPayload.Get());
        }
        else if (!m.m_bSuccess && (m.m_ascInReferenceTo.GetLength() > 2)) {
            pTag->add_tag("inReferenceTo", m.m_ascInReferenceTo.Get());
        }

        parent.add_tag(pTag);
    }

    static RegisterStrategy reg;
};
RegisterStrategy StrategyGetMarketListResponse::reg(
    "getMarketListResponse", new StrategyGetMarketListResponse());

} // namespace opentxs
