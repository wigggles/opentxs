// Copyright (c) 2019 The Open-Transactions developers
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
    NymParameterType CreateType() const final
    {
        return NymParameterType::secp256k1;
    }

    ~Secp256k1() final = default;

private:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using ot_super = HD;
#else
    using ot_super = EllipticCurve;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    friend key::Asymmetric;
    friend opentxs::Factory;

    Secp256k1* clone() const final { return new Secp256k1(*this); }
    Secp256k1* clone_ec() const final { return clone(); }
    std::shared_ptr<proto::AsymmetricKey> get_public() const final
    {
        return serialize_public(clone());
    }

    Secp256k1(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey,
        const PasswordPrompt& reason) noexcept;
    Secp256k1(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::KeyRole role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    Secp256k1(
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
    Secp256k1() = delete;
    Secp256k1(const Secp256k1&) noexcept;
    Secp256k1(Secp256k1&&) = delete;
    Secp256k1& operator=(const Secp256k1&) = delete;
    Secp256k1& operator=(Secp256k1&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
