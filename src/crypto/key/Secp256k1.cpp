// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "crypto/key/Secp256k1.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "Factory.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
auto Factory::Secp256k1Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input) -> crypto::key::Secp256k1*
{
    try {

        return new crypto::key::implementation::Secp256k1(api, ecdsa, input);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::Secp256k1Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) -> crypto::key::Secp256k1*
{
    try {

        return new crypto::key::implementation::Secp256k1(
            api, ecdsa, input, version, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Factory::Secp256k1Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const OTPassword& privateKey,
    const OTPassword& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) -> crypto::key::Secp256k1*
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
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Secp256k1::Secp256k1(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : ot_super(api, ecdsa, serializedKey)
{
}

Secp256k1::Secp256k1(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(api, ecdsa, proto::AKEYTYPE_SECP256K1, role, version, reason)
{
}

#if OT_CRYPTO_WITH_BIP32
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
    const opentxs::PasswordPrompt& reason) noexcept(false)
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
#endif  // OT_CRYPTO_WITH_BIP32

Secp256k1::Secp256k1(const Secp256k1& rhs) noexcept
    : key::Secp256k1()
    , ot_super(rhs)
{
}
}  // namespace opentxs::crypto::key::implementation
#endif
