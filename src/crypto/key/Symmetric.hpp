// Copyright (c) 2019 The Open-Transactions developers
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
    operator bool() const final { return true; }

    const api::Core& api() const final { return api_; }

    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        std::string& plaintext) const final;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        String& plaintext) const final;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        Data& plaintext) const final;
    bool Decrypt(
        const proto::Ciphertext& ciphertext,
        const PasswordPrompt& reason,
        OTPassword& plaintext) const final;
    bool Encrypt(
        const std::string& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const final;
    bool Encrypt(
        const String& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const final;
    bool Encrypt(
        const OTPassword& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const final;
    bool Encrypt(
        const Data& plaintext,
        const Data& iv,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool attachKey = true,
        const proto::SymmetricMode mode = proto::SMODE_ERROR) const final;
    OTIdentifier ID(const PasswordPrompt& reason) const final;
    bool RawKey(const PasswordPrompt& reason, OTPassword& output) const final;
    bool Serialize(proto::SymmetricKey& output) const final;
    bool Unlock(const PasswordPrompt& reason) const final;

    bool ChangePassword(
        const PasswordPrompt& reason,
        const OTPassword& newPassword) final;

    ~Symmetric() = default;

private:
    friend opentxs::Factory;
    friend key::Symmetric;

    const api::internal::Core& api_;
    /// The library providing the underlying crypto algorithms
    const crypto::SymmetricProvider& engine_;
    const VersionNumber version_{0};
    const proto::SymmetricKeyType type_{proto::SKEYTYPE_ERROR};
    /// Size of the plaintext key in bytes;
    std::size_t key_size_{0};
    std::uint64_t operations_{0};
    std::uint64_t difficulty_{0};
    mutable std::unique_ptr<std::string> salt_;
    /// The unencrypted, fully-derived version of the key which is provided to
    /// encryption functions.
    mutable std::unique_ptr<OTPassword> plaintext_key_;
    /// The encrypted form of the plaintext key
    mutable std::unique_ptr<proto::Ciphertext> encrypted_key_;
    mutable std::mutex lock_;

    Symmetric* clone() const final;

    static bool Allocate(const std::size_t size, String& container);
    static bool Allocate(const std::size_t size, Data& container);
    static bool Allocate(
        const std::size_t size,
        std::string& container,
        const bool random);

    bool allocate(
        const Lock& lock,
        const std::size_t size,
        OTPassword& container,
        const bool text = false) const;
    bool decrypt(
        const Lock& lock,
        const proto::Ciphertext& input,
        const PasswordPrompt& reason,
        std::uint8_t* plaintext) const;
    bool encrypt(
        const Lock& lock,
        const std::uint8_t* input,
        const std::size_t inputSize,
        const std::uint8_t* iv,
        const std::size_t ivSize,
        const proto::SymmetricMode mode,
        const PasswordPrompt& reason,
        proto::Ciphertext& ciphertext,
        const bool text = false) const;
    bool encrypt_key(
        const Lock& lock,
        const OTPassword& plaintextKey,
        const PasswordPrompt& reason,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2) const;
    std::unique_ptr<proto::Ciphertext>& get_encrypted(const Lock& lock) const;
    bool get_password(
        const Lock& lock,
        const PasswordPrompt& keyPassword,
        OTPassword& password) const;
    std::unique_ptr<OTPassword>& get_plaintext(const Lock& lock) const;
    std::unique_ptr<std::string>& get_salt(const Lock& lock) const;
    bool serialize(const Lock& lock, proto::SymmetricKey& output) const;
    bool unlock(const Lock& lock, const PasswordPrompt& reason) const;

    Symmetric(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine);
    Symmetric(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const proto::SymmetricKey serialized);
    Symmetric(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const OTPassword& seed,
        const std::string& salt,
        const std::size_t size,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        const proto::SymmetricKeyType type = proto::SKEYTYPE_ARGON2);
    Symmetric(
        const api::internal::Core& api,
        const crypto::SymmetricProvider& engine,
        const VersionNumber version,
        const proto::SymmetricKeyType type,
        const std::size_t keySize,
        std::string* salt,
        const std::uint64_t operations,
        const std::uint64_t difficulty,
        OTPassword* plaintextKey,
        proto::Ciphertext* encryptedKey);
    Symmetric() = delete;
    Symmetric(const Symmetric&);
    Symmetric& operator=(const Symmetric&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
