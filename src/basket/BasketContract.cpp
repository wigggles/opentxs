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

#include <opentxs/basket/BasketContract.hpp>
#include <opentxs/basket/Basket.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/Contract.hpp>
#include <opentxs/core/Nym.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/util/Tag.hpp>

#include <irrxml/irrXML.hpp>

namespace opentxs
{

// Normally, Asset Contracts do NOT update / rewrite their contents, since their
// primary goal is for the signature to continue to verify.  But when first
// creating a basket contract, we have to rewrite the contents, which is done
// here.
BasketContract::BasketContract(Basket& theBasket, Nym& theSigner)
    : AssetContract()
{
    // Grab a string copy of the basket information.
    theBasket.SaveContractRaw(m_strBasketInfo);

    String strTemplate;

    Tag tag("basketContract");
    tag.add_attribute("version", m_strVersion.Get());

    OTASCIIArmor theBasketArmor(m_strBasketInfo);
    tag.add_tag("basketInfo", theBasketArmor.Get());

    std::string str_result;
    tag.output(str_result);

    strTemplate.Format("%s", str_result.c_str());

    CreateContract(strTemplate, theSigner);
}

BasketContract::~BasketContract()
{
}

void BasketContract::CreateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("basketContract");
    tag.add_attribute("version", m_strVersion.Get());

    OTASCIIArmor theBasketArmor(m_strBasketInfo);
    tag.add_tag("basketInfo", theBasketArmor.Get());

    // This is where OTContract scribes tag with its keys,
    // conditions, etc.
    CreateInnerContents(tag);

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Format("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t BasketContract::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = Contract::ProcessXMLNode(xml);

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.

    if (nReturnVal == 1 || nReturnVal == (-1)) return nReturnVal;

    String strNodeName(xml->getNodeName());

    if (strNodeName.Compare("basketContract")) {
        m_strVersion = xml->getAttributeValue("version");

        otWarn << "\n"
                  "===> Loading XML portion of basket contract into memory "
                  "structures...\n\n"
                  "Digital Basket Contract: " << m_strName
               << "\nContract version: " << m_strVersion << "\n----------\n";
        nReturnVal = 1;
    }
    else if (strNodeName.Compare("basketInfo")) {
        if (!Contract::LoadEncodedTextField(xml, m_strBasketInfo)) {
            otErr << "Error in OTAssetContract::ProcessXMLNode: basketInfo "
                     "field without value.\n";
            return (-1); // error condition
        }
        nReturnVal = 1;
    }

    return nReturnVal;
}

} // namespace opentxs
