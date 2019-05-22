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

    explicit HD(const proto::AsymmetricKey& serializedKey) noexcept;
    HD(const proto::AsymmetricKeyType keyType,
       const proto::KeyRole role,
       const VersionNumber version)
    noexcept;
    HD(const proto::AsymmetricKeyType keyType,
       const String& publicKey,
       const VersionNumber version)
    noexcept;

private:
    Bip32Fingerprint parent_;

    HD* clone() const override final;
    std::tuple<bool, Bip32Depth, Bip32Index> get_params() const;

    HD() = delete;
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
