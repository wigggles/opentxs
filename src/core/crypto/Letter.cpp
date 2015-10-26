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
#include <opentxs/core/crypto/OTEnvelope.hpp>

namespace opentxs
{

void Letter::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag rootNode("letter");

    rootNode.add_attribute("ephemeralkey", ephemeralKey_.Get());
    rootNode.add_attribute("mac", macType_.Get());
    rootNode.add_attribute("iv", iv_.Get());

    if (!sessionKeys_.empty()) {
        for (auto& it : sessionKeys_) {
            TagPtr sessionKeyNode = std::make_shared<Tag>("sessionkey");
            OTASCIIArmor sessionKey;

            it.second.GetAsciiArmoredData(sessionKey);

            sessionKeyNode->add_attribute("nonce", it.first.Get());
            sessionKeyNode->set_text(sessionKey.Get());

            rootNode.add_tag(sessionKeyNode);
        }
    }

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
        iv_ = xml->getAttributeValue("iv");
        nReturnVal = 1;
    } else if (strNodeName.Compare("ciphertext")) {
        if (false ==
            Contract::LoadEncodedTextField(xml, ciphertext_)) {
            otErr << "Error in Letter::ProcessXMLNode: no ciphertext.\n";
            return (-1); // error condition
        }
        nReturnVal = 1;
    } else if (strNodeName.Compare("sessionkey")) {
        String nonce;
        OTASCIIArmor armoredText;

        nonce = xml->getAttributeValue("nonce");

        if (false == Contract::LoadEncodedTextField(xml, armoredText)) {
            otErr << "Error in Letter::ProcessXMLNode: no ciphertext.\n";

            return (-1); // error condition
        } else {
            OTEnvelope sessionKey(armoredText);
            sessionKeys_.insert(std::pair<String, OTEnvelope>(nonce, sessionKey));

            nReturnVal = 1;
        }
    }

    return nReturnVal;
}

Letter::Letter(
        const String& ephemeralKey,
        const String& macType,
        const String& iv,
        const OTASCIIArmor& ciphertext,
        const mapOfSessionKeys& sessionKeys)
    : Contract()
    , ephemeralKey_(ephemeralKey)
    , macType_(macType)
    , iv_(iv)
    , ciphertext_(ciphertext)
    , sessionKeys_(sessionKeys)

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

const String& Letter::IV() const
{
    return iv_;
}

const String& Letter::MACType() const
{
    return macType_;
}

const mapOfSessionKeys& Letter::SessionKeys() const
{
    return sessionKeys_;
}

const OTASCIIArmor& Letter::Ciphertext() const
{
   return ciphertext_;
}

} // namespace opentxs
