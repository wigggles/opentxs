// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Asymmetric final : virtual public api::crypto::Asymmetric
{
public:
    const opentxs::crypto::Bip32& BIP32() const override { return bip32_; }
    const crypto::Encode& Encode() const override { return encode_; }
    const crypto::Hash& Hash() const override { return hash_; }
#if OT_CRYPTO_SUPPORTED_KEY_HD
    HDKey InstantiateHDKey(
        const proto::AsymmetricKey& serialized) const override;
    HDKey InstantiateKey(
        const proto::AsymmetricKeyType type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const proto::KeyRole role,
        const VersionNumber version) const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Key InstantiateKey(const proto::AsymmetricKey& serialized) const override;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    HDKey NewHDKey(
        const std::string& seedID,
        const OTPassword& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const proto::KeyRole role,
        const VersionNumber version) const override;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Key NewKey(
        const NymParameters& params,
        const proto::KeyRole role,
        const VersionNumber version) const override;
    const crypto::Symmetric& Symmetric() const override { return symmetric_; }

    ~Asymmetric() override = default;

private:
    friend opentxs::Factory;

    using TypeMap = std::map<EcdsaCurve, proto::AsymmetricKeyType>;

    static const VersionNumber serialized_path_version_;
    static const TypeMap curve_to_key_type_;

    const crypto::Encode& encode_;
    const crypto::Hash& hash_;
    const crypto::Util& util_;
    const crypto::Symmetric& symmetric_;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const opentxs::crypto::Bip32& bip32_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    const opentxs::crypto::EcdsaProvider& ed25519_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const opentxs::crypto::AsymmetricProvider& rsa_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const opentxs::crypto::EcdsaProvider& secp256k1_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

#if OT_CRYPTO_SUPPORTED_KEY_HD
    static proto::HDPath serialize_path(
        const std::string& seedID,
        const opentxs::crypto::Bip32::Path& children);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    Asymmetric(
        const crypto::Encode& encode,
        const crypto::Hash& hash,
        const crypto::Util& util,
        const crypto::Symmetric& symmetric,
#if OT_CRYPTO_SUPPORTED_KEY_HD
        const opentxs::crypto::Bip32& bip32,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        const opentxs::crypto::EcdsaProvider& ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        const opentxs::crypto::AsymmetricProvider& rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        const opentxs::crypto::EcdsaProvider& secp256k1
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    );
    Asymmetric() = delete;
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
