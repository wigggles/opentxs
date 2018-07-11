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
// Each KeyCredential has 3 crypto::key::Keypairs: encryption, signing, and
// authentication. Each crypto::key::Keypair has 2 crypto::key::Asymmetrics
// (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "stdafx.hpp"

#include "opentxs/core/crypto/KeyCredential.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTSignature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/Sodium.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/crypto/Bip39.hpp"
#endif
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <ostream>

namespace opentxs
{

bool KeyCredential::VerifySignedBySelf(const Lock& lock) const
{
    OT_ASSERT(m_SigningKey);

    auto publicSig = SelfSignature(PUBLIC_VERSION);

    if (!publicSig) {
        otErr << __FUNCTION__ << ": Could not find public self signature."
              << std::endl;

        return false;
    }

    bool goodPublic = VerifySig(lock, *publicSig, PUBLIC_VERSION);

    if (!goodPublic) {
        otErr << __FUNCTION__ << ": Could not verify public self signature."
              << std::endl;

        return false;
    }

    if (Private()) {
        auto privateSig = SelfSignature(PRIVATE_VERSION);

        if (!privateSig) {
            otErr << __FUNCTION__ << ": Could not find private self signature."
                  << std::endl;

            return false;
        }

        bool goodPrivate = VerifySig(lock, *privateSig, PRIVATE_VERSION);

        if (!goodPrivate) {
            otErr << __FUNCTION__
                  << ": Could not verify private self signature." << std::endl;

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
std::int32_t KeyCredential::GetPublicKeysBySignature(
    listOfAsymmetricKeys& listOutput,
    const OTSignature& theSignature,
    char cKeyType) const  // 'S' (signing key) or 'E' (encryption key)
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
    std::int32_t nCount = 0;

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
                    nCount = m_AuthentKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'E':
                    nCount = m_EncryptKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'S':
                    nCount = m_SigningKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                default:
                    otErr
                        << __FUNCTION__
                        << ": Unexpected keytype value in signature metadata: "
                        << theSignature.getMetaData().GetKeyType()
                        << " (failure)\n";
                    return 0;
            }
            break;
        }
        // Generalized search which specifies key type and returns keys
        // even for signatures with no metadata. (When metadata is present,
        // it's still used to eliminate keys.)
        case 'A':
            nCount = m_AuthentKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'E':
            nCount = m_EncryptKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'S':
            nCount = m_SigningKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        default:
            otErr
                << __FUNCTION__
                << ": Unexpected value for cKeyType (should be 0, A, E, or S): "
                << cKeyType << "\n";
            return 0;
    }
    return nCount;
}

bool KeyCredential::verify_internally(const Lock& lock) const
{
    // Perform common Credential verifications
    if (!ot_super::verify_internally(lock)) { return false; }

    // All KeyCredentials must sign themselves
    if (!VerifySignedBySelf(lock)) {
        otOut << __FUNCTION__
              << ": Failed verifying key credential: it's not "
                 "signed by itself (its own signing key.)\n";
        return false;
    }

    return true;
}

