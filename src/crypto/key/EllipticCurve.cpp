// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
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
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized) noexcept
    : Asymmetric(
          crypto,
          ecdsa,
          serialized,
          true,
          proto::KEYMODE_PRIVATE == serialized.mode())
    , ecdsa_(ecdsa)
    , key_(Data::Factory())
    , encrypted_key_(extract_key(ecdsa, serialized, key_))
{
}

EllipticCurve::EllipticCurve(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : Asymmetric(crypto, ecdsa, keyType, role, version)
    , ecdsa_(ecdsa)
    , key_(Data::Factory())
    , encrypted_key_(nullptr)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
EllipticCurve::EllipticCurve(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKeyType keyType,
    const OTPassword& privateKey,
    const Data& publicKey,
    const proto::KeyRole role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const OTPasswordData& reason) noexcept
    : Asymmetric(crypto, ecdsa, keyType, role, true, true, version)
    , ecdsa_(ecdsa)
    , key_(publicKey)
    , encrypted_key_(encrypt_key(sessionKey, reason, true, privateKey))
{
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

EllipticCurve::EllipticCurve(const EllipticCurve& rhs) noexcept
    : key::EllipticCurve()
    , Asymmetric(rhs)
    , ecdsa_(rhs.ecdsa_)
    , key_(rhs.key_)
    , encrypted_key_(
          bool(rhs.encrypted_key_) ? new proto::Ciphertext(*rhs.encrypted_key_)
                                   : nullptr)
{
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

        const auto hashed = crypto_.Hash().Digest(hashType, key, output);

        if (false == hashed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate hash")
                .Flush();

            return Data::Factory();
        }

        return Data::Factory(output.getMemory(), output.getMemorySize());
    } else {
        auto output = Data::Factory();
        const auto hashed = crypto_.Hash().Digest(hashType, key_, output);

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
    const OTPasswordData& reason,
    const bool attach,
    const OTPassword& plaintext)
{
    auto output = std::make_unique<proto::Ciphertext>();

    if (false == bool(output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to construct output")
            .Flush();

        return {};
    }

    auto& ciphertext = *output;
    auto blankIV = Data::Factory();
    const auto encrypted =
        sessionKey.Encrypt(plaintext, blankIV, reason, ciphertext, attach);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt key").Flush();

        return {};
    }

    return output;
}

void EllipticCurve::erase_private_data()
{
    encrypted_key_.reset();
    has_private_ = false;
}

std::unique_ptr<proto::Ciphertext> EllipticCurve::extract_key(
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized,
    Data& publicKey)
{
    if (proto::KEYMODE_PUBLIC == serialized.mode()) {
        publicKey.Assign(serialized.key().c_str(), serialized.key().size());

        return {};
    } else if (proto::KEYMODE_PRIVATE == serialized.mode()) {
        auto output =
            std::make_unique<proto::Ciphertext>(serialized.encryptedkey());

        OT_ASSERT(output);

        ecdsa.PrivateToPublic(*output, publicKey);

        return output;
    } else {
        OT_FAIL
    }
}

bool EllipticCurve::get_public_key(String& strKey) const
{
    strKey.reset();
    strKey.Set(crypto_.Encode().DataEncode(key_.get()).c_str());

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

const key::Asymmetric& EllipticCurve::GetPrivateKey() const
{
    if (false == has_private_) { throw std::out_of_range("No private key"); }

    return *this;
}

const key::Asymmetric& EllipticCurve::GetPublicKey() const
{
    if (false == has_public_) { throw std::out_of_range("No public key"); }

    return *this;
}

std::int32_t EllipticCurve::GetPublicKeyBySignature(
    Keys& listOutput,
    const Signature& theSignature,
    bool bInclusive) const
{
    OT_ASSERT(has_public_);

    const auto* metadata = GetMetadata();

    OT_ASSERT(nullptr != metadata);

    if ((false == bInclusive) &&
        (false == theSignature.getMetaData().HasMetadata())) {

        return 0;
    }

    if (!theSignature.getMetaData().HasMetadata() || !metadata->HasMetadata() ||
        (metadata->HasMetadata() && theSignature.getMetaData().HasMetadata() &&
         (theSignature.getMetaData() == *(metadata)))) {
        listOutput.push_back(this);

        return 1;
    }

    return 0;
}

std::shared_ptr<proto::AsymmetricKey> EllipticCurve::GetSerialized(
    bool privateKey) const
{
    if (privateKey) {
        if (false == has_private_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Not a private key").Flush();

            return {};
        }
    } else {
        if (false == has_public_) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Not a public key").Flush();

            return {};
        }

        if (has_private_) { return get_public(); }
    }

    return Serialize();
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
    OTPasswordData& password) const
{
    if (false == HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Not a private key").Flush();

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
    auto sessionKey = crypto_.Symmetric().Key(
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

bool EllipticCurve::Seal(
    const opentxs::api::Core& api,
    OTAsymmetricKey& dhPublic,
    crypto::key::Symmetric& key,
    OTPasswordData& password) const
{
    if (false == HasPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing public key").Flush();

        return false;
    }

    const auto& engine =
        dynamic_cast<const crypto::EcdsaProvider&>(this->engine());
    NymParameters parameters(proto::CREDTYPE_LEGACY);
    parameters.setNymParameterType(CreateType());
    auto dhKeypair =
        api.Factory().Keypair(parameters, version_, proto::KEYROLE_ENCRYPT);
    auto dhRawKey =
        api.Factory().AsymmetricKey(*dhKeypair->GetSerialized(true));
    dhPublic = api.Factory().AsymmetricKey(*dhKeypair->GetSerialized(false));
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

bool EllipticCurve::TransportKey(Data& publicKey, OTPassword& privateKey) const
{
    if (false == HasPrivate()) { return false; }

    if (!encrypted_key_) { return false; }

    OTPassword seed;
    ECDSA().AsymmetricKeyToECPrivatekey(*this, "Get transport key", seed);

    return ECDSA().SeedToCurveKey(seed, privateKey, publicKey);
}
}  // namespace opentxs::crypto::key::implementation
