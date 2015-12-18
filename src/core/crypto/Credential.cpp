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

#include <memory>

#include <opentxs/core/crypto/Credential.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/stdafx.hpp>
#include <opentxs/core/crypto/ChildKeyCredential.hpp>
#include <opentxs/core/crypto/ContactCredential.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/crypto/MasterCredential.hpp>
#include <opentxs/core/crypto/OTASCIIArmor.hpp>

#include <map>

// Contains 3 key pairs: signing, authentication, and encryption.
// This is stored as an Contract, and it must be signed by the
// master key. (which is also an Credential.)
//

namespace opentxs
{

Credential* Credential::CredentialFactory(
        CredentialSet& parent,
        const proto::Credential& serialized,
        const proto::CredentialRole& purportedRole)
{
    Credential* result = nullptr;

    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Verify(serialized, purportedRole)) {
        otErr << __FUNCTION__ << ": Invalid serialized credential.\n";
        return result;
    }

    switch (serialized.role()) {
        case proto::CREDROLE_MASTERKEY :
            result = new MasterCredential(parent, serialized);

            return result;
        case proto::CREDROLE_CHILDKEY :
            result = new ChildKeyCredential(parent, serialized);

            return result;
        case proto::CREDROLE_CONTACT :
            result = new ContactCredential(parent, serialized);

            return result;
        default :
            break;
    }

    return result;
}

Credential::Credential(CredentialSet& theOwner, const NymParameters& nymParameters)
    : Contract()
    , type_(nymParameters.credentialType())
    , mode_(proto::KEYMODE_PRIVATE)
    , owner_backlink_(&theOwner)
    , version_(1)
{
    m_strContractType = "CREDENTIAL";
}

Credential::Credential(CredentialSet& theOwner, const proto::Credential& serializedCred)
    : Contract()
    , type_(serializedCred.type())
    , role_(serializedCred.role())
    , mode_(serializedCred.mode())
    , owner_backlink_(&theOwner)
    , version_(serializedCred.version())
    {
    m_strContractType = "CREDENTIAL";

    if (serializedCred.has_nymid()) {
        nym_id_ = serializedCred.nymid();
        SetIdentifier(serializedCred.id());
    }

    serializedSignature sig;
    for (auto& it : serializedCred.signature()) {
        sig.reset(new proto::Signature(it));
        m_listSerializedSignatures.push_back(sig);
    }
}

Credential::~Credential()
{
    Release_Credential();
}

// virtual
void Credential::Release()
{
    Release_Credential(); // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    Contract::Release(); // since I've overridden the base class, I call it
                         // now...
}

void Credential::Release_Credential()
{
    // Release any dynamically allocated members here. (Normally.)
}

// VERIFICATION

// Verify that nym_id_ matches the nymID of the parent credential set
//
bool Credential::VerifyNymID() const
{

    return (nym_id_ == owner_backlink_->GetNymID());
}

// Verify that master_id_ matches the MasterID of the parent credential set
//
bool Credential::VerifyMasterID() const
{
    if (proto::CREDROLE_MASTERKEY == role_) {
        return (m_ID == owner_backlink_->GetMasterCredID());
    } else {
        return (master_id_ == owner_backlink_->GetMasterCredID());
    }
}

// Verify this credential produces its purported ID
bool Credential::VerifyCredentialID() const
{
    Identifier realID;
    CalculateContractID(realID);

    return (m_ID == realID);
}

// Verifies the cryptographic integrity of a credential.
// Assumes the CredentialSet specified by owner_backlink_ is valid.
bool Credential::VerifyInternally() const
{
    OT_ASSERT(nullptr != owner_backlink_);

    if (nullptr == owner_backlink_) {
        otErr << __FUNCTION__ << ": This credential is not attached"
              << " to a CredentialSet. Can not verify.\n";

        return false;
    }

    if (!VerifyNymID()) {
        otErr << __FUNCTION__ << ": NymID for this credential does not match"
              << "NymID of parent CredentialSet.\n";

        return false;
    }

    if (!VerifyMasterID()) {
        otErr << __FUNCTION__ << ": MasterID for this credential does not match"
              << "MasterID of parent CredentialSet.\n";

        return false;
    }

    if (!VerifyCredentialID()) {
        otErr << __FUNCTION__ << ": Purported ID for this credential does not"
              << " match its actual contents.\n";

        return false;
    }

    bool GoodMasterSignature = false;

    if (proto::CREDROLE_MASTERKEY == role_) {
        GoodMasterSignature = true; // Covered by VerifySignedBySelf()
    } else {
        GoodMasterSignature = VerifySignedByMaster();
    }

    if (!GoodMasterSignature) {
        otErr << __FUNCTION__ << ": Failure: This credential hasn't been "
                                 "signed by its master credential.\n";
        return false;
    }

    return true;
}

