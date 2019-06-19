// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/util/Timer.hpp"
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
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input,
    const opentxs::PasswordPrompt& reason)
{
    return new crypto::key::implementation::Ed25519(api, ecdsa, input, reason);
}

crypto::key::Ed25519* Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version)
{
    return new crypto::key::implementation::Ed25519(api, ecdsa, input, version);
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
crypto::key::Ed25519* Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const OTPassword& privateKey,
    const OTPassword& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    auto sessionKey = api.Symmetric().Key(reason);

    return new crypto::key::implementation::Ed25519(
        api,
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
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    const PasswordPrompt& reason) noexcept
    : ot_super(api, ecdsa, serializedKey, reason)
{
}

Ed25519::Ed25519(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : ot_super(api, ecdsa, proto::AKEYTYPE_ED25519, role, version)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Ed25519::Ed25519(
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
    const PasswordPrompt& reason) noexcept
    : ot_super(
          api,
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
