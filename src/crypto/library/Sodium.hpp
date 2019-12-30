// Copyright (c) 2010-2019 The Open-Transactions developers
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
        std::uint8_t* output) const final;
    bool HMAC(
        const proto::HashType hashType,
        const std::uint8_t* input,
        const size_t inputSize,
        const std::uint8_t* key,
        const size_t keySize,
        std::uint8_t* output) const final;
    bool RandomizeMemory(void* destination, const std::size_t size) const final;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    bool RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const proto::KeyRole role,
        const NymParameters& options,
        const AllocateOutput params) const noexcept final;
    bool ScalarAdd(
        const ReadView lhs,
        const ReadView rhs,
        const AllocateOutput result) const noexcept final;
    bool ScalarMultiplyBase(const ReadView scalar, const AllocateOutput result)
        const noexcept final;
    bool SharedSecret(
        const key::Asymmetric& publicKey,
        const key::Asymmetric& privateKey,
        const PasswordPrompt& reason,
        OTPassword& secret) const noexcept final;
    bool Sign(
        const api::internal::Core& api,
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const proto::HashType hashType,
        Data& signature,  // output
        const PasswordPrompt& reason,
        const OTPassword* exportPassword = nullptr) const final;
    bool Verify(
        const Data& plaintext,
        const key::Asymmetric& theKey,
        const Data& signature,
        const proto::HashType hashType) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

    ~Sodium() final = default;

private:
    friend opentxs::Factory;

    static const proto::SymmetricMode DEFAULT_MODE{
        proto::SMODE_CHACHA20POLY1305};

    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const std::uint8_t* key,
        const std::size_t keySize,
        std::uint8_t* plaintext) const final;
    proto::SymmetricMode DefaultMode() const final { return DEFAULT_MODE; }
    bool Derive(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* salt,
        const std::size_t saltSize,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type,
        std::uint8_t* output,
        std::size_t outputSize) const final;
    bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* key,
        const std::size_t keySize,
        proto::Ciphertext& ciphertext) const final;
    std::size_t IvSize(const proto::SymmetricMode mode) const final;
    std::size_t KeySize(const proto::SymmetricMode mode) const final;
    std::size_t SaltSize(const proto::SymmetricKeyType type) const final;
    std::size_t TagSize(const proto::SymmetricMode mode) const final;

    Sodium(const api::Crypto& crypto);
    Sodium() = delete;
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    Sodium& operator=(const Sodium&) = delete;
    Sodium& operator=(Sodium&&) = delete;
};
}  // namespace opentxs::crypto::implementation
