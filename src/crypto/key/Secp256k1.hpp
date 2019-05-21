// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "EllipticCurve.hpp"

namespace opentxs::crypto::key::implementation
{
class Secp256k1 final : virtual public key::Secp256k1, public EllipticCurve
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
    using ot_super = EllipticCurve;

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
