// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "crypto/library/Sodium.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <array>
#include <functional>
#include <string>
#include <string_view>

#include "Factory.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/Proto.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Asymmetric.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/HashingProvider.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "util/Sodium.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

#define OT_METHOD "opentxs::crypto::implementation::Sodium::"

namespace opentxs
{
crypto::Sodium* Factory::Sodium(const api::Crypto& crypto)
{
    return new crypto::implementation::Sodium(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
Sodium::Sodium(const api::Crypto& crypto)
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    : AsymmetricProvider()
    , EcdsaProvider(crypto)
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
{
    const auto result = ::sodium_init();

    OT_ASSERT(-1 != result);
}

bool Sodium::Decrypt(
    const proto::Ciphertext& ciphertext,
    const std::uint8_t* key,
    const std::size_t keySize,
    std::uint8_t* plaintext) const
{
    const auto& message = ciphertext.data();
    const auto& nonce = ciphertext.iv();
    const auto& mac = ciphertext.tag();
    const auto& mode = ciphertext.mode();

    if (KeySize(mode) != keySize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.").Flush();

        return false;
    }

    if (IvSize(mode) != nonce.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect nonce size.").Flush();

        return false;
    }

    switch (ciphertext.mode()) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_decrypt_detached(
                         plaintext,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(),
                         reinterpret_cast<const unsigned char*>(mac.data()),
                         nullptr,
                         0,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }

    return false;
}

bool Sodium::Derive(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* salt,
    const std::size_t saltSize,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricKeyType type,
    std::uint8_t* output,
    std::size_t outputSize) const
{
    const auto requiredSize = SaltSize(type);

    if (requiredSize != saltSize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect salt size (")(saltSize)(
            "). Required: (")(requiredSize)(").")
            .Flush();

        return false;
    }

    const auto success = 0 == crypto_pwhash(
                                  output,
                                  outputSize,
                                  reinterpret_cast<const char*>(input),
                                  inputSize,
                                  salt,
                                  operations,
                                  difficulty,
                                  crypto_pwhash_ALG_ARGON2I13);

    if (false == success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive key").Flush();
    }

    return success;
}

bool Sodium::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return (
                0 == ::crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         nullptr,
                         0));
        }
        case (proto::HASHTYPE_SHA256): {
            return (0 == ::crypto_hash_sha256(output, input, inputSize));
        }
        case (proto::HASHTYPE_SHA512): {
            return (0 == ::crypto_hash_sha512(output, input, inputSize));
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash function.").Flush();

    return false;
}

bool Sodium::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* key,
    const std::size_t keySize,
    proto::Ciphertext& ciphertext) const
{
    OT_ASSERT(nullptr != input);
    OT_ASSERT(nullptr != key);

    const auto& mode = ciphertext.mode();
    const auto& nonce = ciphertext.iv();
    auto& tag = *ciphertext.mutable_tag();
    auto& output = *ciphertext.mutable_data();

    bool result = false;

    if (mode == proto::SMODE_ERROR) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mode.").Flush();

        return result;
    }

    if (KeySize(mode) != keySize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.").Flush();

        return result;
    }

    if (IvSize(mode) != nonce.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect nonce size.").Flush();

        return result;
    }

    ciphertext.set_version(1);
    tag.resize(TagSize(mode), 0x0);
    output.resize(inputSize, 0x0);

    OT_ASSERT(false == nonce.empty());
    OT_ASSERT(false == tag.empty());

    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_encrypt_detached(
                         reinterpret_cast<unsigned char*>(output.data()),
                         reinterpret_cast<unsigned char*>(tag.data()),
                         nullptr,
                         input,
                         inputSize,
                         nullptr,
                         0,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }

    return result;
}

