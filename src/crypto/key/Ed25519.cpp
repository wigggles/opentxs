// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/Sodium.hpp"

#include "crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "crypto/key/HD.hpp"
#endif

#include "Ed25519.hpp"

namespace opentxs
{
crypto::key::Ed25519* Factory::Ed25519Key(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input)
{
    return new crypto::key::implementation::Ed25519(crypto, ecdsa, input);
}

crypto::key::Ed25519* Factory::Ed25519Key(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version)
{
    return new crypto::key::implementation::Ed25519(
        crypto, ecdsa, input, version);
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
crypto::key::Ed25519* Factory::Ed25519Key(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const OTPassword& privateKey,
    const OTPassword& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version)
{
    const auto reason = OTPasswordData(__FUNCTION__);
    auto sessionKey = crypto.Symmetric().Key(__FUNCTION__);

    return new crypto::key::implementation::Ed25519(
        crypto,
        ecdsa,
        privateKey,
        chainCode,
        publicKey,
        path,
        parent,
        role,
        version,
        sessionKey,
        reason);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Ed25519::Ed25519(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    : ot_super(crypto, ecdsa, serializedKey)
{
}

Ed25519::Ed25519(
    const api::crypto::Asymmetric& crypto,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : ot_super(crypto, ecdsa, proto::AKEYTYPE_ED25519, role, version)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Ed25519::Ed25519(
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
    const OTPasswordData& reason) noexcept
    : ot_super(
          crypto,
          ecdsa,
          proto::AKEYTYPE_ED25519,
          privateKey,
          chainCode,
          publicKey,
          path,
          parent,
          role,
          version,
          sessionKey,
          reason)
{
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

Ed25519::Ed25519(const Ed25519& rhs) noexcept
    : key::Ed25519()
    , ot_super(rhs)
{
}

bool Ed25519::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::AUTHENTICATE_CONNECTION): {

            return true;
        }
        default: {
            return ot_super::hasCapability(capability);
        }
    }
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
