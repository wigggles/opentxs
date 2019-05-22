// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "EllipticCurve.hpp"

namespace opentxs::crypto::key::implementation
{
class Ed25519 final : virtual public key::Ed25519,
                      public implementation::EllipticCurve
{
public:
    NymParameterType CreateType() const override
    {
        return NymParameterType::ED25519;
    }
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;
    bool hasCapability(const NymCapability& capability) const override;

    ~Ed25519() = default;

private:
    using ot_super = implementation::EllipticCurve;

    friend opentxs::Factory;
    friend LowLevelKeyGenerator;

    explicit Ed25519(const VersionNumber version) noexcept;
    explicit Ed25519(const proto::AsymmetricKey& serializedKey) noexcept;
    Ed25519(const proto::KeyRole role, const VersionNumber version) noexcept;
    Ed25519(const String& publicKey, const VersionNumber version) noexcept;
    Ed25519() = delete;
    Ed25519(const Ed25519&) = delete;
    Ed25519(Ed25519&&) = delete;
    Ed25519& operator=(const Ed25519&) = delete;
    Ed25519& operator=(Ed25519&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
