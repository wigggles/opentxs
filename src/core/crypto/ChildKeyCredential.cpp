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
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Log.hpp>

// return -1 if error, 0 if nothing, and 1 if the node was processed.

namespace opentxs
{

ChildKeyCredential::ChildKeyCredential(CredentialSet& other, const String& stringCred)
    : ChildKeyCredential(other, *Credential::ExtractArmoredCredential(stringCred))
{
    m_strContractType = "KEY CREDENTIAL";
    m_Role = proto::CREDROLE_CHILDKEY;
}

ChildKeyCredential::ChildKeyCredential(CredentialSet& other, const proto::Credential& serializedCred)
    : ot_super(other, serializedCred)
{
    m_strContractType = "KEY CREDENTIAL";
    m_Role = proto::CREDROLE_CHILDKEY;

    SetMasterCredID(serializedCred.publiccredential().childdata().masterid());
}

ChildKeyCredential::ChildKeyCredential(CredentialSet& other, const NymParameters& nymParameters)
    : ot_super(other, nymParameters)
{
    m_strContractType = "KEY CREDENTIAL";
    m_Role = proto::CREDROLE_CHILDKEY;

    m_strNymID = other.GetNymID();
    SetMasterCredID(other.GetMasterCredID());

    Identifier childID;
    CalculateAndSetContractID(childID);

    OT_ASSERT(SelfSign());
    OT_ASSERT(AddMasterSignature());

    OT_ASSERT(VerifySignedBySelf());
    OT_ASSERT(VerifySignedByMaster());

    String credID(childID);

    String strFoldername, strFilename;
    strFoldername.Format("%s%s%s", OTFolders::Credential().Get(),
                         Log::PathSeparator(), other.GetNymID().Get());
    strFilename.Format("%s", credID.Get());

    SaveContract(strFoldername.Get(), strFilename.Get());
}

ChildKeyCredential::~ChildKeyCredential()
{
}

bool ChildKeyCredential::VerifySignedByMaster()
{
    OT_ASSERT(m_pOwner->GetMasterCredential().m_SigningKey);

    serializedSignature masterSig = GetMasterSignature();

    if (!masterSig) {
        otErr << __FUNCTION__ << ": Could not find master signature.\n";
        return false;
    }

    bool goodSig = VerifySig(*masterSig, m_pOwner->GetMasterCredential().m_SigningKey->GetPublicKey(), false);

    if (!goodSig) {
        otErr << __FUNCTION__ << ": Could not verify master signature.\n";
        return false;
    }

    return true;
}

bool ChildKeyCredential::AddMasterSignature()
{
    if (nullptr == m_pOwner) {
        otErr << __FUNCTION__ << ": Missing master credential.\n";
        return false;
    }

    String credID = m_pOwner->GetMasterCredID();
    OTData masterSignature;
    serializedSignature serializedMasterSignature;
    serializedMasterSignature.reset(new proto::Signature);

    serializedCredential publicVersion = SerializeForPublicSignature();
    bool havePublicSig = m_pOwner->GetMasterCredential().Sign(
        *publicVersion,
        Identifier::DefaultHashAlgorithm,
        masterSignature);

    if (!havePublicSig) {
        otErr << __FUNCTION__ << ": Failed to obtain signature from master credential.\n";
        return false;
    }

    serializedMasterSignature->set_version(1);
    serializedMasterSignature->set_credentialid(credID.Get());
    serializedMasterSignature->set_role(proto::SIGROLE_PUBCREDENTIAL);
    serializedMasterSignature->set_hashtype(static_cast<proto::HashType>(Identifier::DefaultHashAlgorithm));
    serializedMasterSignature->set_signature(masterSignature.GetPointer(), masterSignature.GetSize());

    m_listSerializedSignatures.push_back(serializedMasterSignature);

    OT_ASSERT(m_listSerializedSignatures.size()==3);

    return true;
}

serializedSignature ChildKeyCredential::GetMasterSignature() const
{
    serializedSignature masterSignature;
    proto::SignatureRole targetRole = proto::SIGROLE_PUBCREDENTIAL;
    for (auto& it : m_listSerializedSignatures) {

        if ((it->role() == targetRole) &&
            (it->credentialid() == GetMasterCredID().Get())) {

            masterSignature = it;
            break;
        }
    }

    return masterSignature;
}

serializedCredential ChildKeyCredential::Serialize(bool asPrivate, bool asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::Serialize(asPrivate, asSigned);

    proto::ChildCredentialParameters* parameters = new proto::ChildCredentialParameters;

    parameters->set_version(1);
    parameters->set_masterid(GetMasterCredID().Get());

    // Only the public credential gets ChildCredentialParameters
    if (serializedCredential->has_publiccredential()) {
        proto::KeyCredential* keyCredential = serializedCredential->mutable_publiccredential();
        keyCredential->set_allocated_childdata(parameters);

    }

    if (asSigned) {
        serializedSignature masterSignature = GetMasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature = serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            otErr << __FUNCTION__ << ": Failed to get master signature.\n";
        }
    }

    serializedCredential->set_role(proto::CREDROLE_CHILDKEY);

    return serializedCredential;
}

} // namespace opentxs
