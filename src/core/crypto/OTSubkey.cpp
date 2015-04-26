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

// A nym contains a list of credentials
//
// Each credential contains a "master" subkey, and a list of subkeys
// signed by that master.
//
// The same class (subkey) is used because there are master credentials
// and subkey credentials, so we're using a single "subkey" class to
// encapsulate each credential, both for the master credential and
// for each subkey credential.
//
// Each subkey has 3 key pairs: encryption, signing, and authentication.
//
// Each key pair has 2 OTAsymmetricKeys (public and private.)

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/OTSubkey.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/OTCredential.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Log.hpp>

#include "irrxml/irrXML.hpp"

// return -1 if error, 0 if nothing, and 1 if the node was processed.

namespace opentxs
{

OTSubkey::OTSubkey()
    : OTKeyCredential()
{
    m_strContractType = "KEY CREDENTIAL";
}

OTSubkey::OTSubkey(OTCredential& other)
    : OTKeyCredential(other)
{
    m_strContractType = "KEY CREDENTIAL";
}

OTSubkey::~OTSubkey()
{
}

int32_t OTSubkey::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t retval = OTKeyCredential::ProcessXMLNode(xml);

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do in the case of OTAccount.
    if (retval != 0) return retval;

    String nodeName(xml->getNodeName());
    if (nodeName.Compare("keyCredential")) {
        m_strNymID = xml->getAttributeValue("nymID");
        m_strMasterCredID = xml->getAttributeValue("masterID");

        Log::Output(1, "Loading keyCredential...\n");
        retval = 1;
    }
    else if (nodeName.Compare("masterSigned")) {
        if (!Contract::LoadEncodedTextField(xml, m_strMasterSigned)) {
            Log::vError("Error in %s line %d: failed loading expected "
                        "master-signed version while loading "
                        "keyCredential.\n",
                        __FILE__, __LINE__);
            return -1;
        }
        retval = 1;
    }
    return retval;
}

void OTSubkey::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("keyCredential");

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());
    // Hash of the master credential that signed this subcredential.
    tag.add_attribute("masterID", GetMasterCredID().Get());

    if (GetNymIDSource().Exists()) {
        OTASCIIArmor ascSource;
        // A nym should always verify through its own
        // source. (Whatever that may be.)
        ascSource.SetString(GetNymIDSource());
        tag.add_tag("nymIDSource", ascSource.Get());
    }
    // MASTER-SIGNED INFO
    if (OTSubcredential::credMasterSigned == m_StoreAs ||
        OTSubcredential::credPrivateInfo == m_StoreAs) {
        UpdatePublicContentsToTag(tag);
    }
    // PUBLIC INFO (signed by subkey, contains master signed info.)
    if (OTSubcredential::credPublicInfo == m_StoreAs ||
        OTSubcredential::credPrivateInfo == m_StoreAs) {
        // GetMasterSigned() returns the contract
        // containing the master-signed contents
        // from the above block.
        OTASCIIArmor ascMasterSigned(GetMasterSigned());

        // Contains all the public info, signed by the master key.
        // Packaged up here inside a final, subkey-signed credential.
        tag.add_tag("masterSigned", ascMasterSigned.Get());
    }
    // PRIVATE INFO
    //
    // If we're saving the private credential info...
    if (OTSubcredential::credPrivateInfo == m_StoreAs) {
        UpdatePublicCredentialToTag(tag);
        UpdatePrivateContentsToTag(tag);
    }

    // <=== SET IT BACK TO DEFAULT BEHAVIOR. Any other state
    // processes ONCE, and then goes back to this again.
    m_StoreAs = OTSubcredential::credPrivateInfo;

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

bool OTSubkey::VerifySignedByMaster()
{
    // See if m_strMasterSigned was signed by my master credential.
    OTSubkey masterKey(*m_pOwner);

    if (m_strMasterSigned.Exists() &&
        masterKey.LoadContractFromString(m_strMasterSigned)) {
        // Here we need to MAKE SURE that the "master signed" version contains
        // the same CONTENTS as the actual version.
        if (!GetNymID().Compare(masterKey.GetNymID())) {
            Log::vOutput(0, "%s: Failure, NymID of this key credential "
                            "doesn't match NymID of master-signed version of "
                            "this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (!GetNymIDSource().Compare(masterKey.GetNymIDSource())) {
            Log::vOutput(0, "%s: Failure, NymIDSource of this key credential "
                            "doesn't match NymIDSource of master-signed "
                            "version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (!GetMasterCredID().Compare(masterKey.GetMasterCredID())) {
            Log::vOutput(0, "%s: Failure, MasterCredID of this key "
                            "credential doesn't match MasterCredID of "
                            "master-signed version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (GetPublicMap().size() > 0 &&
            GetPublicMap() != masterKey.GetPublicMap()) {
            Log::vOutput(0, "%s: Failure, public info of this key credential "
                            "doesn't match public info of master-signed "
                            "version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        // Master-signed version of subkey does not contain the private keys,
        // since normally the master is signing
        // the public version of the sub credential (to validate it) and you
        // don't want the public seeing your private keys.
        // So we would never expect these to match, since the master signed
        // version should have no private keys in it.
        //
        //        if (GetPrivateMap() != masterKey.GetPrivateMap())
        //        {
        //            OTLog::vOutput(0, "%s: Failure, private info of this key
        // credential doesn't match private info of master-signed version of
        // this key credential.\n", __FUNCTION__);
        //            return false;
        //        }

        bool verifiedWithKey = masterKey.VerifyWithKey(
            m_pOwner->GetMasterkey().m_SigningKey.GetPublicKey());

        // ON SERVER SIDE, THE ACTUAL SUBKEY doesn't have any public key, only
        // the master-signed version of it.
        // (The master-signed version being basically the only contents of the
        // public version.)
        // So we need to be able to, after verifying, load up those contents so
        // they are available on the
        // subkey itself, and not just on some master-signed version of itself
        // hidden inside itself.
        // Otherwise I would have to load up the master-signed version anytime
        // the server-side wanted to
        // mess with any of the keys.
        // Thus: copy the public info from master signed, to* this, if the above
        // call was successful
        if (verifiedWithKey && GetPublicMap().size() == 0) {
            // For master credential.
            return SetPublicContents(masterKey.GetPublicMap());
        }
        return verifiedWithKey;
    }
    return false;
}

} // namespace opentxs
