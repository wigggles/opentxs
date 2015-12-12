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

#include <opentxs/core/crypto/KeyCredential.hpp>

#include <opentxs/core/stdafx.hpp>
#include <opentxs/core/Proto.hpp>
#include <opentxs/core/crypto/CredentialSet.hpp>
#include <opentxs/core/crypto/CryptoEngine.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/Log.hpp>

namespace opentxs
{

bool KeyCredential::VerifySignedBySelf() const
{
    OT_ASSERT(m_SigningKey);

    serializedSignature publicSig = GetSelfSignature(false);

    if (!publicSig) {
        otErr << __FUNCTION__ << ": Could not find public self signature.\n";
        return false;
    }

    bool goodPublic = VerifySig(*publicSig, m_SigningKey->GetPublicKey(), false);

    if (!goodPublic) {
        otErr << __FUNCTION__ << ": Could not verify public self signature.\n";
        return false;
    }

    if (isPrivate())
    {
        serializedSignature privateSig = GetSelfSignature(true);

        if (!privateSig)
        {
            otErr << __FUNCTION__ << ": Could not find private self signature.\n";
            return false;
        }

        bool goodPrivate = VerifySig(*privateSig, m_SigningKey->GetPublicKey(), true);

        if (!goodPrivate) {
            otErr << __FUNCTION__ << ": Could not verify private self signature.\n";
            return false;
        }
    }

    return true;
}

// NOTE: You might ask, if we are using theSignature's metadata to narrow down
// the key type,
// then why are we still passing the key type as a separate parameter? Good
// question. Because
// often, theSignature will have no metadata at all! In that case, normally we
// would just NOT
// return any keys, period. Because we assume, if a key credential signed it,
// then it WILL have
// metadata, and if it doesn't have metadata, then a key credential did NOT sign
// it, and therefore
// we know from the get-go that none of the keys from the key credentials will
// work to verify it,
// either. That's why, normally, we don't return any keys if theSignature has no
// metadata.
// BUT...Let's say you know this, that the signature has no metadata, yet you
// also still believe
// it may be signed with one of these keys. Further, while you don't know
// exactly which key it
// actually is, let's say you DO know by context that it's a signing key, or an
// authentication key,
// or an encryption key. So you specify that. In which case, OT should return
// all possible matching
// pubkeys based on that 1-letter criteria, instead of its normal behavior,
// which is to return all
// possible matching pubkeys based on a full match of the metadata.
//
int32_t KeyCredential::GetPublicKeysBySignature(
    listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
    char cKeyType) const // 'S' (signing key) or 'E' (encryption key)
                         // or 'A' (authentication key)
{
    // Key type was not specified, because we only want keys that match the
    // metadata on theSignature.
    // And if theSignature has no metadata, then we want to return 0 keys.
    if (('0' == cKeyType) && !theSignature.getMetaData().HasMetadata())
        return 0;

    // By this point, we know that EITHER exact metadata matches must occur, and
    // the signature DOES have metadata, ('0')
    // OR the search is only for 'A', 'E', or 'S' candidates, based on cKeyType,
    // and that the signature's metadata
    // can additionally narrow the search down, if it's present, which in this
    // case it's not guaranteed to be.
    int32_t nCount = 0;

    OT_ASSERT(m_AuthentKey);
    OT_ASSERT(m_EncryptKey);
    OT_ASSERT(m_SigningKey);

    switch (cKeyType) {
    // Specific search only for signatures with metadata.
    // FYI, theSignature.getMetaData().HasMetadata() is true, in this case.
    case '0': {
        // That's why I can just assume theSignature has a key type here:
        switch (theSignature.getMetaData().GetKeyType()) {
        case 'A':
            nCount =
                m_AuthentKey->GetPublicKeyBySignature(listOutput, theSignature);
            break; // bInclusive=false by default
        case 'E':
            nCount =
                m_EncryptKey->GetPublicKeyBySignature(listOutput, theSignature);
            break; // bInclusive=false by default
        case 'S':
            nCount =
                m_SigningKey->GetPublicKeyBySignature(listOutput, theSignature);
            break; // bInclusive=false by default
        default:
            otErr << __FUNCTION__
                  << ": Unexpected keytype value in signature metadata: "
                  << theSignature.getMetaData().GetKeyType() << " (failure)\n";
            return 0;
        }
        break;
    }
    // Generalized search which specifies key type and returns keys
    // even for signatures with no metadata. (When metadata is present,
    // it's still used to eliminate keys.)
    case 'A':
        nCount = m_AuthentKey->GetPublicKeyBySignature(listOutput, theSignature,
                                                      true);
        break; // bInclusive=true
    case 'E':
        nCount = m_EncryptKey->GetPublicKeyBySignature(listOutput, theSignature,
                                                      true);
        break; // bInclusive=true
    case 'S':
        nCount = m_SigningKey->GetPublicKeyBySignature(listOutput, theSignature,
                                                      true);
        break; // bInclusive=true
    default:
        otErr << __FUNCTION__
              << ": Unexpected value for cKeyType (should be 0, A, E, or S): "
              << cKeyType << "\n";
        return 0;
    }
    return nCount;
}

// Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
// Child key verifies (master does not): NymID against master credential NymID, and master
// credential ID against hash of master credential.
// (How? Because MasterCredential overrides this function and does NOT call the
// parent, whereas ChildKeyCredential does.)
// Then verify the (self-signed) signature on *this.
//
bool KeyCredential::VerifyInternally()
{
    // Verify that m_strNymID is the same as the hash of m_strSourceForNymID.
    if (!ot_super::VerifyInternally()) return false;

    // Any KeyCredential (both master and child, but no other credentials)
    // must ** sign itself.**
    //
    if (!VerifySignedBySelf()) {
        otOut << __FUNCTION__ << ": Failed verifying key credential: it's not "
                                 "signed by itself (its own signing key.)\n";
        return false;
    }

    return true;
}

// otErr << "%s line %d: \n", __FILE__, __LINE__);

KeyCredential::KeyCredential(CredentialSet& theOwner, const proto::Credential& serializedCred)
: ot_super(theOwner, serializedCred)
{
    const bool hasPrivate = (proto::KEYMODE_PRIVATE == serializedCred.mode())
        ? true : false;

    // Auth key
    proto::AsymmetricKey publicAuth = serializedCred.publiccredential().key(proto::KEYROLE_AUTH - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateAuth = serializedCred.privatecredential().key(proto::KEYROLE_AUTH - 1);

        m_AuthentKey = std::make_shared<OTKeypair>(publicAuth, privateAuth);
    } else {
        m_AuthentKey = std::make_shared<OTKeypair>(publicAuth);
    }

    // Encrypt key
    proto::AsymmetricKey publicEncrypt = serializedCred.publiccredential().key(proto::KEYROLE_ENCRYPT - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateEncrypt = serializedCred.privatecredential().key(proto::KEYROLE_ENCRYPT - 1);

        m_EncryptKey = std::make_shared<OTKeypair>(publicEncrypt, privateEncrypt);
    } else {
        m_EncryptKey = std::make_shared<OTKeypair>(publicEncrypt);
    }

    // Sign key
    proto::AsymmetricKey publicSign = serializedCred.publiccredential().key(proto::KEYROLE_SIGN - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateSign = serializedCred.privatecredential().key(proto::KEYROLE_SIGN - 1);

        m_SigningKey = std::make_shared<OTKeypair>(publicSign, privateSign);
    } else {
        m_SigningKey = std::make_shared<OTKeypair>(publicSign);
    }
}

KeyCredential::KeyCredential(
    CredentialSet& theOwner,
    const NymParameters& nymParameters,
    const proto::CredentialRole role)
        : ot_super(theOwner, nymParameters)
{
    if (Credential::HD != nymParameters.credentialType()) {
        m_AuthentKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_AUTH);
        m_EncryptKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_ENCRYPT);
        m_SigningKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_SIGN);
    } else {
        m_AuthentKey =
            DeriveHDKeypair(
                nymParameters.Nym(),
                0, // FIXME When multiple credential sets per nym are
                   // implemented, this number must increment with each one.
                (proto::CREDROLE_MASTERKEY == role) ? 0 : 1, // FIXME
                   // When multiple child credentials per credential set
                   // are imeplemnted, this number must increment with each one.
                proto::KEYROLE_AUTH);
        m_EncryptKey =
            DeriveHDKeypair(
                nymParameters.Nym(),
                0, //FIXME
                (proto::CREDROLE_MASTERKEY == role) ? 0 : 1, //FIXME
                proto::KEYROLE_ENCRYPT);
        m_SigningKey =
            DeriveHDKeypair(
                nymParameters.Nym(),
                0, //FIXME
                (proto::CREDROLE_MASTERKEY == role) ? 0 : 1, //FIXME
                proto::KEYROLE_SIGN);
    }
}

