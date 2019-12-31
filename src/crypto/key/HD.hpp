// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "EllipticCurve.hpp"

namespace opentxs::crypto::key::implementation
{
class HD : virtual public key::HD, public EllipticCurve
{
public:
    ReadView Chaincode(const PasswordPrompt& reason) const noexcept final;
    int Depth() const noexcept final;
    Bip32Fingerprint Fingerprint() const noexcept final;
    const std::string Path() const noexcept final;
    bool Path(proto::HDPath& output) const noexcept final;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const noexcept final;
    std::string Xprv(const PasswordPrompt& reason) const noexcept final;
    std::string Xpub(const PasswordPrompt& reason) const noexcept final;

protected:
    void erase_private_data() final;

    HD(const api::internal::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey)
    noexcept(false);
    HD(const api::internal::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKeyType keyType,
       const proto::KeyRole role,
       const VersionNumber version,
       const PasswordPrompt& reason)
    noexcept(false);
#if OT_CRYPTO_WITH_BIP32
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
    noexcept(false);
#endif  // OT_CRYPTO_WITH_BIP32
    HD(const HD&) noexcept;

private:
    const std::shared_ptr<const proto::HDPath> path_;
    const std::unique_ptr<const proto::Ciphertext> chain_code_;
    mutable OTPassword plaintext_chain_code_;
    const Bip32Fingerprint parent_;

    auto get_params() const noexcept
        -> std::tuple<bool, Bip32Depth, Bip32Index>;

    HD() = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
