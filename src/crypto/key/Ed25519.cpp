// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/Sodium.hpp"

#include "crypto/key/HD.hpp"
#include "util/Sodium.hpp"

#include "Ed25519.hpp"

namespace opentxs
{
crypto::key::Ed25519* Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input)
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

crypto::key::Ed25519* Factory::Ed25519Key(
    const api::internal::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::KeyRole input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
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
    const OTPassword& privateKey,
    const OTPassword& chainCode,
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
    OTPassword& privateKey,
    const PasswordPrompt& reason) const noexcept -> bool
{
    if (false == HasPublic()) { return false; }
    if (false == HasPrivate()) { return false; }

    return sodium::ToCurveKeypair(
        PrivateKey(reason),
        PublicKey(),
        privateKey.WriteInto(OTPassword::Mode::Mem),
        publicKey.WriteInto());
}
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
