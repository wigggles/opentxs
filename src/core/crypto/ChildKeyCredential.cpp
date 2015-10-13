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

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/crypto/ChildKeyCredential.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/util/Tag.hpp>
#include <opentxs/core/Log.hpp>

#include "irrxml/irrXML.hpp"

// return -1 if error, 0 if nothing, and 1 if the node was processed.

namespace opentxs
{

ChildKeyCredential::ChildKeyCredential(CredentialSet& other)
    : ot_super(other)
{
    m_strContractType = "KEY CREDENTIAL";
}

ChildKeyCredential::ChildKeyCredential(CredentialSet& other, const Credential::CredentialType childType)
    : ot_super(other, childType)
{
    m_strContractType = "KEY CREDENTIAL";
}

ChildKeyCredential::ChildKeyCredential(CredentialSet& other, const std::shared_ptr<NymParameters>& nymParameters)
    : ot_super(other, nymParameters)
{
    m_strContractType = "KEY CREDENTIAL";
}

ChildKeyCredential::~ChildKeyCredential()
{
}

int32_t ChildKeyCredential::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t retval = ot_super::ProcessXMLNode(xml);

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

        String KeyCredentialType = xml->getAttributeValue("type");

        Credential::CredentialType actualCredentialType = Credential::ERROR_TYPE;

        if (KeyCredentialType.Exists()) {
            actualCredentialType = StringToCredentialType(KeyCredentialType.Get());
        } else {
            actualCredentialType = Credential::RSA_PUBKEY; //backward compatibility
        }

        OT_ASSERT(!m_AuthentKey);
        OT_ASSERT(!m_SigningKey);
        OT_ASSERT(!m_EncryptKey);

        if (Credential::ERROR_TYPE == actualCredentialType) {
            otErr << "Invalid child key credential type.\n";
            return -1;
        } else {
            m_Type = actualCredentialType;
            OTAsymmetricKey::KeyType actualKeyType = Credential::CredentialTypeToKeyType(actualCredentialType);
            m_AuthentKey = std::make_shared<OTKeypair>(actualKeyType);
            m_SigningKey = std::make_shared<OTKeypair>(actualKeyType);
            m_EncryptKey = std::make_shared<OTKeypair>(actualKeyType);
        }
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

void ChildKeyCredential::UpdateContents()
{
    m_xmlUnsigned.Release();

    Tag tag("keyCredential");

    // a hash of the nymIDSource
    tag.add_attribute("nymID", GetNymID().Get());
    // Hash of the master credential that signed this credential.
    tag.add_attribute("masterID", GetMasterCredID().Get());

    String KeyCredentialType = CredentialTypeToString(this->GetType());

    if (KeyCredentialType.Exists()) {
        tag.add_attribute("type", KeyCredentialType.Get());
    } else {
        tag.add_attribute("type", CredentialTypeToString(Credential::RSA_PUBKEY).Get()); //backward compatibility
    }

    if (GetNymIDSource().Exists()) {
        OTASCIIArmor ascSource;
        // A nym should always verify through its own
        // source. (Whatever that may be.)
        ascSource.SetString(GetNymIDSource());
        tag.add_tag("nymIDSource", ascSource.Get());
    }
    // MASTER-SIGNED INFO
    if (Credential::credMasterSigned == m_StoreAs ||
        Credential::credPrivateInfo == m_StoreAs) {
        UpdatePublicContentsToTag(tag);
    }
    // PUBLIC INFO (signed by child key credential, contains master signed info.)
    if (Credential::credPublicInfo == m_StoreAs ||
        Credential::credPrivateInfo == m_StoreAs) {
        // GetMasterSigned() returns the contract
        // containing the master-signed contents
        // from the above block.
        OTASCIIArmor ascMasterSigned(GetMasterSigned());

        // Contains all the public info, signed by the master key.
        // Packaged up here inside a final, child key credential-signed credential.
        tag.add_tag("masterSigned", ascMasterSigned.Get());
    }
    // PRIVATE INFO
    //
    // If we're saving the private credential info...
    if (Credential::credPrivateInfo == m_StoreAs) {
        UpdatePublicCredentialToTag(tag);
        UpdatePrivateContentsToTag(tag);
    }

    // <=== SET IT BACK TO DEFAULT BEHAVIOR. Any other state
    // processes ONCE, and then goes back to this again.
    m_StoreAs = Credential::credPrivateInfo;

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

bool ChildKeyCredential::VerifySignedByMaster()
{
    // See if m_strMasterSigned was signed by my master credential.
    ChildKeyCredential masterCredential(*m_pOwner);

    if (m_strMasterSigned.Exists() &&
        masterCredential.LoadContractFromString(m_strMasterSigned)) {
        // Here we need to MAKE SURE that the "master signed" version contains
        // the same CONTENTS as the actual version.
        if (!GetNymID().Compare(masterCredential.GetNymID())) {
            Log::vOutput(0, "%s: Failure, NymID of this key credential "
                            "doesn't match NymID of master-signed version of "
                            "this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (!GetNymIDSource().Compare(masterCredential.GetNymIDSource())) {
            Log::vOutput(0, "%s: Failure, NymIDSource of this key credential "
                            "doesn't match NymIDSource of master-signed "
                            "version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (!GetMasterCredID().Compare(masterCredential.GetMasterCredID())) {
            Log::vOutput(0, "%s: Failure, MasterCredID of this key "
                            "credential doesn't match MasterCredID of "
                            "master-signed version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        if (GetPublicMap().size() > 0 &&
            GetPublicMap() != masterCredential.GetPublicMap()) {
            Log::vOutput(0, "%s: Failure, public info of this key credential "
                            "doesn't match public info of master-signed "
                            "version of this key credential.\n",
                         __FUNCTION__);
            return false;
        }

        // Master-signed version of child key credential does not contain the private keys,
        // since normally the master is signing
        // the public version of the sub credential (to validate it) and you
        // don't want the public seeing your private keys.
        // So we would never expect these to match, since the master signed
        // version should have no private keys in it.
        //
        //        if (GetPrivateMap() != masterCredential.GetPrivateMap())
        //        {
        //            OTLog::vOutput(0, "%s: Failure, private info of this key
        // credential doesn't match private info of master-signed version of
        // this key credential.\n", __FUNCTION__);
        //            return false;
        //        }

        OT_ASSERT(m_pOwner->GetMasterCredential().m_SigningKey);
        bool verifiedWithKey = masterCredential.VerifyWithKey(
            m_pOwner->GetMasterCredential().m_SigningKey->GetPublicKey());

        // ON SERVER SIDE, THE ACTUAL CHILD KEY CREDENTIAL doesn't have any public key, only
        // the master-signed version of it.
        // (The master-signed version being basically the only contents of the
        // public version.)
        // So we need to be able to, after verifying, load up those contents so
        // they are available on the
        // child key credential itself, and not just on some master-signed version of itself
        // hidden inside itself.
        // Otherwise I would have to load up the master-signed version anytime
        // the server-side wanted to
        // mess with any of the keys.
        // Thus: copy the public info from master signed, to* this, if the above
        // call was successful
        if (verifiedWithKey && GetPublicMap().size() == 0) {
            // For master credential.
            return SetPublicContents(masterCredential.GetPublicMap());
        }
        return verifiedWithKey;
    }
    return false;
}

} // namespace opentxs
