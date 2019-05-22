// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "Asymmetric.hpp"

namespace opentxs::crypto::key::implementation
{
class EllipticCurve :
#if OT_CRYPTO_SUPPORTED_KEY_HD
    virtual public key::HD,
#else
    virtual public key::EllipticCurve,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    public Asymmetric
{
public:
    OTData CalculateHash(
        const proto::HashType hashType,
        const OTPasswordData& password) const override;
    virtual NymParameterType CreateType() const = 0;
    bool IsEmpty() const override;
    virtual bool GetKey(Data& key) const override;
    virtual bool GetKey(proto::Ciphertext& key) const override;
    bool GetPublicKey(String& strKey) const override;
    virtual bool GetPublicKey(Data& key) const override;
    bool Open(
        crypto::key::Asymmetric& dhPublic,
        crypto::key::Symmetric& sessionKey,
        OTPasswordData& password) const override;
    const std::string Path() const override { return {}; }
    bool Path(proto::HDPath& output) const override { return {}; }
    OTData PrivateKey() const override;
    OTData PublicKey() const override;
    virtual bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;
    bool Seal(
        OTAsymmetricKey& dhPublic,
        crypto::key::Symmetric& key,
        OTPasswordData& password) const override;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

    virtual bool SetKey(const Data& key) override;
    virtual bool SetKey(std::unique_ptr<proto::Ciphertext>& key) override;

    virtual ~EllipticCurve() = default;

protected:
    OTData key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_{nullptr};

#if OT_CRYPTO_SUPPORTED_KEY_HD
    EllipticCurve* clone() const override;
#endif

    explicit EllipticCurve(const proto::AsymmetricKey& serializedKey) noexcept;
    EllipticCurve(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version) noexcept;
    EllipticCurve(
        const proto::AsymmetricKeyType keyType,
        const String& publicKey,
        const VersionNumber version) noexcept;

private:
    friend class crypto::EcdsaProvider;

#if !OT_CRYPTO_SUPPORTED_KEY_HD
    EllipticCurve* clone() const override final;
#endif

    EllipticCurve() = delete;
    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
