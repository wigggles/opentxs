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
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;

    ~Secp256k1() = default;

private:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using ot_super = HD;
#else
    using ot_super = EllipticCurve;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    friend key::Asymmetric;
    friend opentxs::Factory;

    explicit Secp256k1(const VersionNumber version) noexcept;
    Secp256k1(const proto::KeyRole role, const VersionNumber version) noexcept;
    Secp256k1(const proto::AsymmetricKey& serializedKey) noexcept;
    Secp256k1(const String& publicKey, const VersionNumber version) noexcept;
    Secp256k1() = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
