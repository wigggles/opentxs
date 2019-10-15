// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/api/Api.hpp"

namespace opentxs::crypto::key::implementation
{
class Asymmetric : virtual public key::Asymmetric
{
public:
    /** Only works for public keys. */
    bool CalculateID(Identifier& theOutput) const final;
    const crypto::AsymmetricProvider& engine() const final { return provider_; }
    const OTSignatureMetadata* GetMetadata() const final { return m_pMetadata; }
    bool hasCapability(const NymCapability& capability) const override;
    bool HasPrivate() const final { return has_private_; }
    bool HasPublic() const final { return has_public_; }
    proto::AsymmetricKeyType keyType() const final;
    proto::Signature NewSignature(
        const Identifier& credentialID,
        const proto::SignatureRole role,
        const proto::HashType hash) const;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
    const proto::KeyRole& Role() const final { return role_; }
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    OTData SerializeKeyToData(const proto::AsymmetricKey& rhs) const final;
    proto::HashType SigHashType() const override { return StandardHash; }
    bool Sign(
        const Data& plaintext,
        proto::Signature& sig,
        const PasswordPrompt& reason,
        const OTPassword* exportPassword = nullptr,
        const String& credID = String::Factory(""),
        const proto::SignatureRole role = proto::SIGROLE_ERROR) const final;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const Identifier& credential,
        const PasswordPrompt& reason,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const proto::HashType hash = proto::HASHTYPE_BLAKE2B256) const final;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const PasswordPrompt& reason) const final;
    VersionNumber Version() const final { return version_; }

    void Release() override;
    void ReleaseKey() final { Release(); }
    /** Don't use this, normally it's not necessary. */
    void SetAsPublic() final
    {
        has_public_ = true;
        has_private_ = false;
    }
    /** (Only if you really know what you are doing.) */
    void SetAsPrivate() final
    {
        has_public_ = false;
        has_private_ = true;
    }

    operator bool() const override;
    bool operator==(const proto::AsymmetricKey&) const final;

    ~Asymmetric() override;

protected:
    friend OTAsymmetricKey;

    const api::internal::Core& api_;
    const crypto::AsymmetricProvider& provider_;
    const VersionNumber version_;
    const proto::AsymmetricKeyType type_;
    const proto::KeyRole role_;
    bool has_public_{false};
    bool has_private_{false};
    OTSignatureMetadata* m_pMetadata{nullptr};

    virtual bool get_public_key(String& strKey) const = 0;

    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const bool hasPublic,
        const bool hasPrivate,
        const VersionNumber version) noexcept;
    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey) noexcept;
    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKey& serializedKey,
        const bool hasPublic,
        const bool hasPrivate) noexcept;
    Asymmetric(
        const api::internal::Core& api,
        const crypto::AsymmetricProvider& engine,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version) noexcept;
    Asymmetric(const Asymmetric& rhs) noexcept;

    virtual void ReleaseKeyLowLevel_Hook() {}

private:
    static const std::map<proto::SignatureRole, VersionNumber> sig_version_;

    Asymmetric() = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
