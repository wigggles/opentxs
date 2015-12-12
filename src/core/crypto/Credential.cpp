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

#include <opentxs/core/crypto/Credential.hpp>

#include <opentxs/core/crypto/OTASCIIArmor.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <map>

// Contains 3 key pairs: signing, authentication, and encryption.
// This is stored as an Contract, and it must be signed by the
// master key. (which is also an Credential.)
//

namespace opentxs
{

Credential::Credential(CredentialSet& theOwner, const NymParameters& nymParameters)
    : Contract()
    , m_Type(nymParameters.credentialType())
    , m_mode(proto::KEYMODE_PRIVATE)
    , m_pOwner(&theOwner)
{
    m_strContractType = "CREDENTIAL";
}

Credential::Credential(CredentialSet& theOwner, const proto::Credential& serializedCred)
    : Contract()
    , m_Type(static_cast<Credential::CredentialType>(serializedCred.type()))
    , m_mode(serializedCred.mode())
    , m_pOwner(&theOwner)
    {
    m_strContractType = "CREDENTIAL";
    m_Role = serializedCred.role();

    if (serializedCred.has_nymid()) {
        m_strNymID = serializedCred.nymid();
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

void Credential::SetMasterCredID(const String& strMasterCredID)
{
    m_strMasterCredID = strMasterCredID;
}

// VERIFICATION

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
//
bool Credential::VerifyNymID() const
{

    return (m_strNymID == m_pOwner->GetNymID());
}

// Call VerifyNymID. Then verify m_strMasterCredID against the hash of
// m_pOwner->GetMasterCredential().GetPubCredential() (the master credential.) Verify
// that
// m_pOwner->GetMasterCredential() and *this have the same NymID. Then verify the
// signature of m_pOwner->GetMasterCredential().
//
bool Credential::VerifyInternally()
{
    OT_ASSERT(nullptr != m_pOwner);

    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    //
    if (!VerifyNymID()) return false;

    // Verify that m_pOwner->GetMasterCredential() and *this have the same NymID.
    //
    if (!m_strNymID.Compare(m_pOwner->GetMasterCredential().GetNymID())) {
        otOut << __FUNCTION__
              << ": Failure: The actual master credential's NymID doesn't "
                 "match the NymID on this child credential.\n"
                 "    This NymID: " << m_strNymID
              << "\nMaster's NymID: " << m_pOwner->GetMasterCredential().GetNymID()
              << "\n My Master Cred ID: " << m_strMasterCredID << "\n";
        return false;
    }

    // Verify m_strMasterCredID against the hash of
    // m_pOwner->GetMasterCredential().GetPubCredential()
    // (the master credentialID is a hash of the master credential.)
    //
    String strActualMasterID;
    m_pOwner->GetMasterCredential().GetIdentifier(strActualMasterID);

    if (!m_strMasterCredID.Compare(strActualMasterID)) {
        otOut << __FUNCTION__
              << ": Failure: When the actual Master Credential is hashed, the "
                 "result doesn't match the expected Master Credential ID.\n"
                 "Expected: " << m_strMasterCredID
              << "\n   Found: " << strActualMasterID << "\nMaster Cred:\n" << "\n";
        return false;
    }

    // Then verify the signature of m_pOwner->GetMasterCredential()...
    // Let's get a few things straight:
    // * MasterCredential is a key (derived from KeyCredential, derived from
    // Credential) and it can only sign itself.
    // * The only further verification a master credential can get is if its hash is
    // posted at the source. Or, if the source
    //   is a public key, then the master key must be signed by the
    // corresponding private key. (Again: itself.)
    // * Conversely to a master credential which can ONLY sign itself, all key credentials must
    // ALSO sign themselves.
    //
    // * Thus: Any KeyCredential (both master and child, but no other
    // credentials) must ** sign itself.**
    // * Whereas m_strMasterSigned is only used by ChildKeyCredential, and thus only must
    // be verified there.
    // * Any Credential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    // * Any MasterCredential must (at some point, and/or regularly) verify against
    // its own source.

    // * Any Credential must also be signed by its master. (Except masters,
    // which already sign themselves.)
    //
    if (!VerifySignedByMaster()) {
        otOut << __FUNCTION__ << ": Failure: This child credential hasn't been "
                                 "signed by its master credential.\n";
        return false;
    }

    return true;
}

bool Credential::VerifySignedByMaster()
{
    OT_ASSERT(nullptr != m_pOwner);
    OT_ASSERT(m_pOwner->GetMasterCredential().m_SigningKey);
    return VerifyWithKey(m_pOwner->GetMasterCredential().m_SigningKey->GetPublicKey());
}

bool Credential::VerifyContract()
{
    if (!VerifyContractID()) {
        otWarn << __FUNCTION__ << ": Failed verifying credential ID against "
                                  "whatever it was expected to be.\n";
        return false;
    }

    if (!VerifyInternally()) // Logs copiously.
        return false;

    return true;
}

// Overriding from Contract.
void Credential::CalculateContractID(Identifier& newID) const
{
    serializedCredential idVersion = SerializeForIdentifier();

    OTData serializedData = SerializeCredToData(*idVersion);

    if (!newID.CalculateDigest(serializedData))
        otErr << __FUNCTION__ << ": Error calculating credential digest.\n";
}

const serializedCredential Credential::GetSerializedPubCredential() const
{
    return Serialize(false, true);
}

String Credential::CredentialTypeToString(Credential::CredentialType credentialType)

{
    String credentialString;

    switch (credentialType) {
        case Credential::LEGACY :
            credentialString="legacy";
            break;
        case Credential::HD :
            credentialString="hd";
            break;
        default :
            credentialString="error";
    }
    return credentialString;
}

Credential::CredentialType Credential::StringToCredentialType(const String & credentialType)

{
    if (credentialType.Compare("legacy"))
        return Credential::LEGACY;
    else if (credentialType.Compare("hd"))
        return Credential::HD;
    return Credential::ERROR_TYPE;
}

Credential::CredentialType Credential::GetType() const

{
    return m_Type;
}

serializedCredential Credential::Serialize(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const

{
    serializedCredential serializedCredential = std::make_shared<proto::Credential>();

    serializedCredential->set_version(1);
    serializedCredential->set_type(static_cast<proto::CredentialType>(m_Type));

    if (asPrivate) {
        OT_ASSERT(proto::KEYMODE_PRIVATE == m_mode);
        serializedCredential->set_mode(m_mode);

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
            privateSig = GetSelfSignature(true);

            OT_ASSERT(nullptr != privateSig);
            if (nullptr != privateSig) {
                pPrivateSig = serializedCredential->add_signature();
                *pPrivateSig = *privateSig;
            }
        }

        publicSig = GetSelfSignature(false);

        OT_ASSERT(nullptr != publicSig);
        if (nullptr != publicSig) {
            pPublicSig = serializedCredential->add_signature();
            *pPublicSig = *GetSelfSignature(false);
        }

        sourceSig = GetSourceSignature();
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
    serializedCredential->set_nymid(GetNymID().Get());

    return serializedCredential;
}

serializedCredential Credential::SerializeForPublicSignature() const
{
    serializedCredential pubsigVersion = Serialize(AS_PUBLIC, WITHOUT_SIGNATURES);

    return pubsigVersion;
}

serializedCredential Credential::SerializeForPrivateSignature() const
{
    serializedCredential privsigVersion = Serialize(AS_PRIVATE, WITHOUT_SIGNATURES);

    return privsigVersion;
}

serializedCredential Credential::SerializeForIdentifier() const
{
    serializedCredential idVersion = SerializeForPublicSignature();

    if (idVersion->has_id()) {
        idVersion->clear_id();
    }

    return idVersion;
}

OTData Credential::SerializeCredToData(const proto::Credential& serializedCred) const
{
    return proto::ProtoAsData<proto::Credential>(serializedCred);
}

bool Credential::SaveContract()
{
    SerializationModeFlag serializationMode = AS_PUBLIC;

    if (proto::KEYMODE_PRIVATE == m_mode) {
        serializationMode = AS_PRIVATE;
    }

    serializedCredential serializedProto = Serialize(serializationMode, WITH_SIGNATURES);

    bool validProto = proto::Verify(*serializedProto, m_Role, WITH_SIGNATURES);

    if (!validProto) {
        otErr << __FUNCTION__ << ": Invalid serialized credential.\n";
        return false;
    }

    OT_ASSERT(validProto);

    OTData serializedData = proto::ProtoAsData<proto::Credential>(*serializedProto);
    OTASCIIArmor armoredData(serializedData);
    m_strRawFile.Set(armoredData.Get());

    return true;
}

serializedSignature Credential::GetSelfSignature(CredentialModeFlag version) const
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

serializedSignature Credential::GetSourceSignature() const
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
    return (proto::KEYMODE_PRIVATE == m_mode);
}

bool Credential::isPublic() const
{
    return (proto::KEYMODE_PUBLIC == m_mode);
}

std::string Credential::AsString(const bool asPrivate) const
{
    serializedCredential credenial;
    OTData dataCredential;
    String stringCredential;

    credenial = Serialize(asPrivate, true);
    dataCredential = SerializeCredToData(*credenial);

    OTASCIIArmor armoredCredential(dataCredential);

    armoredCredential.WriteArmoredString(stringCredential, "Credential");

    return stringCredential.Get();
}

bool Credential::LoadContractFromString(__attribute__((unused)) const String& theStr)
{
    OT_ASSERT_MSG(false, "Error: Any code still calling this function is broken and wrong.\n");
    return false;
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

} // namespace opentxs
