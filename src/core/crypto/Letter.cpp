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

#include <opentxs/core/crypto/Letter.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/util/Tag.hpp>

#include <cstring>
#include <irrxml/irrXML.hpp>

namespace opentxs
{

void Letter::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag rootNode("letter");

    rootNode.add_attribute("ephemeralkey", ephemeralKey_.Get());
    rootNode.add_attribute("mac", macType_.Get());
    rootNode.add_attribute("nonce", nonce_.Get());
    rootNode.add_attribute("sessionkey", sessionKey_.Get());
    rootNode.add_tag("ciphertext", ciphertext_.Get());

    std::string str_result;
    rootNode.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

int32_t Letter::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = 0;

    const String strNodeName(xml->getNodeName());

    if (strNodeName.Compare("letter")) {
        ephemeralKey_ = xml->getAttributeValue("ephemeralkey");
        macType_ = xml->getAttributeValue("mac");
        nonce_ = xml->getAttributeValue("nonce");
        sessionKey_ = xml->getAttributeValue("sessionkey");
        nReturnVal = 1;
    }
    else if (strNodeName.Compare("ciphertext")) {
        if (false ==
            Contract::LoadEncodedTextField(xml, ciphertext_)) {
            otErr << "Error in Letter::ProcessXMLNode: no ciphertext.\n";
            return (-1); // error condition
        }
        return 1;
    }

    return nReturnVal;
}

Letter::Letter(
        const String& ephemeralKey,
        const String& macType,
        const String& nonce,
        const String& sessionKey,
        const OTASCIIArmor& ciphertext)
    : Contract()
    , ephemeralKey_(ephemeralKey)
    , macType_(macType)
    , nonce_(nonce)
    , sessionKey_(sessionKey)
    , ciphertext_(ciphertext)

{
    m_strContractType.Set("LETTER");
}

Letter::Letter(
        const String& input)
    : Contract()
{
    m_strContractType.Set("LETTER");
    m_xmlUnsigned = input;
    LoadContractXML();
}

Letter::~Letter()
{
    Release_Letter();
}

void Letter::Release_Letter()
{
}

void Letter::Release()
{
    Release_Letter();

    ot_super::Release();

    m_strContractType.Set("LETTER");
}

const String& Letter::EphemeralKey() const
{
    return ephemeralKey_;
}

const String& Letter::Nonce() const
{
    return nonce_;
}

const String& Letter::MACType() const
{
    return macType_;
}

const String& Letter::SessionKey() const
{
    return sessionKey_;
}

const OTASCIIArmor& Letter::Ciphertext() const
{
   return ciphertext_;
}

} // namespace opentxs
