// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::crypto::key::implementation
{
class Symmetric final : virtual public key::Symmetric
{
public:
    bool ChangePassword(
        const OTPasswordData& oldPassword,
        const OTPassword& newPassword) override;

    /** Decrypt ciphertext using the symmetric key
     *
     *  \param[in] ciphertext The data to be decrypted
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] plaintext The decrypted output
     */
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        std::string& plaintext) override;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        String& plaintext) override;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        Data& plaintext) override;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const OTPasswordData& keyPassword,
        OTPassword& plaintext) override;

    /** Encrypt plaintext using the symmetric key
     *
     *  \param[in] plaintext The data to be encrypted
     *  \param[in] iv Nonce for the encryption operation
     *  \param[in] keyPassword The password needed to decrypt the key
     *  \param[out] ciphertext The encrypted output
     *  \param[in] attachKey Set to true if the serialized key should be
     *                       embedded in the ciphertext
     *  \param[in] mode The symmetric algorithm to use for encryption
     */
    bool Encrypt(
        const std::string& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) override;
    bool Encrypt(
        const String& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) override;
    bool Encrypt(
        const OTPassword& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) override;
    bool Encrypt(
        const Data& plaintext,
        const Data& iv,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) override;

    OTIdentifier ID() override;

    bool Serialize(proto::SymmetricKey& output) const override;

    bool Unlock(const OTPasswordData& keyPassword) override;

    operator bool() const override { return true; }

    ~Symmetric() = default;

private:
    friend key::Symmetric;

    /// The library providing the underlying crypto algorithms
    const crypto::SymmetricProvider& engine_;
    const VersionNumber version_{0};
    const proto::SymmetricKeyType type_{proto::SKEYTYPE_ERROR};
    /// Size of the plaintext key in bytes;
    std::size_t key_size_{0};
    std::unique_ptr<std::string> salt_;
    std::uint64_t operations_{0};
    std::uint64_t difficulty_{0};
    /// The unencrypted, fully-derived version of the key which is provided to
    /// encryption functions.
    std::unique_ptr<OTPassword> plaintext_key_;
    /// The encrypted form of the plaintext key
    std::unique_ptr<proto::Ciphertext> encrypted_key_;

    Symmetric* clone() const override;

    static bool Allocate(const std::size_t size, String& container);
    static bool Allocate(const std::size_t size, Data& container);
    static bool Allocate(
        const std::size_t size,
        std::string& container,
        const bool random);
    bool Allocate(
        const std::size_t size,
        OTPassword& container,
        const bool text = false);
    bool Decrypt(
        const proto::Ciphertext& input,
        const OTPasswordData& keyPassword,
        std::uint8_t* plaintext);
    bool Encrypt(
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* iv,
        const std::size_t ivSize,
        const proto::SymmetricMode mode,
        const OTPasswordData& keyPassword,
        proto::Ciphertext& ciphertext,
        const bool text = false);
    bool EncryptKey(
        const OTPassword& plaintextKey,
        const OTPasswordData& keyPassword,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);
    bool GetPassword(const OTPasswordData& keyPassword, OTPassword& password);

    Symmetric(const crypto::SymmetricProvider& engine);
    Symmetric(
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized);
    Symmetric(
        const crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::string& salt,
        const std::size_t size,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);
    Symmetric() = delete;
    Symmetric(const Symmetric&);
    Symmetric& operator=(const Symmetric&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
