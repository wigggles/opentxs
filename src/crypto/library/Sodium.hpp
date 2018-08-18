// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::crypto::implementation
{
class Sodium final : virtual public crypto::Sodium
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
                     public AsymmetricProvider,
                     public EcdsaProvider
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
{
public:
    bool Digest(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        std::uint8_t* output) const override;
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const override;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool RandomKeypair(OTPassword& privateKey, Data& publicKey) const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool RandomizeMemory(void* destination, const std::size_t size)
        const override;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool Sign(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr) const override;
    bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const override;
    bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

    ~Sodium() = default;

private:
    friend opentxs::Factory;

    static const proto::SymmetricMode DEFAULT_MODE{
        proto::SMODE_CHACHA20POLY1305};

    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const override;
    proto::SymmetricMode DefaultMode() const override { return DEFAULT_MODE; }
    bool Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type,
        std::uint8_t* output,
        std::size_t outputSize) const override;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool ECDH(const Data& publicKey, const OTPassword& seed, OTPassword& secret)
        const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const override;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool ExpandSeed(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    std::size_t IvSize(const proto::SymmetricMode mode) const override;
    std::size_t KeySize(const proto::SymmetricMode mode) const override;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool ScalarBaseMultiply(const OTPassword& seed, Data& publicKey)
        const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    std::size_t SaltSize(const proto::SymmetricKeyType type) const override;
    std::size_t TagSize(const proto::SymmetricMode mode) const override;

    Sodium(const api::Crypto& crypto);
    Sodium() = delete;
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    Sodium& operator=(const Sodium&) = delete;
    Sodium& operator=(Sodium&&) = delete;
};
}  // namespace opentxs::crypto::implementation