std::shared_ptr<OTKeypair> KeyCredential::DeriveHDKeypair(
    const uint32_t nym,
    const uint32_t credset,
    const uint32_t credindex,
    const proto::KeyRole role)
{
    proto::HDPath keyPath;
    keyPath.add_child(NYM_PURPOSE | HARDENED);
    keyPath.add_child(nym | HARDENED);
    keyPath.add_child(credset | HARDENED);
    keyPath.add_child(credindex | HARDENED);

    switch (role) {
        case proto::KEYROLE_AUTH :
            keyPath.add_child(AUTH_KEY | HARDENED);
            break;
        case proto::KEYROLE_ENCRYPT :
            keyPath.add_child(ENCRYPT_KEY | HARDENED);
            break;
        case proto::KEYROLE_SIGN :
            keyPath.add_child(SIGN_KEY | HARDENED);
            break;
        default :
            break;
    }

    serializedAsymmetricKey privateKey =
        CryptoEngine::Instance().BIP32().GetHDKey(keyPath);
    privateKey->set_role(role);

    serializedAsymmetricKey publicKey = CryptoEngine::Instance().BIP32().PrivateToPublic(*privateKey);

    std::shared_ptr<OTKeypair> newKeypair = std::make_shared<OTKeypair>(
        *publicKey,
        *privateKey);

    return newKeypair;
}

