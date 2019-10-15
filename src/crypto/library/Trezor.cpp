// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_TREZOR
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/Trezor.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "crypto/Bip32.hpp"
#endif
#include "AsymmetricProvider.hpp"
#include "EcdsaProvider.hpp"

extern "C" {
#include <bip39.h>
#include <bignum.h>
#include <bip32.h>
#include <base58.h>
#include <curves.h>
#include <ecdsa.h>
#include <ripemd160.h>
}

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "Trezor.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Trezor::"

namespace opentxs
{
crypto::Trezor* Factory::Trezor(const api::Crypto& crypto)
{
    return new crypto::implementation::Trezor(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
Trezor::Trezor(const api::Crypto& crypto)
#if OT_CRYPTO_WITH_BIP32
    : Bip32(crypto)
#endif  // OT_CRYPTO_WITH_BIP32
#if OPENTXS_TREZOR_PROVIDES_ECDSA
#if OT_CRYPTO_WITH_BIP32
    ,
#else
    :
#endif  // OT_CRYPTO_WITH_BIP32
    EcdsaProvider(crypto)
#endif  // OPENTXS_TREZOR_PROVIDES_ECDSA
{
#if OPENTXS_TREZOR_PROVIDES_ECDSA
    secp256k1_ = ::get_curve_by_name(curve_name(EcdsaCurve::secp256k1).c_str());
    ed25519_ = ::get_curve_by_name(curve_name(EcdsaCurve::ed25519).c_str());

    OT_ASSERT(nullptr != secp256k1_);
    OT_ASSERT(nullptr != ed25519_);
#endif  // OPENTXS_TREZOR_PROVIDES_ECDSA
}

bool Trezor::Base58CheckDecode(const std::string&& input, RawData& output) const
{
    const std::size_t inputSize = input.size();

    if (0 == inputSize) { return false; }

    if (128 < inputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Input too long.").Flush();

        return false;
    }

    std::size_t outputSize = inputSize;
    output.resize(outputSize, 0x0);
    outputSize = ::base58_decode_check(
        input.data(), HASHER_SHA2D, output.data(), output.size());

    if (0 == outputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Decoding failed.").Flush();

        return false;
    }

    OT_ASSERT(outputSize <= output.size());

    output.resize(outputSize);

    return true;
}

std::string Trezor::Base58CheckEncode(
    const std::uint8_t* inputStart,
    const std::size_t& inputSize) const
{
    std::string output{};

    if (0 == inputSize) { return output; }

    if (128 < inputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Input too long.").Flush();

        return output;
    }

    const std::size_t bufferSize = inputSize + 32 + 4;
    output.resize(bufferSize, 0x0);
    const std::size_t outputSize = ::base58_encode_check(
        inputStart,
        inputSize,
        HASHER_SHA2D,
        const_cast<char*>(output.c_str()),
        output.size());

    OT_ASSERT(outputSize <= bufferSize);

    output.resize(outputSize - 1);

    return output;
}

std::string Trezor::curve_name(const EcdsaCurve& curve)
{
    switch (curve) {
        case (EcdsaCurve::secp256k1): {
            return ::SECP256K1_NAME;
        }
        case (EcdsaCurve::ed25519): {
            return ::ED25519_NAME;
        }
        default: {
        }
    }

    return "";
}

bool Trezor::RIPEMD160(
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    ripemd160(input, inputSize, output);

    return true;
}

#if OT_CRYPTO_WITH_BIP32
std::unique_ptr<HDNode> Trezor::derive_child(
    const HDNode& parent,
    const Bip32Index index,
    const DerivationMode privateVersion)
{
    auto output = std::make_unique<HDNode>(parent);
    int result{0};

    if (!output) { OT_FAIL; }

    if (privateVersion) {
        result = hdnode_private_ckd(output.get(), index);
        hdnode_fill_public_key(output.get());
    } else {
        result = hdnode_public_ckd(output.get(), index);
    }

    if (1 != result) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive child").Flush();

        return {};
    }

    return output;
}

std::unique_ptr<HDNode> Trezor::derive_child(
    const api::crypto::Hash& hash,
    const EcdsaCurve& curve,
    const OTPassword& seed,
    const Path& path,
    Bip32Fingerprint& parentID) const
{
    std::unique_ptr<HDNode> output{nullptr};
    Bip32Index depth = path.size();

    if (0 == depth) {
        parentID = 0;
        output = instantiate_node(curve, seed);
    } else {
        auto parentPath{path};
        parentPath.pop_back();
        auto parentNode = derive_child(hash, curve, seed, parentPath, parentID);

        if (parentNode) {
            const auto& childIndex = *path.crbegin();
            output = derive_child(*parentNode, childIndex, DERIVE_PRIVATE);
            const auto pubkey = Data::Factory(
                parentNode->public_key, sizeof(parentNode->public_key));
            parentID = key::HD::CalculateFingerprint(hash, pubkey);
        }
    }

    return output;
}

Trezor::Key Trezor::DeriveKey(
    const api::crypto::Hash& hash,
    const EcdsaCurve& curve,
    const OTPassword& seed,
    const Path& path) const
{
    Key output{OTPassword{}, OTPassword{}, Data::Factory(), {}, 0};
    auto& [privateKey, chainCode, publicKey, pathOut, parent] = output;
    const auto pNode = derive_child(hash, curve, seed, path, parent);

    if (false == bool(pNode)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive child.").Flush();
        privateKey = OTPassword{};
        chainCode = OTPassword{};
        publicKey = Data::Factory();
        pathOut = {};
        parent = 0;

        return output;
    }

    const auto& node = *pNode;
    privateKey.setMemory(node.private_key, sizeof(node.private_key));
    chainCode.setMemory(node.chain_code, sizeof(node.chain_code));
    publicKey->Assign(node.public_key, sizeof(node.public_key));
    pathOut = path;

    return output;
}

std::unique_ptr<HDNode> Trezor::instantiate_node(
    const EcdsaCurve& curve,
    const OTPassword& seed)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode);

