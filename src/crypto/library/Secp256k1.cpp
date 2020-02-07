// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/Secp256k1.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/Proto.hpp"

#include "AsymmetricProvider.hpp"
#include "EcdsaProvider.hpp"

extern "C" {
#include "secp256k1.h"
#include "secp256k1_ecdh.h"
}

#include <array>
#include <cstdint>
#include <ostream>

#include "Secp256k1.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Secp256k1::"

namespace opentxs
{
crypto::Secp256k1* Factory::Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& util)
{
    return new crypto::implementation::Secp256k1(crypto, util);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
bool Secp256k1::Initialized_ = false;

Secp256k1::Secp256k1(const api::Crypto& crypto, const api::crypto::Util& ssl)
    : EcdsaProvider(crypto)
    , context_(secp256k1_context_create(
          SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY))
    , ssl_(ssl)
{
}

bool Secp256k1::RandomKeypair(
    const AllocateOutput privateKey,
    const AllocateOutput publicKey,
    const proto::KeyRole,
    const NymParameters&,
    const AllocateOutput) const noexcept
{
    if (nullptr == context_) { return false; }

    if (false == bool(privateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    auto counter{0};
    auto valid{false};
    auto temp = OTPassword{};
    const auto null = std::array<std::uint8_t, PrivateKeySize>{};

    while (false == valid) {
        temp.randomizeMemory(PrivateKeySize);

        OT_ASSERT(temp.getMemorySize() == PrivateKeySize);

        // We add the random key to a zero value key because
        // secp256k1_privkey_tweak_add checks the result to make sure it's in
        // the correct range for secp256k1.
        //
        // This loop should almost always run exactly one time (about 1/(2^128)
        // chance of randomly generating an invalid key thus requiring a second
        // attempt)
        valid = secp256k1_ec_privkey_tweak_add(
            context_,
            static_cast<unsigned char*>(temp.getMemoryWritable()),
            null.data());

        OT_ASSERT(3 > ++counter);
    }

    auto prv = privateKey(PrivateKeySize);

    if (false == prv.valid(PrivateKeySize)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for private key")
            .Flush();

        return false;
    }

    std::memcpy(prv, temp.getMemory(), prv);

    return ScalarMultiplyBase({prv.as<const char>(), prv.size()}, publicKey);
}

bool Secp256k1::ScalarAdd(
    const ReadView lhs,
    const ReadView rhs,
    const AllocateOutput result) const noexcept
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    if (PrivateKeySize != lhs.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid lhs scalar").Flush();

        return false;
    }

    if (PrivateKeySize != rhs.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid rhs scalar").Flush();

        return false;
    }

    auto key = result(PrivateKeySize);

    if (false == key.valid(PrivateKeySize)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for result")
            .Flush();

        return false;
    }

    std::memcpy(key.data(), lhs.data(), lhs.size());

    return 1 == ::secp256k1_ec_privkey_tweak_add(
                    context_,
                    key.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(rhs.data()));
}

auto Secp256k1::ScalarMultiplyBase(
    const ReadView scalar,
    const AllocateOutput result) const noexcept -> bool
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    if (PrivateKeySize != scalar.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid scalar").Flush();

        return false;
    }

    auto key = secp256k1_pubkey{};
    const auto created =
        1 == ::secp256k1_ec_pubkey_create(
                 context_,
                 &key,
                 reinterpret_cast<const unsigned char*>(scalar.data()));

    if (1 != created) { return false; }

    auto pub = result(PublicKeySize);

    if (false == pub.valid(PublicKeySize)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    auto size{pub.size()};

    return 1 == ::secp256k1_ec_pubkey_serialize(
                    context_,
                    pub.as<unsigned char>(),
                    &size,
                    &key,
                    SECP256K1_EC_COMPRESSED);
}

auto Secp256k1::SharedSecret(
    const key::Asymmetric& publicKey,
    const key::Asymmetric& privateKey,
    const PasswordPrompt& reason,
    OTPassword& secret) const noexcept -> bool
{
    if (publicKey.keyType() != proto::AKEYTYPE_SECP256K1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Public key is wrong type")
            .Flush();

        return false;
    }

    if (privateKey.keyType() != proto::AKEYTYPE_SECP256K1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Private key is wrong type")
            .Flush();

        return false;
    }

    const auto pub = publicKey.PublicKey();
    const auto prv = privateKey.PrivateKey(reason);
    static const auto blank = std::array<std::byte, 32>{};

    if (32 != prv.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid private key").Flush();

        return false;
    }

    auto key = ::secp256k1_pubkey{};

    if (1 != ::secp256k1_ec_pubkey_parse(
                 context_,
                 &key,
                 reinterpret_cast<const unsigned char*>(pub.data()),
                 pub.size())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key").Flush();

        return false;
    }

    secret.setMemory(blank.data(), blank.size());

    OT_ASSERT(32 == secret.getMemorySize());

    return 1 == ::secp256k1_ecdh(
                    context_,
                    static_cast<unsigned char*>(secret.getMemoryWritable()),
                    &key,
                    reinterpret_cast<const unsigned char*>(prv.data()));
}

bool Secp256k1::Sign(
    const api::internal::Core& api,
    const Data& plaintext,
    const key::Asymmetric& key,
    const proto::HashType type,
    Data& signature,  // output
    const PasswordPrompt& reason,
    const OTPassword* exportPassword) const
{
    if (proto::AKEYTYPE_SECP256K1 != key.keyType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key type").Flush();

        return false;
    }

    if (false == key.HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": A private key required when generating signatures")
            .Flush();

        return false;
    }

    try {
        const auto digest = hash(type, plaintext);
        const auto priv = key.PrivateKey(reason);

        if (nullptr == priv.data() || 0 == priv.size()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Missing private key").Flush();

            return false;
        }

        if (PrivateKeySize != priv.size()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid private key").Flush();

            return false;
        }

        auto ecdsaSignature = ::secp256k1_ecdsa_signature{};
        const bool signatureCreated = ::secp256k1_ecdsa_sign(
            context_,
            &ecdsaSignature,
            reinterpret_cast<const unsigned char*>(digest->data()),
            reinterpret_cast<const unsigned char*>(priv.data()),
            nullptr,
            nullptr);

        if (signatureCreated) {
            signature.Assign(ecdsaSignature.data, sizeof(ecdsaSignature));

            return true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Call to secp256k1_ecdsa_sign() failed.")
                .Flush();

            return false;
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }
}

bool Secp256k1::Verify(
    const Data& plaintext,
    const key::Asymmetric& key,
    const Data& signature,
    const proto::HashType type) const
{
    if (proto::AKEYTYPE_SECP256K1 != key.keyType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key type").Flush();

        return false;
    }

    try {
        const auto digest = hash(type, plaintext);
        const auto parsed = parsed_public_key(key.PublicKey());
        const auto sig = parsed_signature(signature.Bytes());

        return 1 == ::secp256k1_ecdsa_verify(
                        context_,
                        &sig,
                        reinterpret_cast<const unsigned char*>(digest->data()),
                        &parsed);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return false;
    }
}

auto Secp256k1::hash(const proto::HashType type, const Data& data) const
    noexcept(false) -> OTData
{
    auto output = Data::Factory();

    if (false ==
        crypto_.Hash().Digest(type, data.Bytes(), output->WriteInto())) {
        throw std::runtime_error("Failed to obtain contract hash");
    }

    if (0 == output->size()) { throw std::runtime_error("Invalid hash"); }

    output->resize(32);

    OT_ASSERT(nullptr != output->data());
    OT_ASSERT(32 == output->size());

    return output;
}

void Secp256k1::Init()
{
    OT_ASSERT(false == Initialized_);

    auto seed = std::array<std::uint8_t, 32>{};
    ssl_.RandomizeMemory(seed.data(), seed.size());

    OT_ASSERT(nullptr != context_);

    const auto randomize = secp256k1_context_randomize(context_, seed.data());

    OT_ASSERT(1 == randomize);

    Initialized_ = true;
}

auto Secp256k1::parsed_public_key(const ReadView bytes) const noexcept(false)
    -> ::secp256k1_pubkey
{
    if (nullptr == bytes.data() || 0 == bytes.size()) {
        throw std::runtime_error("Missing public key");
    }

    auto output = ::secp256k1_pubkey{};

    if (1 != ::secp256k1_ec_pubkey_parse(
                 context_,
                 &output,
                 reinterpret_cast<const unsigned char*>(bytes.data()),
                 bytes.size())) {
        throw std::runtime_error("Invalid public key");
    }

    return output;
}

auto Secp256k1::parsed_signature(const ReadView bytes) const noexcept(false)
    -> ::secp256k1_ecdsa_signature
{
    auto output = ::secp256k1_ecdsa_signature{};

    if (sizeof(output.data) != bytes.size()) {
        throw std::runtime_error("Invalid signature");
    }

    std::memcpy(&output.data, bytes.data(), sizeof(output.data));

    return output;
}

Secp256k1::~Secp256k1()
{
    if (nullptr != context_) {
        secp256k1_context_destroy(context_);
        context_ = nullptr;
    }

    Initialized_ = false;
}
}  // namespace opentxs::crypto::implementation
#endif
