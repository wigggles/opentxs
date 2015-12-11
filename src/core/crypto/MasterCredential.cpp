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

#include <opentxs/core/crypto/MasterCredential.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/crypto/Credential.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/Log.hpp>

namespace opentxs
{

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID. Also
// verify that
// *this == m_pOwner->GetMasterCredential() (the master credential.) Verify the
// (self-signed)
// signature on *this.
//
bool MasterCredential::VerifyInternally()
{
    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    // We can't use super here, since Credential::VerifyInternally will
    // verify
    // m_strMasterCredID against the actual Master, which is not relevant to us
    // in
    // MasterCredential. But this means if we need anything else that
    // KeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    //  if (!ot_super::VerifyInternally())
    //      return false;
    if (!VerifyNymID()) return false;

    OT_ASSERT(nullptr != m_pOwner);
    // Verify that *this == m_pOwner->GetMasterCredential() (the master credential.)
    //
    if (this != &(m_pOwner->GetMasterCredential())) {
        otOut << __FUNCTION__ << ": Failure: Expected *this object to be the "
                                 "same as m_pOwner->GetMasterCredential(), "
                                 "but it wasn't.\n";
        return false;
    }

    // Remember this note above: ...if we need anything else that
    // KeyCredential::VerifyInternally
    // was doing, we will have to duplicate that here as well...
    // Since we aren't calling KeyCredential::VerifyInternally (the super) and
    // since that function
    // verifies that the credential is self-signed, we must do the same
    // verification here:
    //
    // Any KeyCredential (both master and child, but no other credentials)
    // must ** sign itself.**
    //
    if (!VerifySignedBySelf()) {
        otOut << __FUNCTION__ << ": Failed verifying master credential: it's "
                                 "not signed by itself (its own signing "
                                 "key.)\n";
        return false;
    }

    return true;
}

bool MasterCredential::VerifyAgainstSource() const
{
    return m_pOwner->Source().Verify(*this);
}

MasterCredential::MasterCredential(CredentialSet& theOwner, const String&
stringCred)
    : MasterCredential(theOwner, *Credential::ExtractArmoredCredential(stringCred))
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;

}

MasterCredential::MasterCredential(CredentialSet& theOwner, const proto::Credential& serializedCred)
: ot_super(theOwner, serializedCred)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source = std::make_shared<NymIDSource>(
serializedCred.publiccredential().masterdata().source());

    m_pOwner->SetSource(source);
    source_proof_.reset(new proto::SourceProof(serializedCred.publiccredential().masterdata().sourceproof()));
}

MasterCredential::MasterCredential(CredentialSet& theOwner, const NymParameters& nymParameters)
    : ot_super(theOwner, nymParameters, proto::CREDROLE_MASTERKEY)
{
    m_strContractType = "MASTER KEY CREDENTIAL";
    m_Role = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source;

    std::unique_ptr<proto::SourceProof> sourceProof;
    sourceProof.reset(new proto::SourceProof);

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
      nymParameters.SourceProofType())  {
        source = std::make_shared<NymIDSource>(
            nymParameters,
            *(m_SigningKey->GetPublicKey().Serialize()));
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    }
    else {
        // FIXME this master credential will be invalid.
        // VerifyInternally() will catch the error.
        // Need to implement non-self signed credentials for real though.
    }

    source_proof_.reset(sourceProof.release());
    m_pOwner->SetSource(source);
    String nymID = m_pOwner->GetNymID();

    m_strNymID = nymID;

    Identifier masterID;
    CalculateAndSetContractID(masterID);

    SelfSign();

    String credID(masterID), nym(nymID);

    String strFoldername, strFilename;
    strFoldername.Format("%s%s%s", OTFolders::Credential().Get(),
                         Log::PathSeparator(), nym.Get());
    strFilename.Format("%s", credID.Get());

    SaveContract(strFoldername.Get(), strFilename.Get());
}

MasterCredential::~MasterCredential()
{
}

serializedCredential MasterCredential::Serialize(bool asPrivate, bool asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::Serialize(asPrivate, asSigned);

    proto::MasterCredentialParameters* parameters = new proto::MasterCredentialParameters;

    parameters->set_version(1);
    *(parameters->mutable_source()) = *(m_pOwner->Source().Serialize());

    // Only the public credential gets MasterCredentialParameters
    if (serializedCredential->has_publiccredential()) {
        proto::KeyCredential* keyCredential = serializedCredential->mutable_publiccredential();
        keyCredential->set_allocated_masterdata(parameters);

    }

    serializedCredential->set_role(proto::CREDROLE_MASTERKEY);
    *(serializedCredential->mutable_publiccredential()->mutable_masterdata()->mutable_sourceproof())=*source_proof_;

    return serializedCredential;
}

} // namespace opentxs
