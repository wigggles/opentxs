// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_CRYPTO_SUPPORTED_KEY_ED25519

namespace opentxs::crypto::key::implementation
{
class Ed25519 final : virtual public key::Ed25519,
#if OT_CRYPTO_SUPPORTED_KEY_HD
                      public implementation::HD
#else
                      public implementation::EllipticCurve
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
{
public:
    NymParameterType CreateType() const override
    {
        return NymParameterType::ED25519;
    }
    bool hasCapability(const NymCapability& capability) const override;

    ~Ed25519() = default;

private:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using ot_super = HD;
#else
    using ot_super = EllipticCurve;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    friend opentxs::Factory;
    friend LowLevelKeyGenerator;

    Ed25519* clone() const override final { return new Ed25519(*this); }
    std::shared_ptr<proto::AsymmetricKey> get_public() const override
    {
        return serialize_public(clone());
    }

    Ed25519(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        const PasswordPrompt& reason) noexcept;
    Ed25519(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version) noexcept;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Ed25519(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const OTPassword& privateKey,
        const OTPassword& chainCode,
        const Data& publicKey,
        const proto::HDPath& path,
        const Bip32Fingerprint parent,
        const proto::KeyRole role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Ed25519() = delete;
    Ed25519(const Ed25519&) noexcept;
    Ed25519(Ed25519&&) = delete;
    Ed25519& operator=(const Ed25519&) = delete;
    Ed25519& operator=(Ed25519&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