KeyCredential::KeyCredential(
    const api::client::Wallet& wallet,
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(wallet, theOwner, serializedCred)
{
    const bool hasPrivate =
        (proto::KEYMODE_PRIVATE == serializedCred.mode()) ? true : false;

    // Auth key
    proto::AsymmetricKey publicAuth =
        serializedCred.publiccredential().key(proto::KEYROLE_AUTH - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateAuth =
            serializedCred.privatecredential().key(proto::KEYROLE_AUTH - 1);

        m_AuthentKey =
            std::make_shared<crypto::key::Keypair>(publicAuth, privateAuth);
    } else {
        m_AuthentKey = std::make_shared<crypto::key::Keypair>(publicAuth);
    }

    // Encrypt key
    proto::AsymmetricKey publicEncrypt =
        serializedCred.publiccredential().key(proto::KEYROLE_ENCRYPT - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateEncrypt =
            serializedCred.privatecredential().key(proto::KEYROLE_ENCRYPT - 1);

        m_EncryptKey = std::make_shared<crypto::key::Keypair>(
            publicEncrypt, privateEncrypt);
    } else {
        m_EncryptKey = std::make_shared<crypto::key::Keypair>(publicEncrypt);
    }

    // Sign key
    proto::AsymmetricKey publicSign =
        serializedCred.publiccredential().key(proto::KEYROLE_SIGN - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateSign =
            serializedCred.privatecredential().key(proto::KEYROLE_SIGN - 1);

        m_SigningKey =
            std::make_shared<crypto::key::Keypair>(publicSign, privateSign);
    } else {
        m_SigningKey = std::make_shared<crypto::key::Keypair>(publicSign);
    }
}

KeyCredential::KeyCredential(
    const api::client::Wallet& wallet,
    CredentialSet& theOwner,
    const NymParameters& nymParameters)
    : ot_super(wallet, theOwner, KEY_CREDENTIAL_VERSION, nymParameters)
{
    if (proto::CREDTYPE_HD != nymParameters.credentialType()) {
        m_AuthentKey = std::make_shared<crypto::key::Keypair>(
            nymParameters, proto::KEYROLE_AUTH);
        m_EncryptKey = std::make_shared<crypto::key::Keypair>(
            nymParameters, proto::KEYROLE_ENCRYPT);
        m_SigningKey = std::make_shared<crypto::key::Keypair>(
            nymParameters, proto::KEYROLE_SIGN);
    } else {
#if OT_CRYPTO_SUPPORTED_KEY_HD
        const auto keyType = nymParameters.AsymmetricKeyType();
        const auto curve = crypto::AsymmetricProvider::KeyTypeToCurve(keyType);

        if ((EcdsaCurve::ERROR != curve) && nymParameters.Entropy()) {
            m_AuthentKey = DeriveHDKeypair(
                *nymParameters.Entropy(),
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_AUTH);
            m_EncryptKey = DeriveHDKeypair(
                *nymParameters.Entropy(),
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_ENCRYPT);
            m_SigningKey = DeriveHDKeypair(
                *nymParameters.Entropy(),
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_SIGN);
        }
#endif
    }
}

bool KeyCredential::New(const NymParameters& nymParameters)
{
    bool output = false;

    output = ot_super::New(nymParameters);

    if (output) {
        output = SelfSign();
    } else {
        OT_FAIL;
    }

    OT_ASSERT(output);

    return output;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
std::shared_ptr<crypto::key::Keypair> KeyCredential::DeriveHDKeypair(
    const OTPassword& seed,
    const std::string& fingerprint,
    const std::uint32_t nym,
    const std::uint32_t credset,
    const std::uint32_t credindex,
    const EcdsaCurve& curve,
    const proto::KeyRole role)
{
    proto::HDPath keyPath;
    keyPath.set_version(1);
    std::string input(fingerprint);
    keyPath.set_root(input.c_str(), input.size());

    keyPath.add_child(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(nym | static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(
        credset | static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(
        credindex | static_cast<std::uint32_t>(Bip32Child::HARDENED));

    switch (role) {
        case proto::KEYROLE_AUTH:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::AUTH_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        case proto::KEYROLE_ENCRYPT:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::ENCRYPT_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        case proto::KEYROLE_SIGN:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::SIGN_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        default:
            break;
    }

    std::shared_ptr<crypto::key::Keypair> newKeypair;
    auto privateKey = OT::App().Crypto().BIP32().GetHDKey(curve, seed, keyPath);

    if (!privateKey) { return newKeypair; }

    privateKey->set_role(role);
    const crypto::EcdsaProvider* engine = nullptr;

    switch (curve) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (EcdsaCurve::SECP256K1): {
            engine = dynamic_cast<const crypto::Secp256k1*>(
                &OT::App().Crypto().SECP256K1());
            break;
        }
#endif
        case (EcdsaCurve::ED25519): {
            engine = dynamic_cast<const crypto::Sodium*>(
                &OT::App().Crypto().ED25519());
            break;
        }
        default: {
        }
    }

    if (nullptr == engine) { return newKeypair; }

    proto::AsymmetricKey publicKey;
    const bool haveKey = engine->PrivateToPublic(*privateKey, publicKey);

    if (haveKey) {
        newKeypair =
            std::make_shared<crypto::key::Keypair>(publicKey, *privateKey);
    }

    return newKeypair;
}
#endif

bool KeyCredential::ReEncryptKeys(
    const OTPassword& theExportPassword,
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

    return bSuccessReEncrypting;  // Note: Caller must re-sign credential after
                                  // doing this,
                                  // to keep these changes.
}

serializedCredential KeyCredential::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = ot_super::serialize(lock, asPrivate, asSigned);

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
    std::shared_ptr<proto::AsymmetricKey> key;
    std::shared_ptr<crypto::key::Keypair> pKey;

    switch (role) {
        case proto::KEYROLE_AUTH:
            pKey = m_AuthentKey;
            break;
        case proto::KEYROLE_ENCRYPT:
            pKey = m_EncryptKey;
            break;
        case proto::KEYROLE_SIGN:
            pKey = m_SigningKey;
            break;
        default:
            return false;
    }

    if (!pKey) { return false; }

    key = pKey->Serialize(getPrivate);

    if (!key) { return false; }

    key->set_role(role);

    auto newKey = credential.add_key();
    *newKey = *key;

    return true;
}

bool KeyCredential::addKeyCredentialtoSerializedCredential(
    serializedCredential credential,
    const bool addPrivate) const
{
    std::unique_ptr<proto::KeyCredential> keyCredential(
        new proto::KeyCredential);

    if (!keyCredential) {
        otErr << __FUNCTION__ << ": failed to allocate keyCredential protobuf."
              << std::endl;

        return false;
    }

    keyCredential->set_version(KEY_CREDENTIAL_VERSION);

    // These must be serialized in this order
    bool auth = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_AUTH);
    bool encrypt = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_ENCRYPT);
    bool sign = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_SIGN);

    if (auth && encrypt && sign) {
        if (addPrivate) {
            keyCredential->set_mode(proto::KEYMODE_PRIVATE);
            credential->set_allocated_privatecredential(
                keyCredential.release());

            return true;
        } else {
            keyCredential->set_mode(proto::KEYMODE_PUBLIC);
            credential->set_allocated_publiccredential(keyCredential.release());

            return true;
        }
    }

    return false;
}