bool Credential::VerifySignedByMaster() const
{
    OT_ASSERT(owner_backlink_);

    return (owner_backlink_->GetMasterCredential().Verify(*this));
}

serializedSignature Credential::MasterSignature() const
{
    serializedSignature masterSignature;
    proto::SignatureRole targetRole = proto::SIGROLE_PUBCREDENTIAL;
    for (auto& it : m_listSerializedSignatures) {

        if ((it->role() == targetRole) &&
            (it->credentialid() == MasterID().Get())) {

            masterSignature = it;
            break;
        }
    }

    return masterSignature;
}

// Perform syntax (non-cryptographic) verifications of a credential
bool Credential::isValid() const
{
    serializedCredential serializedProto;

    return isValid(serializedProto);
}

// Returns the serialized form to prevent unnecessary serializations
bool Credential::isValid(serializedCredential& credential) const
{
    SerializationModeFlag serializationMode = AS_PUBLIC;

    if (proto::KEYMODE_PRIVATE == mode_) {
        serializationMode = AS_PRIVATE;
    }

    credential = asSerialized(serializationMode, WITH_SIGNATURES);

    return proto::Verify(*credential, role_, WITH_SIGNATURES);
}

// Overrides opentxs::Contract()
bool Credential::VerifyContract() const
{
    // Check syntax
    if (!isValid()) {
        return false;
    }

    // Check cryptographic requirements
    return VerifyInternally();
}

// Overriding from Contract.
void Credential::CalculateContractID(Identifier& newID) const
{
    serializedCredential idVersion = asSerialized(
        Credential::AS_PUBLIC,
        Credential::WITHOUT_SIGNATURES);

    if (idVersion->has_id()) {
        idVersion->clear_id();
    }

    OTData serializedData = proto::ProtoAsData<proto::Credential>(*idVersion);

    if (!newID.CalculateDigest(serializedData))
        otErr << __FUNCTION__ << ": Error calculating credential digest.\n";
}

String Credential::CredentialTypeToString(proto::CredentialType credentialType)
{
    String credentialString;

    switch (credentialType) {
        case proto::CREDTYPE_LEGACY :
            credentialString="Legacy";
            break;
        case proto::CREDTYPE_HD :
            credentialString="HD";
            break;
        default :
            credentialString="Error";
    }
    return credentialString;
}

proto::CredentialType Credential::Type() const

{
    return type_;
}

