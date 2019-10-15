// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::crypto::key::implementation
{
class HD : virtual public key::HD, public EllipticCurve
{
public:
    OTData Chaincode(const PasswordPrompt& reason) const override;
    int Depth() const override;
    Bip32Fingerprint Fingerprint(const PasswordPrompt& reason) const override;
    const std::string Path() const override;
    bool Path(proto::HDPath& output) const override;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    std::string Xprv(const PasswordPrompt& reason) const override;
    std::string Xpub(const PasswordPrompt& reason) const override;

protected:
    std::shared_ptr<proto::HDPath> path_{nullptr};
    std::unique_ptr<proto::Ciphertext> chain_code_{nullptr};

    void erase_private_data() override;

    HD(const api::internal::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey,
       const PasswordPrompt& reason)
    noexcept;
    HD(const api::internal::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKeyType keyType,
       const proto::KeyRole role,
       const VersionNumber version,
       const PasswordPrompt& reason)
    noexcept(false);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    HD(const api::internal::Core& api,
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
       const PasswordPrompt& reason)
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