bool KeyCredential::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const proto::KeyRole key) const
{
    const crypto::key::Keypair* keyToUse = nullptr;

    switch (key) {
        case (proto::KEYROLE_AUTH):
            keyToUse = m_AuthentKey.get();
            break;
        case (proto::KEYROLE_SIGN):
            keyToUse = m_SigningKey.get();
            break;
        default:
            otErr << __FUNCTION__ << ": Can not verify signatures with the "
                  << "specified key.\n";
            return false;
    }

    OT_ASSERT(nullptr != keyToUse);

    return keyToUse->Verify(plaintext, sig);
}

bool KeyCredential::SelfSign(
    const OTPassword*,
    const OTPasswordData* pPWData,
    const bool onlyPrivate)
{
    Lock lock(lock_);
    CalculateID(lock);
    SerializedSignature publicSignature = std::make_shared<proto::Signature>();
    SerializedSignature privateSignature = std::make_shared<proto::Signature>();
    bool havePublicSig = false;

    if (!onlyPrivate) {
        const serializedCredential publicVersion =
            serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        auto& signature = *publicVersion->add_signature();
        signature.set_role(proto::SIGROLE_PUBCREDENTIAL);
        havePublicSig =
            SignProto(*publicVersion, signature, proto::KEYROLE_SIGN, pPWData);

        OT_ASSERT(havePublicSig);

        if (havePublicSig) {
            publicSignature->CopyFrom(signature);
            signatures_.push_back(publicSignature);
        }
    }

    serializedCredential privateVersion =
        serialize(lock, AS_PRIVATE, WITHOUT_SIGNATURES);
    auto& signature = *privateVersion->add_signature();
    signature.set_role(proto::SIGROLE_PRIVCREDENTIAL);
    const bool havePrivateSig =
        SignProto(*privateVersion, signature, proto::KEYROLE_SIGN, pPWData);

    OT_ASSERT(havePrivateSig);

    if (havePrivateSig) {
        privateSignature->CopyFrom(signature);
        signatures_.push_back(privateSignature);
    }

    return ((havePublicSig | onlyPrivate) && havePrivateSig);
}

bool KeyCredential::VerifySig(
    const Lock& lock,
    const proto::Signature& sig,
    const CredentialModeFlag asPrivate) const
{
    serializedCredential serialized;

    if ((proto::KEYMODE_PRIVATE != mode_) && asPrivate) {
        otErr << __FUNCTION__ << ": Can not serialize a public credential "
              << "as a private credential.\n";
        return false;
    }

    if (asPrivate) {
        serialized = serialize(lock, AS_PRIVATE, WITHOUT_SIGNATURES);
    } else {
        serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
    }

    auto& signature = *serialized->add_signature();
    signature.CopyFrom(sig);
    signature.clear_signature();
    auto plaintext = proto::ProtoAsData(*serialized);

    return Verify(plaintext, sig);
}

bool KeyCredential::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    OT_ASSERT(m_AuthentKey);

    return m_AuthentKey->TransportKey(publicKey, privateKey);
}

bool KeyCredential::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_MESSAGE): {
            if (m_SigningKey) {
                return m_SigningKey->hasCapability(capability);
            }

            break;
        }
        case (NymCapability::ENCRYPT_MESSAGE): {
            if (m_EncryptKey) {
                return m_EncryptKey->hasCapability(capability);
            }

            break;
        }
        case (NymCapability::AUTHENTICATE_CONNECTION): {
            if (m_AuthentKey) {
                return m_AuthentKey->hasCapability(capability);
            }

            break;
        }
        default: {
        }
    }

    return false;
}
}  // namespace opentxs