serializedCredential Credential::asSerialized(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const

{
    serializedCredential serializedCredential = std::make_shared<proto::Credential>();

    serializedCredential->set_version(version_);
    serializedCredential->set_type(static_cast<proto::CredentialType>(type_));

    if (asPrivate) {
        OT_ASSERT(proto::KEYMODE_PRIVATE == mode_);
        serializedCredential->set_mode(mode_);

    } else {
        serializedCredential->set_mode(proto::KEYMODE_PUBLIC);
    }

    if (asSigned) {
        serializedSignature publicSig;
        serializedSignature privateSig;
        serializedSignature sourceSig;

        proto::Signature* pPrivateSig;
        proto::Signature* pPublicSig;
        proto::Signature* pSourceSig;

        if (asPrivate) {
            privateSig = SelfSignature(true);

            OT_ASSERT(nullptr != privateSig);
            if (nullptr != privateSig) {
                pPrivateSig = serializedCredential->add_signature();
                *pPrivateSig = *privateSig;
            }
        }

        publicSig = SelfSignature(false);

        OT_ASSERT(nullptr != publicSig);
        if (nullptr != publicSig) {
            pPublicSig = serializedCredential->add_signature();
            *pPublicSig = *SelfSignature(false);
        }

        sourceSig = SourceSignature();
        if (sourceSig) {
            pSourceSig = serializedCredential->add_signature();
            *pSourceSig = *sourceSig;
        }
    } else {
        serializedCredential->clear_signature(); // just in case...
    }

    String credID;
    GetIdentifier(credID);

    serializedCredential->set_id(credID.Get());
    serializedCredential->set_nymid(NymID().Get());

    return serializedCredential;
}

bool Credential::SaveContract()
{
    serializedCredential serializedProto;

    if (!isValid(serializedProto)) {
        otErr << __FUNCTION__ << ": Invalid serialized credential.\n";
        OT_ASSERT(false);
        return false;
    }

    OTData serializedData = proto::ProtoAsData<proto::Credential>(*serializedProto);
    OTASCIIArmor armoredData(serializedData);
    m_strRawFile.Set(armoredData.Get());

    return true;
}

serializedSignature Credential::SelfSignature(CredentialModeFlag version) const
{
    proto::SignatureRole targetRole;

    if (Credential::PRIVATE_VERSION == version) {
        targetRole = proto::SIGROLE_PRIVCREDENTIAL;
    } else {
        targetRole = proto::SIGROLE_PUBCREDENTIAL;
    }

    // Perhaps someday this method will return a list of matching signatures
    // For now, it only returns the first one.
    for (auto& it : m_listSerializedSignatures) {
        if (it->role() == targetRole) {
            return it;
        }
    }

    return nullptr;
}

serializedSignature Credential::SourceSignature() const
{
    serializedSignature signature;

    for (auto& it : m_listSerializedSignatures) {
        if (it->role() == proto::SIGROLE_NYMIDSOURCE) {
            signature = std::make_shared<proto::Signature>(*it);

            break;
        }
    }
    return signature;
}

bool Credential::SaveContract(const char* szFoldername, const char* szFilename)
{
    OT_ASSERT_MSG(nullptr != szFilename,
                  "Null filename sent to Contract::SaveContract\n");
    OT_ASSERT_MSG(nullptr != szFoldername,
                  "Null foldername sent to Contract::SaveContract\n");

    m_strFoldername.Set(szFoldername);
    m_strFilename.Set(szFilename);

    OT_ASSERT(m_strFoldername.GetLength() > 2);
    OT_ASSERT(m_strFilename.GetLength() > 2);

    OT_ASSERT(SaveContract());

    if (!m_strRawFile.Exists()) {
        otErr << "Contract::SaveContract: Error saving file (contract "
                 "contents are empty): " << szFoldername << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    }

    bool bSaved =
        OTDB::StorePlainString(m_strRawFile.Get(), szFoldername, szFilename);

    if (!bSaved) {
        otErr << "Contract::SaveContract: Error saving file: " << szFoldername
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    return true;
}

bool Credential::isPrivate() const
{
    return (proto::KEYMODE_PRIVATE == mode_);
}

bool Credential::isPublic() const
{
    return (proto::KEYMODE_PUBLIC == mode_);
}

std::string Credential::asString(const bool asPrivate) const
{
    serializedCredential credenial;
    OTData dataCredential;
    String stringCredential;

    credenial = asSerialized(asPrivate, Credential::WITH_SIGNATURES);
    dataCredential = proto::ProtoAsData<proto::Credential>(*credenial);

    OTASCIIArmor armoredCredential(dataCredential);

    armoredCredential.WriteArmoredString(stringCredential, "Credential");

    return stringCredential.Get();
}

//static
serializedCredential Credential::ExtractArmoredCredential(const String stringCredential)
{
    OTASCIIArmor armoredCredential;
    String strTemp(stringCredential);
    armoredCredential.LoadFromString(strTemp);
    return ExtractArmoredCredential(armoredCredential);
}

//static
serializedCredential Credential::ExtractArmoredCredential(const OTASCIIArmor armoredCredential)
{
    OTData dataCredential(armoredCredential);

    serializedCredential serializedCred = std::make_shared<proto::Credential>();

    serializedCred->ParseFromArray(dataCredential.GetPointer(), dataCredential.GetSize());

    return serializedCred;
}

void Credential::ReleaseSignatures(const bool onlyPrivate)
{
    for (auto i = m_listSerializedSignatures.begin(); i != m_listSerializedSignatures.end();) {
        if (!onlyPrivate ||
            (onlyPrivate && (proto::SIGROLE_PRIVCREDENTIAL == (*i)->role()))) {
            i = m_listSerializedSignatures.erase(i);
        } else {
            i++;
        }
    }
}

bool Credential::AddMasterSignature()
{
    if (nullptr == owner_backlink_) {
        otErr << __FUNCTION__ << ": Missing master credential.\n";
        return false;
    }

    serializedSignature serializedMasterSignature =
        std::make_shared<proto::Signature>();

    bool havePublicSig = owner_backlink_->Sign(
        *this,
        *serializedMasterSignature);

    if (!havePublicSig) {
        otErr << __FUNCTION__ << ": Failed to obtain signature from master credential.\n";
        return false;
    }

    m_listSerializedSignatures.push_back(serializedMasterSignature);

    return true;
}

// Override this method for credentials capable of returning contact data.
bool Credential::GetContactData(proto::ContactData& contactData) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.\n");

    return false;
}

// Override this method for credentials capable of signing Contracts and
// producing xml signatures.
bool Credential::Sign(Contract& theContract, const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.\n");

    return false;
}

// Override this method for credentials capable of signing binary data and
// producing protobuf signatures.
bool Credential::Sign(
    const OTData& plaintext,
    proto::Signature& sig,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword,
    const proto::SignatureRole role,
    proto::KeyRole key) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.\n");

    return false;
}

// Override this method for credentials capable of verifying other credentials
bool Credential::Verify(const Credential& credential) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.\n");

    return false;
}

} // namespace opentxs