bool Sodium::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return (
                0 == ::crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         key,
                         keySize));
        }
        case (proto::HASHTYPE_SHA256): {
            if (crypto_auth_hmacsha256_KEYBYTES != keySize) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.")
                    .Flush();

                return false;
            }

            return (
                0 == ::crypto_auth_hmacsha256(output, input, inputSize, key));
        }
        case (proto::HASHTYPE_SHA512): {
            if (crypto_auth_hmacsha512_KEYBYTES != keySize) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.")
                    .Flush();

                return false;
            }

            return (0 == crypto_auth_hmacsha512(output, input, inputSize, key));
        }
        case (proto::HASHTYPE_SIPHASH24): {
            if (crypto_shorthash_KEYBYTES != keySize) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.")
                    .Flush();

                return false;
            }

            return 0 == ::crypto_shorthash(output, input, inputSize, key);
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash function.").Flush();

    return false;
}

std::size_t Sodium::IvSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_NPUBBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

std::size_t Sodium::KeySize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_KEYBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::RandomKeypair(
    const AllocateOutput privateKey,
    const AllocateOutput publicKey,
    const proto::KeyRole,
    const NymParameters&,
    const AllocateOutput) const noexcept
{
    auto seed = OTPassword{};
    seed.randomizeMemory(crypto_sign_SEEDBYTES);

    return sodium::ExpandSeed(seed.Bytes(), privateKey, publicKey);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

bool Sodium::RandomizeMemory(void* destination, const std::size_t size) const
{
    ::randombytes_buf(destination, size);

    return true;
}

std::size_t Sodium::SaltSize(const proto::SymmetricKeyType type) const
{
    switch (type) {
        case (proto::SKEYTYPE_ARGON2): {

            return crypto_pwhash_SALTBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported key type (")(
                type)(").")
                .Flush();
        }
    }

    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::ScalarAdd(
    const ReadView lhs,
    const ReadView rhs,
    const AllocateOutput result) const noexcept
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    if (crypto_core_ed25519_SCALARBYTES != lhs.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid lhs scalar").Flush();

        return false;
    }

    if (crypto_core_ed25519_SCALARBYTES != rhs.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid rhs scalar").Flush();

        return false;
    }

    auto key = result(crypto_core_ed25519_SCALARBYTES);

    if (false == key.valid(crypto_core_ed25519_SCALARBYTES)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for result")
            .Flush();

        return false;
    }

    ::crypto_core_ed25519_scalar_add(
        key.as<unsigned char>(),
        reinterpret_cast<const unsigned char*>(lhs.data()),
        reinterpret_cast<const unsigned char*>(rhs.data()));

    return true;
}

auto Sodium::ScalarMultiplyBase(
    const ReadView scalar,
    const AllocateOutput result) const noexcept -> bool
{
    if (false == bool(result)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    if (crypto_scalarmult_ed25519_SCALARBYTES != scalar.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid scalar").Flush();

        return false;
    }

    auto pub = result(crypto_scalarmult_ed25519_BYTES);

    if (false == pub.valid(crypto_scalarmult_ed25519_BYTES)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    return 0 == ::crypto_scalarmult_ed25519_base(
                    pub.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(scalar.data()));
}

auto Sodium::SharedSecret(
    const key::Asymmetric& publicKey,
    const key::Asymmetric& privateKey,
    const PasswordPrompt& reason,
    OTPassword& secret) const noexcept -> bool
{
    if (publicKey.keyType() != proto::AKEYTYPE_ED25519) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Public key is wrong type")
            .Flush();

        return false;
    }

    if (privateKey.keyType() != proto::AKEYTYPE_ED25519) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Private key is wrong type")
            .Flush();

        return false;
    }

    const auto pub = publicKey.PublicKey();
    const auto prv = privateKey.PrivateKey(reason);

    if (crypto_sign_PUBLICKEYBYTES != pub.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key ").Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Expected: ")(
            crypto_sign_PUBLICKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual:   ")(pub.size()).Flush();

        return false;
    }

    if (crypto_sign_SECRETKEYBYTES != prv.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid private key").Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Expected: ")(
            crypto_sign_SECRETKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual:   ")(prv.size()).Flush();

        return false;
    }

    static const auto blank =
        std::array<unsigned char, crypto_scalarmult_curve25519_BYTES>{};
    auto privateEd = OTPassword{};
    privateEd.setMemory(blank.data(), blank.size());
    secret.setMemory(blank.data(), blank.size());
    auto publicEd{blank};

    OT_ASSERT(crypto_scalarmult_curve25519_BYTES == privateEd.getMemorySize());

    if (0 != ::crypto_sign_ed25519_pk_to_curve25519(
                 publicEd.data(),
                 reinterpret_cast<const unsigned char*>(pub.data()))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": crypto_sign_ed25519_pk_to_curve25519 error")
            .Flush();

        return false;
    }

    if (0 !=
        ::crypto_sign_ed25519_sk_to_curve25519(
            reinterpret_cast<unsigned char*>(privateEd.getMemoryWritable()),
            reinterpret_cast<const unsigned char*>(prv.data()))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": crypto_sign_ed25519_sk_to_curve25519 error")
            .Flush();

        return false;
    }

    OT_ASSERT(crypto_scalarmult_SCALARBYTES == privateEd.getMemorySize());
    OT_ASSERT(crypto_scalarmult_BYTES == publicEd.size());
    OT_ASSERT(crypto_scalarmult_BYTES == secret.getMemorySize());

    return 0 == ::crypto_scalarmult(
                    static_cast<unsigned char*>(secret.getMemoryWritable()),
                    static_cast<const unsigned char*>(privateEd.getMemory()),
                    publicEd.data());
}

