// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "Asymmetric.hpp"

namespace opentxs::crypto::key::implementation
{
class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
    using ot_super = Asymmetric;

public:
    OTData CalculateHash(
        const proto::HashType hashType,
        const OTPasswordData& password) const override;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OTData Chaincode() const override;
#endif
    virtual NymParameterType CreateType() const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    int Depth() const override;
#endif
    bool IsEmpty() const override;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Bip32Fingerprint Fingerprint() const override;
#endif
    virtual bool GetKey(Data& key) const override;
    virtual bool GetKey(proto::Ciphertext& key) const override;
    bool GetPublicKey(String& strKey) const override;
    virtual bool GetPublicKey(Data& key) const override;
    bool Open(
        crypto::key::Asymmetric& dhPublic,
        crypto::key::Symmetric& sessionKey,
        OTPasswordData& password) const override;
    using ot_super::Path;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
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
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::string Xprv() const override;
    std::string Xpub() const override;
#endif

    virtual bool SetKey(const Data& key) override;
    virtual bool SetKey(std::unique_ptr<proto::Ciphertext>& key) override;

    virtual ~EllipticCurve() = default;

protected:
    OTData key_;
    std::unique_ptr<proto::Ciphertext> encrypted_key_{nullptr};
    std::shared_ptr<proto::HDPath> path_{nullptr};
    std::unique_ptr<proto::Ciphertext> chain_code_{nullptr};

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

#if OT_CRYPTO_SUPPORTED_KEY_HD
    Bip32Fingerprint parent_;
#endif

    EllipticCurve* clone() const override final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::tuple<bool, Bip32Depth, Bip32Index> get_params() const;
#endif

    EllipticCurve() = delete;
    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