    OT_ASSERT_MSG(output, "Instantiation of HD node failed.");

    auto curveName = curve_name(curve);

    if (1 > curveName.size()) { return output; }

    int result = ::hdnode_from_seed(
        static_cast<const std::uint8_t*>(seed.getMemory()),
        seed.getMemorySize(),
        curve_name(curve).c_str(),
        output.get());

    OT_ASSERT_MSG((1 == result), "Setup of HD node failed.");

    ::hdnode_fill_public_key(output.get());

    return output;
}

std::string Trezor::SeedToFingerprint(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    auto node = instantiate_node(curve, seed);

    if (node) {
        auto pubkey = Data::Factory(
            static_cast<void*>(node->public_key), sizeof(node->public_key));
        auto output = Identifier::Factory();
        output->CalculateDigest(pubkey);

        return output->str();
    }

    return "";
}
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_WITH_BIP39
bool Trezor::SeedToWords(const OTPassword& seed, OTPassword& words) const
{
    return words.setPassword(std::string(::mnemonic_from_data(
        static_cast<const std::uint8_t*>(seed.getMemory()),
        seed.getMemorySize())));
}

void Trezor::WordsToSeed(
    const OTPassword& words,
    OTPassword& seed,
    const OTPassword& passphrase) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());

    seed.SetSize(512 / 8);

    ::mnemonic_to_seed(
        words.getPassword(),
        passphrase.getPassword(),
        static_cast<std::uint8_t*>(seed.getMemoryWritable()),
        nullptr);
}
#endif  // OT_CRYPTO_WITH_BIP39

#if OPENTXS_TREZOR_PROVIDES_ECDSA
bool Trezor::ECDH(
    const Data& publicKey,
    const OTPassword& privateKey,
    OTPassword& secret) const
{
    curve_point point;
    auto curve = get_curve(proto::AKEYTYPE_SECP256K1);  // TODO pass as argument

    OT_ASSERT(nullptr != curve);

    const bool havePublic = ecdsa_read_pubkey(
        curve->params,
        static_cast<const std::uint8_t*>(publicKey.data()),
        &point);

    if (!havePublic) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Invalid public key.").Flush();

        return false;
    }

    bignum256 scalar{};
    bn_read_be(privateKey.getMemory_uint8(), &scalar);
    curve_point sharedSecret{};
    point_multiply(curve->params, &scalar, &point, &sharedSecret);
    std::array<std::uint8_t, 32> output{};
    secret.setMemory(output.data(), sizeof(output));

    OT_ASSERT(32 == secret.getMemorySize());

    bn_write_be(
        &sharedSecret.x,
        static_cast<std::uint8_t*>(secret.getMemoryWritable()));

    return true;
}

const curve_info* Trezor::get_curve(const EcdsaCurve& curve) const
{
    switch (curve) {
        case EcdsaCurve::secp256k1: {
            return secp256k1_;
        }
        case EcdsaCurve::ed25519: {
            return ed25519_;
        }
        default: {
            OT_FAIL
        }
    }
}

