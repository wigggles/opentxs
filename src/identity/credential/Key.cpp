// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/HD.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/Sodium.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <ostream>

#include "Key.hpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Key::"

namespace opentxs::identity::credential::implementation
{
const VersionConversionMap Key::credential_subversion_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};
const VersionConversionMap Key::subversion_to_key_version_{
    {1, 1},
    {2, 2},
};

Key::Key(
    const api::Core& api,
    identity::internal::Authority& theOwner,
    const proto::Credential& serialized) noexcept
    : Signable({}, serialized.version())  // TODO Signable
    , credential::implementation::Base(api, theOwner, serialized)
    , subversion_(credential_subversion_.at(version_))
    , signing_key_(deserialize_key(api, proto::KEYROLE_SIGN, serialized))
    , authentication_key_(deserialize_key(api, proto::KEYROLE_AUTH, serialized))
    , encryption_key_(deserialize_key(api, proto::KEYROLE_ENCRYPT, serialized))
{
}

Key::Key(
    const api::Core& api,
    identity::internal::Authority& theOwner,
    const NymParameters& nymParameters,
    const VersionNumber version) noexcept
    : Signable({}, version)  // TODO Signable
    , credential::implementation::Base(api, theOwner, nymParameters, version)
    , subversion_(credential_subversion_.at(version_))
    , signing_key_(new_key(
          api_,
          proto::KEYROLE_SIGN,
          nymParameters,
          subversion_to_key_version_.at(subversion_)))
    , authentication_key_(new_key(
          api_,
          proto::KEYROLE_AUTH,
          nymParameters,
          subversion_to_key_version_.at(subversion_)))
    , encryption_key_(new_key(
          api_,
          proto::KEYROLE_ENCRYPT,
          nymParameters,
          subversion_to_key_version_.at(subversion_)))
{
    OT_ASSERT(0 != version);
}

bool Key::VerifySignedBySelf(const Lock& lock) const
{
    auto publicSig = SelfSignature(PUBLIC_VERSION);

    if (!publicSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Could not find public self signature.")
            .Flush();

        return false;
    }

    bool goodPublic = VerifySig(lock, *publicSig, PUBLIC_VERSION);

    if (!goodPublic) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Could not verify public self signature.")
            .Flush();

        return false;
    }

    if (Private()) {
        auto privateSig = SelfSignature(PRIVATE_VERSION);

        if (!privateSig) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Could not find private self signature.")
                .Flush();

            return false;
        }

        bool goodPrivate = VerifySig(lock, *privateSig, PRIVATE_VERSION);

        if (!goodPrivate) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Could not verify private self signature.")
                .Flush();

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
std::int32_t Key::GetPublicKeysBySignature(
    crypto::key::Keypair::Keys& listOutput,
    const Signature& theSignature,
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

    switch (cKeyType) {
        // Specific search only for signatures with metadata.
        // FYI, theSignature.getMetaData().HasMetadata() is true, in this case.
        case '0': {
            // That's why I can just assume theSignature has a key type here:
            switch (theSignature.getMetaData().GetKeyType()) {
                case 'A':
                    nCount = authentication_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'E':
                    nCount = encryption_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'S':
                    nCount = signing_key_->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                default:
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unexpected keytype value in signature metadata: ")(
                        theSignature.getMetaData().GetKeyType())(" (Failure)!")
                        .Flush();
                    return 0;
            }
            break;
        }
        // Generalized search which specifies key type and returns keys
        // even for signatures with no metadata. (When metadata is present,
        // it's still used to eliminate keys.)
        case 'A':
            nCount = authentication_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'E':
            nCount = encryption_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'S':
            nCount = signing_key_->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unexpected value for cKeyType (should be 0, A, E, or S): ")(
                cKeyType)(".")
                .Flush();
            return 0;
    }
    return nCount;
}

bool Key::verify_internally(const Lock& lock) const
{
    // Perform common Credential verifications
    if (!Base::verify_internally(lock)) { return false; }

    // All KeyCredentials must sign themselves
    if (!VerifySignedBySelf(lock)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying key credential: it's not "
            "signed by itself (its own signing key).")
            .Flush();
        return false;
    }

    return true;
}

OTKeypair Key::deserialize_key(
    const api::Core& api,
    const int index,
    const proto::Credential& credential)
{
    const bool hasPrivate =
        (proto::KEYMODE_PRIVATE == credential.mode()) ? true : false;

    const auto publicKey = credential.publiccredential().key(index - 1);

    if (hasPrivate) {
        const auto privateKey = credential.privatecredential().key(index - 1);

        return api.Factory().Keypair(publicKey, privateKey);
    }

    return api.Factory().Keypair(publicKey);
}

