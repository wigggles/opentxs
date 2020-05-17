// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "crypto/key/Ed25519.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "Factory.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/protobuf/Enums.pb.h"
#include "util/Sodium.hpp"

namespace opentxs
{
auto Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input) -> crypto::key::Ed25519*
{
    try {

        return new crypto::key::implementation::Ed25519(api, ecdsa, input);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) -> crypto::key::Ed25519*
{
    try {

        return new crypto::key::implementation::Ed25519(
            api, ecdsa, input, version, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const Secret& privateKey,
    const Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) -> crypto::key::Ed25519*
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
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Ed25519::Ed25519(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : ot_super(api, ecdsa, serializedKey)
{
}

Ed25519::Ed25519(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(api, ecdsa, proto::AKEYTYPE_ED25519, role, version, reason)
{
}

#if OT_CRYPTO_WITH_BIP32
Ed25519::Ed25519(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const Secret& privateKey,
    const Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const proto::KeyRole role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
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
#endif  // OT_CRYPTO_WITH_BIP32

Ed25519::Ed25519(const Ed25519& rhs) noexcept
    : key::Ed25519()
    , ot_super(rhs)
{
}

auto Ed25519::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const PasswordPrompt& reason) const noexcept -> bool
{
    if (false == HasPublic()) { return false; }
    if (false == HasPrivate()) { return false; }

    return sodium::ToCurveKeypair(
        PrivateKey(reason),
        PublicKey(),
        privateKey.WriteInto(Secret::Mode::Mem),
        publicKey.WriteInto());
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