bool Sodium::Sign(
    const api::internal::Core& api,
    const Data& plaintext,
    const key::Asymmetric& key,
    const proto::HashType type,
    Data& signature,
    const PasswordPrompt& reason,
    const OTPassword* exportPassword) const
{
    if (proto::AKEYTYPE_ED25519 != key.keyType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key type").Flush();

        return false;
    }

    if (false == key.HasPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": A private key required when generating signatures")
            .Flush();

        return false;
    }

    if (proto::HASHTYPE_BLAKE2B256 != type) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Unsupported hash function: ")(
            type)
            .Flush();

        return false;
    }

    const auto priv = key.PrivateKey(reason);

    if (nullptr == priv.data() || 0 == priv.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing private key").Flush();

        return false;
    }

    if (crypto_sign_SECRETKEYBYTES != priv.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid private key").Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Expected: ")(
            crypto_sign_SECRETKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual:   ")(priv.size()).Flush();

        return false;
    }

    auto allocate = signature.WriteInto();

    if (false == bool(allocate)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    auto output = allocate(crypto_sign_BYTES);

    if (false == output.valid(crypto_sign_BYTES)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to allocate space for signature")
            .Flush();

        return false;
    }

    const auto success =
        0 == ::crypto_sign_detached(
                 output.as<unsigned char>(),
                 nullptr,
                 static_cast<const unsigned char*>(plaintext.data()),
                 plaintext.size(),
                 reinterpret_cast<const unsigned char*>(priv.data()));

    if (false == success) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign plaintext.")
            .Flush();
    }

    return success;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

std::size_t Sodium::TagSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_ABYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::Verify(
    const Data& plaintext,
    const key::Asymmetric& key,
    const Data& signature,
    const proto::HashType type) const
{
    if (proto::AKEYTYPE_ED25519 != key.keyType()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid key type").Flush();

        return false;
    }

    if (proto::HASHTYPE_BLAKE2B256 != type) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Unsupported hash function: ")(
            type)
            .Flush();

        return false;
    }

    if (crypto_sign_BYTES != signature.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid signature").Flush();

        return false;
    }

    const auto pub = key.PublicKey();

    if (nullptr == pub.data() || 0 == pub.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing public key").Flush();

        return false;
    }

    if (crypto_sign_PUBLICKEYBYTES != pub.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid public key").Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Expected: ")(
            crypto_sign_PUBLICKEYBYTES)
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual:   ")(pub.size()).Flush();

        return false;
    }

    const auto success =
        0 == ::crypto_sign_verify_detached(
                 static_cast<const unsigned char*>(signature.data()),
                 static_cast<const unsigned char*>(plaintext.data()),
                 plaintext.size(),
                 reinterpret_cast<const unsigned char*>(pub.data()));

    if (false == success) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Failed to verify signature")
            .Flush();
    }

    return success;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace opentxs::crypto::implementation
