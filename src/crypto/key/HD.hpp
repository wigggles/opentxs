// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::crypto::key::implementation
{
class HD : virtual public key::HD, public EllipticCurve
{
public:
    OTData Chaincode() const override;
    int Depth() const override;
    Bip32Fingerprint Fingerprint() const override;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    std::string Xprv() const override;
    std::string Xpub() const override;

protected:
    std::shared_ptr<proto::HDPath> path_{nullptr};
    std::unique_ptr<proto::Ciphertext> chain_code_{nullptr};

    void erase_private_data() override;

    HD(const api::crypto::Asymmetric& crypto,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey)
    noexcept;
    HD(const api::crypto::Asymmetric& crypto,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKeyType keyType,
       const proto::KeyRole role,
       const VersionNumber version)
    noexcept;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    HD(const api::crypto::Asymmetric& crypto,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKeyType keyType,
       const OTPassword& privateKey,
       const OTPassword& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const proto::KeyRole role,
       const VersionNumber version,
       key::Symmetric& sessionKey,
       const OTPasswordData& reason)
    noexcept;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    HD(const HD&) noexcept;

private:
    Bip32Fingerprint parent_;

    std::tuple<bool, Bip32Depth, Bip32Index> get_params() const;

    HD() = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
