// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif

#include "crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "crypto/key/HD.hpp"
#endif

#include "Secp256k1.hpp"

namespace opentxs
{
crypto::key::Secp256k1* Factory::Secp256k1Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input,
    const opentxs::PasswordPrompt& reason)
{
    return new crypto::key::implementation::Secp256k1(
        api, ecdsa, input, reason);
}

crypto::key::Secp256k1* Factory::Secp256k1Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version)
{
    return new crypto::key::implementation::Secp256k1(
        api, ecdsa, input, version);
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
crypto::key::Secp256k1* Factory::Secp256k1Key(
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

    return new crypto::key::implementation::Secp256k1(
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
Secp256k1::Secp256k1(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    const opentxs::PasswordPrompt& reason) noexcept
    : ot_super(api, ecdsa, serializedKey, reason)
{
}

Secp256k1::Secp256k1(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : ot_super(api, ecdsa, proto::AKEYTYPE_SECP256K1, role, version)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Secp256k1::Secp256k1(
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
    const opentxs::PasswordPrompt& reason) noexcept
    : ot_super(
          api,
          ecdsa,
          proto::AKEYTYPE_SECP256K1,
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

Secp256k1::Secp256k1(const Secp256k1& rhs) noexcept
    : key::Secp256k1()
    , ot_super(rhs)
{
}
}  // namespace opentxs::crypto::key::implementation
#endif