OTKeypair Key::new_key(
    const api::Core& api,
    const proto::KeyRole role,
    const NymParameters& nymParameters,
    const VersionNumber version)
{
    if (proto::CREDTYPE_HD != nymParameters.credentialType()) {

        return api.Factory().Keypair(nymParameters, version, role);
    }

#if OT_CRYPTO_SUPPORTED_KEY_HD
    const auto keyType = nymParameters.AsymmetricKeyType();
    const auto curve = crypto::AsymmetricProvider::KeyTypeToCurve(keyType);

    OT_ASSERT(EcdsaCurve::ERROR != curve)
    OT_ASSERT(nymParameters.Entropy())

    return derive_hd_keypair(
        api,
        *nymParameters.Entropy(),
        nymParameters.Seed(),
        nymParameters.Nym(),
        nymParameters.Credset(),
        nymParameters.CredIndex(),
        curve,
        role,
        version);
#else
    OT_FAIL
#endif
}

bool Key::New(const NymParameters& nymParameters)
{
    bool output = false;

    output = Base::New(nymParameters);

    if (output) {
        output = SelfSign();
    } else {
        OT_FAIL;
    }

    OT_ASSERT(output);

    return output;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
OTKeypair Key::derive_hd_keypair(
    const api::Core& api,
    const OTPassword& seed,
    const std::string& fingerprint,
    const Bip32Index nym,
    const Bip32Index credset,
    const Bip32Index credindex,
    const EcdsaCurve& curve,
    const proto::KeyRole role,
    const VersionNumber version)
{
    std::string input(fingerprint);
    Bip32Index roleIndex{0};

    switch (role) {
        case proto::KEYROLE_AUTH: {
            roleIndex = HDIndex{Bip32Child::AUTH_KEY, Bip32Child::HARDENED};
        } break;
        case proto::KEYROLE_ENCRYPT: {
            roleIndex = HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED};
        } break;
        case proto::KEYROLE_SIGN: {
            roleIndex = HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED};
        } break;
        default: {
            OT_FAIL
        }
    }

    const api::HDSeed::Path path{
        HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
        HDIndex{nym, Bip32Child::HARDENED},
        HDIndex{credset, Bip32Child::HARDENED},
        HDIndex{credindex, Bip32Child::HARDENED},
        roleIndex};

    auto pPrivateKey = api.Seeds().GetHDKey(input, curve, path, role);

    OT_ASSERT(pPrivateKey)

    auto& privateKey = *pPrivateKey;
    const auto pSerialized = privateKey.Serialize();

    OT_ASSERT(pSerialized);

    const auto& serialized = *pSerialized;
    proto::AsymmetricKey publicKey;
    const bool haveKey =
        privateKey.ECDSA().PrivateToPublic(serialized, publicKey);

    OT_ASSERT(haveKey)

    return api.Factory().Keypair(publicKey, serialized);
}
#endif

bool Key::ReEncryptKeys(const OTPassword& theExportPassword, bool bImporting)
{
    const bool bSign = signing_key_->ReEncrypt(theExportPassword, bImporting);
    const bool bAuth =
        authentication_key_->ReEncrypt(theExportPassword, bImporting);
    const bool bEncr =
        encryption_key_->ReEncrypt(theExportPassword, bImporting);

    const bool bSuccessReEncrypting = (bSign && bAuth && bEncr);
    OT_ASSERT(bSuccessReEncrypting);

    return bSuccessReEncrypting;  // Note: Caller must re-sign credential after
                                  // doing this,
                                  // to keep these changes.
}

std::shared_ptr<Base::SerializedType> Key::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = Base::serialize(lock, asPrivate, asSigned);

    addKeyCredentialtoSerializedCredential(serializedCredential, false);

    if (asPrivate) {
        addKeyCredentialtoSerializedCredential(serializedCredential, true);
    }

    return serializedCredential;
}

bool Key::addKeytoSerializedKeyCredential(
    proto::KeyCredential& credential,
    const bool getPrivate,
    const proto::KeyRole role) const
{
    std::shared_ptr<proto::AsymmetricKey> key{nullptr};
    const crypto::key::Keypair* pKey{nullptr};

    switch (role) {
        case proto::KEYROLE_AUTH:
            pKey = &authentication_key_.get();
            break;
        case proto::KEYROLE_ENCRYPT:
            pKey = &encryption_key_.get();
            break;
        case proto::KEYROLE_SIGN:
            pKey = &signing_key_.get();
            break;
        default:
            return false;
    }

    if (nullptr == pKey) { return false; }

    key = pKey->Serialize(getPrivate);

    if (!key) { return false; }

    key->set_role(role);

    auto newKey = credential.add_key();
    *newKey = *key;

    return true;
}

