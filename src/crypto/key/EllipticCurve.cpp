// Copyright (c) 2010-2019 The Open-Transactions developers
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
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/Bip32.hpp"
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
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized,
    const PasswordPrompt& reason) noexcept
    : Asymmetric(
          api,
          ecdsa,
          serialized,
          true,
          proto::KEYMODE_PRIVATE == serialized.mode())
    , ecdsa_(ecdsa)
    , key_(Data::Factory())
    , encrypted_key_(extract_key(api, ecdsa, serialized, key_, reason))
{
}

EllipticCurve::EllipticCurve(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(api, ecdsa, keyType, role, version)
    , ecdsa_(ecdsa)
    , key_(Data::Factory())
    , encrypted_key_(std::make_unique<proto::Ciphertext>())
{
    OT_ASSERT(encrypted_key_)

    auto privateKey = OTPassword{};
    const auto generated = ecdsa_.RandomKeypair(privateKey, key_);

    if (false == generated) {
        throw std::runtime_error("Failed to generate key");
    }

    if (false == encrypt_key(api, reason, privateKey, *encrypted_key_)) {
        throw std::runtime_error("Failed to encrypt key");
    }

    has_public_ = true;
    has_private_ = true;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
EllipticCurve::EllipticCurve(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const OTPassword& privateKey,
    const Data& publicKey,
    const proto::KeyRole role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept
    : Asymmetric(api, ecdsa, keyType, role, true, true, version)
    , ecdsa_(ecdsa)
    , key_(publicKey)
    , encrypted_key_(encrypt_key(sessionKey, reason, true, privateKey))
{
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

EllipticCurve::EllipticCurve(const EllipticCurve& rhs) noexcept
    : Asymmetric(rhs)
    , ecdsa_(rhs.ecdsa_)
    , key_(rhs.key_)
    , encrypted_key_(
          bool(rhs.encrypted_key_) ? new proto::Ciphertext(*rhs.encrypted_key_)
                                   : nullptr)
{
}

std::unique_ptr<key::EllipticCurve> EllipticCurve::asPublic(
    const PasswordPrompt& reason) const
{
    std::unique_ptr<EllipticCurve> output{clone_ec()};

    OT_ASSERT(output);

    auto& copy = *output;

    if (false == has_public_) { copy.SetKey(copy.PublicKey(reason)); }

    copy.erase_private_data();

    OT_ASSERT(false == copy.HasPrivate());

    return std::move(output);
}

OTData EllipticCurve::CalculateHash(
    const proto::HashType hashType,
    const PasswordPrompt& password) const
{
    const auto isPrivate = key_->empty();

    if (isPrivate) {
        OTPassword key{};
        OTPassword output{};
        const auto decrypted =
            ECDSA().AsymmetricKeyToECPrivatekey(api_, *this, password, key);

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to extract private key")
                .Flush();

            return Data::Factory();
        }

        const auto hashed = api_.Crypto().Hash().Digest(hashType, key, output);

        if (false == hashed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return Data::Factory();
        }

        return Data::Factory(output.getMemory(), output.getMemorySize());
    } else {
        auto output = Data::Factory();
        const auto hashed = api_.Crypto().Hash().Digest(hashType, key_, output);

        if (false == hashed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return Data::Factory();
        }

        return output;
    }
}

std::unique_ptr<proto::Ciphertext> EllipticCurve::encrypt_key(
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason,
    const bool attach,
    const OTPassword& plaintext) noexcept
{
    auto output = std::make_unique<proto::Ciphertext>();

    if (false == bool(output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct output")
            .Flush();

        return {};
    }

    auto& ciphertext = *output;

    if (encrypt_key(sessionKey, reason, attach, plaintext, ciphertext)) {

        return output;
    } else {

        return {};
    }
}

bool EllipticCurve::encrypt_key(
    const api::internal::Core& api,
    const PasswordPrompt& reason,
    const OTPassword& plaintext,
    proto::Ciphertext& ciphertext) noexcept
{
    auto sessionKey = api.Symmetric().Key(reason);

    return encrypt_key(sessionKey, reason, true, plaintext, ciphertext);
}

bool EllipticCurve::encrypt_key(
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason,
    const bool attach,
    const OTPassword& plaintext,
    proto::Ciphertext& ciphertext) noexcept
{
    auto blankIV = Data::Factory();
    const auto encrypted =
        sessionKey.Encrypt(plaintext, blankIV, reason, ciphertext, attach);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt key").Flush();

        return false;
    }

    return true;
}

void EllipticCurve::erase_private_data()
{
    encrypted_key_.reset();
    has_private_ = false;
}

std::unique_ptr<proto::Ciphertext> EllipticCurve::extract_key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized,
    Data& publicKey,
    const PasswordPrompt& reason)
{
    if (proto::KEYMODE_PUBLIC == serialized.mode()) {
        publicKey.Assign(serialized.key().c_str(), serialized.key().size());

        return {};
    } else if (proto::KEYMODE_PRIVATE == serialized.mode()) {
        auto output =
            std::make_unique<proto::Ciphertext>(serialized.encryptedkey());

        OT_ASSERT(output);

        ecdsa.PrivateToPublic(api, *output, publicKey, reason);

        return output;
    } else {
        OT_FAIL
    }
}

bool EllipticCurve::get_public_key(String& strKey) const
{
    strKey.reset();
    strKey.Set(api_.Crypto().Encode().DataEncode(key_.get()).c_str());

    return true;
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

std::shared_ptr<proto::AsymmetricKey> EllipticCurve::serialize_public(
    EllipticCurve* in)
{
    std::unique_ptr<EllipticCurve> copy{in};

    OT_ASSERT(copy);

    copy->erase_private_data();

    return copy->Serialize();
}

bool EllipticCurve::Open(
    crypto::key::Asymmetric& dhPublic,
    crypto::key::Symmetric& sessionKey,
    PasswordPrompt& sessionKeyPassword,
    const PasswordPrompt& reason) const
{
    if (false == HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Not a private key").Flush();

        return false;
    }

    const auto& engine =
        dynamic_cast<const crypto::EcdsaProvider&>(this->engine());
    const bool haveSessionKey = engine.DecryptSessionKeyECDH(
        api_,
        *this,
        dynamic_cast<crypto::key::EllipticCurve&>(dhPublic),
        sessionKey,
        sessionKeyPassword,
        reason);

    if (false == haveSessionKey) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failed to decrypt session key")
            .Flush();

        return false;
    }

    return true;
}

OTData EllipticCurve::PrivateKey(const PasswordPrompt& reason) const
{
    auto output = Data::Factory();

    if (false == bool(encrypted_key_)) { return Data::Factory(); }

    const auto& privateKey = *encrypted_key_;
    // Private key data and chain code are encrypted to the same session key,
    // and this session key is only embedded in the private key ciphertext
    auto sessionKey =
        api_.Symmetric().Key(privateKey.key(), proto::SMODE_CHACHA20POLY1305);

    if (false == sessionKey.get()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract session key.")
            .Flush();

        return Data::Factory();
    }

    if (false == sessionKey->Decrypt(privateKey, reason, output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt private key")
            .Flush();

        return Data::Factory();
    }

    return output;
}

OTData EllipticCurve::PublicKey(const PasswordPrompt& reason) const
{
    auto output = Data::Factory();

    if (GetKey(output)) { return output; }

    if (false == bool(encrypted_key_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No private key.").Flush();

        return Data::Factory();
    }

    const auto& privateKey = *encrypted_key_;

    if (false == ECDSA().PrivateToPublic(api_, privateKey, output, reason)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate public key")
            .Flush();

        return Data::Factory();
    }

    return output;
}

bool EllipticCurve::Seal(
    const opentxs::api::Core& api,
    OTAsymmetricKey& dhPublic,
    crypto::key::Symmetric& key,
    const PasswordPrompt& reason,
    PasswordPrompt& sessionPassword) const
{
    if (false == HasPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing public key").Flush();

        return false;
    }

    const auto& engine =
        dynamic_cast<const crypto::EcdsaProvider&>(this->engine());
    NymParameters parameters(proto::CREDTYPE_LEGACY);
    parameters.setNymParameterType(CreateType());
    auto dhKeypair = api.Factory().Keypair(
        parameters, version_, proto::KEYROLE_ENCRYPT, reason);
    auto dhRawKey =
        api.Factory().AsymmetricKey(*dhKeypair->GetSerialized(true), reason);
    dhPublic =
        api.Factory().AsymmetricKey(*dhKeypair->GetSerialized(false), reason);
    auto& dhPrivate =
        dynamic_cast<const crypto::key::EllipticCurve&>(dhRawKey.get());
    OTPassword newPassword{};
    const bool haveSessionKey = engine.EncryptSessionKeyECDH(
        api_, dhPrivate, *this, key, sessionPassword, newPassword, reason);

    if (false == haveSessionKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to re-encrypt session key")
            .Flush();

        return false;
    }

    sessionPassword.SetPassword(newPassword);

    return true;
}

std::shared_ptr<proto::AsymmetricKey> EllipticCurve::Serialize() const
{
    auto output = Asymmetric::Serialize();

    OT_ASSERT(output);

    if (HasPrivate()) {
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
    has_public_ = true;
    has_private_ = false;
    key_ = key;

    return true;
}

bool EllipticCurve::SetKey(std::unique_ptr<proto::Ciphertext>& key)
{
    has_public_ = false;
    has_private_ = true;
    encrypted_key_.swap(key);

    return true;
}

bool EllipticCurve::TransportKey(
    Data& publicKey,
    OTPassword& privateKey,
    const PasswordPrompt& reason) const
{
    if (false == HasPrivate()) { return false; }

    if (!encrypted_key_) { return false; }

    OTPassword seed;
    ECDSA().AsymmetricKeyToECPrivatekey(api_, *this, reason, seed);

    return ECDSA().SeedToCurveKey(seed, privateKey, publicKey);
}
}  // namespace opentxs::crypto::key::implementation
