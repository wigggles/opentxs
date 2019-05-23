// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1

namespace opentxs::crypto::key::implementation
{
class Secp256k1 final : virtual public key::Secp256k1,
#if OT_CRYPTO_SUPPORTED_KEY_HD
                        public implementation::HD
#else
                        public implementation::EllipticCurve
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
{
public:
    NymParameterType CreateType() const override
    {
        return NymParameterType::SECP256K1;
    }

    ~Secp256k1() = default;

private:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using ot_super = HD;
#else
    using ot_super = EllipticCurve;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    friend key::Asymmetric;
    friend opentxs::Factory;

    Secp256k1* clone() const override final { return new Secp256k1(*this); }
    std::shared_ptr<proto::AsymmetricKey> get_public() const override
    {
        return serialize_public(clone());
    }

    Secp256k1(
        const api::crypto::Asymmetric& crypto,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) noexcept;
    Secp256k1(
        const api::crypto::Asymmetric& crypto,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version) noexcept;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Secp256k1(
        const api::crypto::Asymmetric& crypto,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const OTPasswordData& reason) noexcept;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) noexcept;
    Secp256k1(Secp256k1&&) = delete;
    Secp256k1& operator=(const Secp256k1&) = delete;
    Secp256k1& operator=(Secp256k1&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
