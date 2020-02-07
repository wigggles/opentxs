// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"

#include "internal/api/Api.hpp"

#include <functional>

namespace opentxs::crypto::key::implementation
{
class Asymmetric : virtual public key::Asymmetric
{
public:
    OTData CalculateHash(
        const proto::HashType hashType,
        const PasswordPrompt& password) const noexcept final;
    bool CalculateTag(
        const identity::Authority& nym,
        const proto::AsymmetricKeyType type,
        const PasswordPrompt& reason,
        std::uint32_t& tag,
        OTPassword& password) const noexcept final;
    bool CalculateTag(
        const key::Asymmetric& dhKey,
        const Identifier& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept final;
    bool CalculateSessionPassword(
        const key::Asymmetric& dhKey,
        const PasswordPrompt& reason,
        OTPassword& password) const noexcept final;
    bool CalculateID(Identifier& theOutput) const noexcept final;
    const crypto::AsymmetricProvider& engine() const noexcept final
    {
        return provider_;
    }
    const OTSignatureMetadata* GetMetadata() const noexcept final
    {
        return m_pMetadata;
    }
    bool hasCapability(const NymCapability& capability) const noexcept override;
    bool HasPrivate() const noexcept final { return has_private_; }
    bool HasPublic() const noexcept final { return has_public_; }
    proto::AsymmetricKeyType keyType() const noexcept final { return type_; }
    proto::Signature NewSignature(
        const Identifier& credentialID,
        const proto::SignatureRole role,
        const proto::HashType hash) const;
    ReadView Params() const noexcept override { return {}; }
    const std::string Path() const noexcept override;
    bool Path(proto::HDPath& output) const noexcept override;
    ReadView PrivateKey(const PasswordPrompt& reason) const noexcept final;
    ReadView PublicKey() const noexcept final { return key_->Bytes(); }
    proto::KeyRole Role() const noexcept final { return role_; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const noexcept override;
    proto::HashType SigHashType() const noexcept override
    {
        return StandardHash;
    }
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const Identifier& credential,
        const PasswordPrompt& reason,
        proto::KeyRole key,
        const proto::HashType hash) const noexcept final;
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const noexcept override;
    bool Verify(const Data& plaintext, const proto::Signature& sig) const
        noexcept final;
    VersionNumber Version() const noexcept final { return version_; }

    operator bool() const noexcept override;
    bool operator==(const proto::AsymmetricKey&) const noexcept final;

    ~Asymmetric() override;

protected:
    friend OTAsymmetricKey;

    using EncryptedKey = std::unique_ptr<proto::Ciphertext>;
    using EncryptedExtractor = std::function<EncryptedKey(Data&, OTPassword&)>;

    const api::internal::Core& api_;
    const crypto::AsymmetricProvider& provider_;
    const VersionNumber version_;
    const proto::AsymmetricKeyType type_;
    const proto::KeyRole role_;
    bool has_public_;
    bool has_private_;
    OTSignatureMetadata* m_pMetadata;
    const OTData key_;
    mutable OTPassword plaintext_key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_;

    static auto create_key(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& provider,
        const NymParameters& options,
        const proto::KeyRole role,
        const AllocateOutput publicKey,
        const AllocateOutput privateKey,
        const OTPassword& prv,
        const AllocateOutput params,
        const PasswordPrompt& reason) noexcept(false)
        -> std::unique_ptr<proto::Ciphertext>;
    static std::unique_ptr<proto::Ciphertext> encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const ReadView plaintext) noexcept;
    static bool encrypt_key(
        const api::internal::Core& api,
        const PasswordPrompt& reason,
        const ReadView plaintext,
        proto::Ciphertext& ciphertext) noexcept;
    static auto encrypt_key(
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason,
        const bool attach,
        const ReadView plaintext,
        proto::Ciphertext& ciphertext) noexcept -> bool;
    static auto generate_key(
        const crypto::AsymmetricProvider& provider,
        const NymParameters& options,
        const proto::KeyRole role,
        const AllocateOutput publicKey,
        const AllocateOutput privateKey,
        const AllocateOutput params) noexcept(false) -> void;

    auto get_password(
        const key::Asymmetric& target,
        const PasswordPrompt& reason,
        OTPassword& password) const noexcept -> bool;
    auto get_tag(
        const key::Asymmetric& target,
        const Identifier& credential,
        const PasswordPrompt& reason,
        std::uint32_t& tag) const noexcept -> bool;

    virtual void erase_private_data();

    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const bool hasPublic,
        const bool hasPrivate,
        const VersionNumber version,
        OTData&& pubkey,
        EncryptedExtractor) noexcept(false);
    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version,
        EncryptedExtractor) noexcept(false);
    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey,
        EncryptedExtractor) noexcept(false);
    Asymmetric(const Asymmetric& rhs) noexcept;

private:
    static const std::map<proto::SignatureRole, VersionNumber> sig_version_;

    OTData SerializeKeyToData(const proto::AsymmetricKey& rhs) const;

    Asymmetric() = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