const curve_info* Trezor::get_curve(const proto::AsymmetricKeyType& curve) const
{
    switch (curve) {
        case proto::AKEYTYPE_SECP256K1: {
            return secp256k1_;
        }
        case proto::AKEYTYPE_ED25519: {
            return ed25519_;
        }
        default: {
            OT_FAIL
        }
    }
}

bool Trezor::is_valid(const OTPassword& key) const
{
    std::unique_ptr<bignum256> input(new bignum256);
    std::unique_ptr<bignum256> max(new bignum256);

    OT_ASSERT(input);
    OT_ASSERT(max);

    bn_read_be(key.getMemory_uint8(), input.get());
    bn_normalize(input.get());
    bn_read_be(KeyMax, max.get());
    bn_normalize(max.get());
    const bool zero = bn_is_zero(input.get());
    const bool size = bn_is_less(input.get(), max.get());

    return (!zero && size);
}

bool Trezor::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    bool valid = false;

    do {
        privateKey.randomizeMemory(256 / 8);

        if (is_valid(privateKey)) { valid = true; }
    } while (false == valid);

    return ScalarBaseMultiply(privateKey, publicKey);
}

bool Trezor::ScalarBaseMultiply(const OTPassword& privateKey, Data& publicKey)
    const
{
    std::array<std::uint8_t, 33> blank{};
    publicKey.Assign(blank.data(), blank.size());
    curve_point notUsed{};
    auto curve = get_curve(proto::AKEYTYPE_SECP256K1);  // TODO pass as argument

    OT_ASSERT(nullptr != curve);

    ecdsa_get_public_key33(
        curve->params,
        privateKey.getMemory_uint8(),
        static_cast<std::uint8_t*>(const_cast<void*>(publicKey.data())));

    return (
        1 == ecdsa_read_pubkey(
                 curve->params,
                 static_cast<const std::uint8_t*>(publicKey.data()),
                 &notUsed));
}

bool Trezor::Sign(
    const api::internal::Core& api,
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const proto::HashType hashType,
    Data& signature,
    const PasswordPrompt& reason,
    const OTPassword* exportPassword) const
{
    auto hash = Data::Factory();
    bool haveDigest =
        EcdsaProvider::crypto_.Hash().Digest(hashType, plaintext, hash);

    if (!haveDigest) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain the contract hash.")
            .Flush();

        return false;
    }

    OT_ASSERT(nullptr != hash->data());

    OTPassword privKey{};
    bool havePrivateKey{false};

    // TODO
    OT_ASSERT_MSG(nullptr == exportPassword, "This case is not yet handled.");

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const crypto::key::Secp256k1*>(&theKey);

    if (nullptr == key) { return false; }

    havePrivateKey = AsymmetricKeyToECPrivatekey(api, *key, reason, privKey);

    if (havePrivateKey) {
        OT_ASSERT(nullptr != privKey.getMemory());

        std::array<std::uint8_t, 64> blank{};
        signature.Assign(blank.data(), blank.size());
        const bool signatureCreated =
            (0 == ecdsa_sign_digest(
                      get_curve(theKey.keyType())->params,
                      static_cast<const std::uint8_t*>(privKey.getMemory()),
                      static_cast<const std::uint8_t*>(hash->data()),
                      static_cast<std::uint8_t*>(
                          const_cast<void*>(signature.data())),
                      nullptr,
                      nullptr));

        if (signatureCreated) {

            return true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Call to ecdsa_sign_digest() failed.")
                .Flush();

            return false;
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not extract ecdsa private key from Asymmetric.")
            .Flush();

        return false;
    }
}

bool Trezor::Verify(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const Data& signature,
    const proto::HashType hashType,
    const PasswordPrompt& reason) const
{
    auto hash = Data::Factory();
    bool haveDigest =
        EcdsaProvider::crypto_.Hash().Digest(hashType, plaintext, hash);

    if (!haveDigest) { return false; }

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const crypto::key::Secp256k1*>(&theKey);

    if (nullptr == key) { return false; }

    auto ecdsaPubkey = Data::Factory();
    const bool havePublicKey = AsymmetricKeyToECPubkey(*key, ecdsaPubkey);

    if (!havePublicKey) { return false; }

    const bool output =
        (0 == ecdsa_verify_digest(
                  get_curve(theKey.keyType())->params,
                  static_cast<const std::uint8_t*>(ecdsaPubkey->data()),
                  static_cast<const std::uint8_t*>(signature.data()),
                  static_cast<const std::uint8_t*>(hash->data())));

    return output;
}
#endif  // OPENTXS_TREZOR_PROVIDES_ECDSA
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_TREZOR
