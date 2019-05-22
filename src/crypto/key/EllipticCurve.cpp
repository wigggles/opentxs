// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/HD.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

extern "C" {
#include <sodium/crypto_box.h>
}

#include "EllipticCurve.hpp"

#define OT_METHOD "opentxs::crypto::key::implementation::EllipticCurve::"

namespace opentxs::crypto::key
{
const VersionNumber EllipticCurve::DefaultVersion{2};
const VersionNumber EllipticCurve::MaxVersion{2};
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
EllipticCurve::EllipticCurve(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : Asymmetric(keyType, role, version)
    , key_(Data::Factory())
    , encrypted_key_(nullptr)
{
}

EllipticCurve::EllipticCurve(const proto::AsymmetricKey& serializedKey) noexcept
    : Asymmetric(serializedKey)
    , key_(Data::Factory())
    , encrypted_key_(nullptr)
{
    m_keyType = serializedKey.type();

    if (proto::KEYMODE_PUBLIC == serializedKey.mode()) {
        auto theKey = Data::Factory(
            serializedKey.key().c_str(), serializedKey.key().size());
        SetKey(theKey);
    } else if (proto::KEYMODE_PRIVATE == serializedKey.mode()) {
        std::unique_ptr<proto::Ciphertext> encryptedKey;
        encryptedKey.reset(new proto::Ciphertext(serializedKey.encryptedkey()));

        OT_ASSERT(encryptedKey);

        SetKey(encryptedKey);
    }
}

EllipticCurve::EllipticCurve(
    const proto::AsymmetricKeyType keyType,
    const String& publicKey,
    const VersionNumber version) noexcept
    : EllipticCurve(keyType, proto::KEYROLE_ERROR, version)
{
    m_keyType = proto::AKEYTYPE_SECP256K1;
    auto key = OT::App().Crypto().Encode().DataDecode(publicKey.Get());
    auto dataKey = Data::Factory(key.data(), key.size());
    SetKey(dataKey);
}

OTData EllipticCurve::CalculateHash(
    const proto::HashType hashType,
    const OTPasswordData& password) const
{
    const auto isPrivate = key_->empty();

    if (isPrivate) {
        OTPassword key{};
        OTPassword output{};
        const auto decrypted =
            ECDSA().AsymmetricKeyToECPrivatekey(*this, password, key);

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to extract private key")
                .Flush();

            return Data::Factory();
        }

        const auto hashed =
            OT::App().Crypto().Hash().Digest(hashType, key, output);

        if (false == hashed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return Data::Factory();
        }

        return Data::Factory(output.getMemory(), output.getMemorySize());
    } else {
        auto output = Data::Factory();
        const auto hashed =
            OT::App().Crypto().Hash().Digest(hashType, key_, output);

        if (false == hashed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return Data::Factory();
        }

        return output;
    }
}

EllipticCurve* EllipticCurve::clone() const
{
    auto output = Asymmetric::clone();

    OT_ASSERT(nullptr != output)

    auto* key = dynamic_cast<EllipticCurve*>(output);

    OT_ASSERT(nullptr != key)

    key->key_ = key_;

    if (encrypted_key_) {
        key->encrypted_key_.reset(new proto::Ciphertext(*encrypted_key_));
    }

    return key;
}

bool EllipticCurve::GetKey(Data& key) const
{
    if (key_->empty()) { return false; }

    key.Assign(key_.get());

    return true;
}

bool EllipticCurve::GetKey(proto::Ciphertext& key) const
{
    if (encrypted_key_) {
        key.CopyFrom(*encrypted_key_);

        return true;
    }

    return false;
}

bool EllipticCurve::GetPublicKey(String& strKey) const
{
    strKey.reset();
    strKey.Set(OT::App().Crypto().Encode().DataEncode(key_.get()).c_str());

    return true;
}

bool EllipticCurve::GetPublicKey(Data& key) const
{
    if (false == key_->empty()) {
        key.Assign(key_->data(), key_->size());

        return true;
    }

    if (false == bool(encrypted_key_)) { return false; }

    return ECDSA().PrivateToPublic(*encrypted_key_, key);
}

bool EllipticCurve::IsEmpty() const { return key_->empty(); }

bool EllipticCurve::Open(
    crypto::key::Asymmetric& dhPublic,
    crypto::key::Symmetric& sessionKey,
    OTPasswordData& password) const
{
    if (false == IsPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type (public)")
            .Flush();

        return false;
    }

    const auto& engine =
        dynamic_cast<const crypto::EcdsaProvider&>(this->engine());
    OTPassword plaintextKey{};
    const bool haveSessionKey = engine.DecryptSessionKeyECDH(
        *this,
        dynamic_cast<crypto::key::EllipticCurve&>(dhPublic),
        password,
        sessionKey,
        plaintextKey);

    if (false == haveSessionKey) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failed to decrypt session key")
            .Flush();

        return false;
    }

    password.SetOverride(plaintextKey);

    return true;
}