KeyCredential::~KeyCredential()
{
    Release_KeyCredential();
}

// virtual
void KeyCredential::Release()
{
    Release_KeyCredential(); // My own cleanup is done here.

    // Next give the base class a chance to do the same...
    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

void KeyCredential::Release_KeyCredential()
{
    // Release any dynamically allocated members here. (Normally.)

}

bool KeyCredential::Sign(Contract& theContract, const OTPasswordData* pPWData)
{
    OT_ASSERT(m_SigningKey);

    return m_SigningKey->SignContract(theContract, pPWData);
}

bool KeyCredential::ReEncryptKeys(const OTPassword& theExportPassword,
                                    bool bImporting)
{
    OT_ASSERT(m_AuthentKey);
    OT_ASSERT(m_EncryptKey);
    OT_ASSERT(m_SigningKey);

    const bool bSign = m_SigningKey->ReEncrypt(theExportPassword, bImporting);
    const bool bAuth = m_AuthentKey->ReEncrypt(theExportPassword, bImporting);
    const bool bEncr = m_EncryptKey->ReEncrypt(theExportPassword, bImporting);

    const bool bSuccessReEncrypting = (bSign && bAuth && bEncr);
    OT_ASSERT(bSuccessReEncrypting);

    return bSuccessReEncrypting; // Note: Caller must re-sign credential after doing this,
                     // to keep these changes.
}

serializedCredential KeyCredential::Serialize(bool asPrivate, bool asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::Serialize(asPrivate, asSigned);

    addKeyCredentialtoSerializedCredential(serializedCredential, false);

    if (asPrivate) {
        addKeyCredentialtoSerializedCredential(serializedCredential, true);
    }

    return serializedCredential;
}

bool KeyCredential::addKeytoSerializedKeyCredential(
    proto::KeyCredential& credential,
    const bool getPrivate,
    const proto::KeyRole role) const
{
    serializedAsymmetricKey key;
    std::shared_ptr<OTKeypair>(pKey);

    switch (role) {
        case proto::KEYROLE_AUTH :
            pKey = m_AuthentKey;
            break;
        case proto::KEYROLE_ENCRYPT :
            pKey = m_EncryptKey;
            break;
        case proto::KEYROLE_SIGN :
            pKey = m_SigningKey;
            break;
        default :
            return false;
    }

    key = pKey->Serialize(getPrivate);
    key->set_role(role);

    proto::AsymmetricKey* newKey = credential.add_key();
    *newKey = *key;

    return true;
}

bool KeyCredential::addKeyCredentialtoSerializedCredential(
    serializedCredential credential,
    const bool addPrivate) const
{
    proto::KeyCredential* keyCredential = new proto::KeyCredential;

    if (nullptr == keyCredential) {
        otErr << "opentxs::KeyCredential" << __FUNCTION__ << "(): failed to allocate keyCredential protobuf.\n";
        return false;
    }

    keyCredential->set_version(1);

    // These must be serialized in this order
    bool auth = addKeytoSerializedKeyCredential(*keyCredential, addPrivate, proto::KEYROLE_AUTH);
    bool encrypt = addKeytoSerializedKeyCredential(*keyCredential, addPrivate, proto::KEYROLE_ENCRYPT);
    bool sign = addKeytoSerializedKeyCredential(*keyCredential, addPrivate, proto::KEYROLE_SIGN);

    if (auth && encrypt && sign) {
        if (addPrivate) {
            keyCredential->set_mode(proto::KEYMODE_PRIVATE);
            credential->set_allocated_privatecredential(keyCredential);

            return true;
        } else  {
            keyCredential->set_mode(proto::KEYMODE_PUBLIC);
            credential->set_allocated_publiccredential(keyCredential);

            return true;
        }
    }

    return false;
}

bool KeyCredential::Sign(
    const OTData& plaintext,
    proto::Signature& sig,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword,
    const proto::SignatureRole role,
    proto::KeyRole key) const
{
    const OTAsymmetricKey* keyToUse;

    switch (key) {
        case (proto::KEYROLE_AUTH) :
            keyToUse = &(this->m_AuthentKey->GetPrivateKey());
            break;
        case (proto::KEYROLE_SIGN) :
            keyToUse = &(this->m_SigningKey->GetPrivateKey());
            break;
        default :
            otErr << __FUNCTION__ << ": Can not sign with the specified key.\n";
            return false;
    }

    String credID;
    GetIdentifier(credID);

    return keyToUse->Sign(plaintext, sig, pPWData, exportPassword, credID, role);
}

bool KeyCredential::Sign(
        const Credential& plaintext,
        proto::Signature& sig,
        const OTPasswordData* pPWData,
        const OTPassword* exportPassword,
        const proto::SignatureRole role) const
{
    serializedCredential serialized = plaintext.SerializeForPublicSignature();
    return Sign(
        proto::ProtoAsData<proto::Credential>(*serialized),
        sig,
        pPWData,
        exportPassword,
        role);
}

bool KeyCredential::SelfSign(
    const OTPassword* exportPassword,
    const OTPasswordData* pPWData,
    const bool onlyPrivate)
{
    String credID;
    GetIdentifier(credID);

    serializedSignature publicSignature =
        std::make_shared<proto::Signature>();
    serializedSignature privateSignature =
        std::make_shared<proto::Signature>();

    bool havePublicSig = false;
    if (!onlyPrivate) {
        const serializedCredential publicVersion = SerializeForPublicSignature();
        havePublicSig = Sign(
            proto::ProtoAsData<proto::Credential>(*publicVersion),
            *publicSignature,
            pPWData,
            exportPassword,
            proto::SIGROLE_PUBCREDENTIAL);

        if (havePublicSig) {
            m_listSerializedSignatures.push_back(publicSignature);
        }
    }

    serializedCredential privateVersion = SerializeForPrivateSignature();
    bool havePrivateSig = Sign(
        proto::ProtoAsData<proto::Credential>(*privateVersion),
        *privateSignature,
        pPWData,
        exportPassword,
        proto::SIGROLE_PRIVCREDENTIAL);

    if (havePrivateSig) {
        m_listSerializedSignatures.push_back(privateSignature);
    }

    return ((havePublicSig | onlyPrivate) && havePrivateSig);
}

bool KeyCredential::VerifySig(
                            const proto::Signature& sig,
                            const OTAsymmetricKey& theKey,
                            const bool asPrivate) const
{
    serializedCredential serialized;

    if ((proto::KEYMODE_PRIVATE != m_mode) && asPrivate) {
        otErr << __FUNCTION__ << ": Can not serialize a public credential as a private credential.\n";
        return false;
    }

    if (asPrivate) {
        serialized = SerializeForPrivateSignature();
    } else {
        serialized = SerializeForPublicSignature();
    }

    OTData plaintext = SerializeCredToData(*serialized);

    return theKey.Verify(plaintext, sig);
}

} // namespace opentxs