bool Key::addKeyCredentialtoSerializedCredential(
    std::shared_ptr<Base::SerializedType> credential,
    const bool addPrivate) const
{
    std::unique_ptr<proto::KeyCredential> keyCredential(
        new proto::KeyCredential);

    if (!keyCredential) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate keyCredential protobuf.")
            .Flush();

        return false;
    }

    keyCredential->set_version(subversion_);

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

bool Key::Verify(
    const Data& plaintext,
    const proto::Signature& sig,
    const proto::KeyRole key) const
{
    const crypto::key::Keypair* keyToUse = nullptr;

    switch (key) {
        case (proto::KEYROLE_AUTH):
            keyToUse = &authentication_key_.get();
            break;
        case (proto::KEYROLE_SIGN):
            keyToUse = &signing_key_.get();
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Can not verify signatures with the "
                "specified key.")
                .Flush();
            return false;
    }

    OT_ASSERT(nullptr != keyToUse);

    return keyToUse->Verify(plaintext, sig);
}

bool Key::SelfSign(
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
        const auto publicVersion =
            serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        auto& signature = *publicVersion->add_signature();
        havePublicSig = Sign(
            [&]() -> std::string {
                return proto::ProtoAsString(*publicVersion);
            },
            proto::SIGROLE_PUBCREDENTIAL,
            signature,
            proto::KEYROLE_SIGN,
            pPWData);

        OT_ASSERT(havePublicSig);

        if (havePublicSig) {
            publicSignature->CopyFrom(signature);
            signatures_.push_back(publicSignature);
        }
    }

    auto privateVersion = serialize(lock, AS_PRIVATE, WITHOUT_SIGNATURES);
    auto& signature = *privateVersion->add_signature();
    const bool havePrivateSig = Sign(
        [&]() -> std::string { return proto::ProtoAsString(*privateVersion); },
        proto::SIGROLE_PRIVCREDENTIAL,
        signature,
        proto::KEYROLE_SIGN,
        pPWData);

    OT_ASSERT(havePrivateSig);

    if (havePrivateSig) {
        privateSignature->CopyFrom(signature);
        signatures_.push_back(privateSignature);
    }

    return ((havePublicSig | onlyPrivate) && havePrivateSig);
}

bool Key::VerifySig(
    const Lock& lock,
    const proto::Signature& sig,
    const CredentialModeFlag asPrivate) const
{
    std::shared_ptr<Base::SerializedType> serialized;

    if ((proto::KEYMODE_PRIVATE != mode_) && asPrivate) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not serialize a public credential "
            "as a private credential.")
            .Flush();
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

bool Key::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    return authentication_key_->TransportKey(publicKey, privateKey);
}

bool Key::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_MESSAGE): {
            return signing_key_->hasCapability(capability);
        }
        case (NymCapability::ENCRYPT_MESSAGE): {
            return encryption_key_->hasCapability(capability);
        }
        case (NymCapability::AUTHENTICATE_CONNECTION): {
            return authentication_key_->hasCapability(capability);
        }
        default: {
        }
    }

    return false;
}

const crypto::key::Keypair& Key::GetKeypair(
    const proto::AsymmetricKeyType type,
    const proto::KeyRole role) const
{
    const crypto::key::Keypair* output{nullptr};

    switch (role) {
        case proto::KEYROLE_AUTH: {
            output = &authentication_key_.get();
        } break;
        case proto::KEYROLE_ENCRYPT: {
            output = &encryption_key_.get();
        } break;
        case proto::KEYROLE_SIGN: {
            output = &signing_key_.get();
        } break;
        default: {
            throw std::out_of_range("wrong key type");
        }
    }

    OT_ASSERT(nullptr != output);

    if (proto::AKEYTYPE_NULL != type) {
        if (type != output->GetPublicKey().keyType()) {
            throw std::out_of_range("wrong key type");
        }
    }

    return *output;
}

bool Key::Sign(
    const GetPreimage input,
    const proto::SignatureRole role,
    proto::Signature& signature,
    proto::KeyRole key,
    const OTPasswordData* pPWData,
    const proto::HashType hash) const
{
    const crypto::key::Keypair* keyToUse{nullptr};

    switch (key) {
        case (proto::KEYROLE_AUTH):
            keyToUse = &authentication_key_.get();
            break;
        case (proto::KEYROLE_SIGN):
            keyToUse = &signing_key_.get();
            break;
        case (proto::KEYROLE_ERROR):
        case (proto::KEYROLE_ENCRYPT):
        default:
            LogOutput(": Can not sign with the "
                      "specified key.")
                .Flush();
            return false;
    }

    if (nullptr != keyToUse) {
        return keyToUse->Sign(input, role, signature, id_, key, pPWData, hash);
    }

    return false;
}
}  // namespace opentxs::identity::credential::implementation