OTData EllipticCurve::PrivateKey() const
{
    auto output = Data::Factory();

    if (false == bool(encrypted_key_)) { return Data::Factory(); }

    const auto& privateKey = *encrypted_key_;
    // Private key data and chain code are encrypted to the same session key,
    // and this session key is only embedded in the private key ciphertext
    auto sessionKey = OT::App().Crypto().Symmetric().Key(
        privateKey.key(), proto::SMODE_CHACHA20POLY1305);

    if (false == sessionKey.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract session key.")
            .Flush();

        return Data::Factory();
    }

    if (false == sessionKey->Decrypt(privateKey, __FUNCTION__, output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt private key")
            .Flush();

        return Data::Factory();
    }

    return output;
}

OTData EllipticCurve::PublicKey() const
{
    auto output = Data::Factory();

    if (false == bool(encrypted_key_)) { return Data::Factory(); }

    const auto& privateKey = *encrypted_key_;

    if (false == ECDSA().PrivateToPublic(privateKey, output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate public key")
            .Flush();

        return Data::Factory();
    }

    return output;
}

bool EllipticCurve::ReEncryptPrivateKey(
    const OTPassword& theExportPassword,
    bool bImporting) const
{
    OT_ASSERT(IsPrivate());

    bool bReturnVal = false;

    if (!IsEmpty() > 0) {
        OTPassword pClearKey;
        bool haveClearKey = false;

        // Here's thePWData we use if we didn't have anything else:
        OTPasswordData thePWData(
            bImporting ? "(Importing) Enter the exported Nym's passphrase."
                       : "(Exporting) Enter your wallet's master passphrase.");

        // If we're importing, that means we're currently stored as an EXPORTED
        // NYM (i.e. with its own password, independent of the wallet.) So we
        // use theExportedPassword.
        if (bImporting) {
            thePWData.SetOverride(theExportPassword);
            haveClearKey = ECDSA().ImportECPrivatekey(
                *encrypted_key_, thePWData, pClearKey);
        }
        // Else if we're exporting, that means we're currently stored in the
        // wallet (i.e. using the wallet's cached master key.) So we use the
        // normal password callback.
        else {
            haveClearKey = ECDSA().AsymmetricKeyToECPrivatekey(
                *this, thePWData, pClearKey);
        }

        if (haveClearKey) {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Success decrypting private key.")
                .Flush();

            // Okay, we have loaded up the private key, now let's save it
            // using the new passphrase.

            // If we're importing, that means we just loaded up the (previously)
            // exported Nym using theExportedPassphrase, so now we need to save
            // it again using the normal password callback (for importing it to
            // the wallet.)

            bool reencrypted = false;

            if (bImporting) {
                thePWData.ClearOverride();
                reencrypted = ECDSA().ECPrivatekeyToAsymmetricKey(
                    pClearKey, thePWData, *const_cast<EllipticCurve*>(this));
            }

            // Else if we're exporting, that means we just loaded up the Nym
            // from the wallet using the normal password callback, and now we
            // need to save it back again using theExportedPassphrase (for
            // exporting it outside of the wallet.)
            else {
                thePWData.SetOverride(theExportPassword);
                reencrypted = ECDSA().ExportECPrivatekey(
                    pClearKey, thePWData, *const_cast<EllipticCurve*>(this));
            }

            if (!reencrypted) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Could not encrypt private key.")
                    .Flush();
            }

            bReturnVal = reencrypted;

        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Could not decrypt private key.")
                .Flush();
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Key is empty.").Flush();
    }

    return bReturnVal;
}

bool EllipticCurve::Seal(
    const opentxs::api::Core& api,
    OTAsymmetricKey& dhPublic,
    crypto::key::Symmetric& key,
    OTPasswordData& password) const
{
    if (false == IsPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type (private)")
            .Flush();

        return false;
    }

    const auto& engine =
        dynamic_cast<const crypto::EcdsaProvider&>(this->engine());
    NymParameters parameters(proto::CREDTYPE_LEGACY);
    parameters.setNymParameterType(CreateType());
    auto dhKeypair =
        api.Factory().Keypair(parameters, version_, proto::KEYROLE_ENCRYPT);
    auto dhRawKey = api.Factory().AsymmetricKey(*dhKeypair->Serialize(true));
    dhPublic = api.Factory().AsymmetricKey(*dhKeypair->Serialize(false));
    auto& dhPrivate =
        dynamic_cast<const crypto::key::EllipticCurve&>(dhRawKey.get());
    OTPassword newPassword{};
    const bool haveSessionKey = engine.EncryptSessionKeyECDH(
        dhPrivate, *this, password, key, newPassword);

    if (false == haveSessionKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to re-encrypt session key")
            .Flush();

        return false;
    }

    password.SetOverride(newPassword);

    return true;
}

std::shared_ptr<proto::AsymmetricKey> EllipticCurve::Serialize() const
{
    auto output = Asymmetric::Serialize();

    OT_ASSERT(output);

    if (IsPrivate()) {
        output->set_mode(proto::KEYMODE_PRIVATE);

        if (encrypted_key_) {
            *output->mutable_encryptedkey() = *encrypted_key_;
        }
    } else {
        output->set_mode(proto::KEYMODE_PUBLIC);

        if (false == key_->empty()) {
            output->set_key(key_->data(), key_->size());
        }
    }

    return output;
}

bool EllipticCurve::SetKey(const Data& key)
{
    m_bIsPublicKey = true;
    m_bIsPrivateKey = false;
    key_ = key;

    return true;
}

bool EllipticCurve::SetKey(std::unique_ptr<proto::Ciphertext>& key)
{
    m_bIsPublicKey = false;
    m_bIsPrivateKey = true;
    encrypted_key_.swap(key);

    return true;
}

bool EllipticCurve::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    if (!IsPrivate()) { return false; }

    if (!encrypted_key_) { return false; }

    OTPassword seed;
    ECDSA().AsymmetricKeyToECPrivatekey(*this, "Get transport key", seed);

    return ECDSA().SeedToCurveKey(seed, privateKey, publicKey);
}
}  // namespace opentxs::crypto::key::implementation
