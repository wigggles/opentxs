// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/OT.hpp"

#include "crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "crypto/key/HD.hpp"
#endif

#include "Secp256k1.hpp"

namespace opentxs
{
crypto::key::Secp256k1* Factory::Secp256k1Key(const proto::AsymmetricKey& input)
{
    return new crypto::key::implementation::Secp256k1(input);
}

crypto::key::Secp256k1* Factory::Secp256k1Key(
    const String& input,
    const VersionNumber version)
{
    return new crypto::key::implementation::Secp256k1(input, version);
}

crypto::key::Secp256k1* Factory::Secp256k1Key(
    const proto::KeyRole input,
    const VersionNumber version)
{
    return new crypto::key::implementation::Secp256k1(input, version);
}
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
Secp256k1::Secp256k1(const VersionNumber version) noexcept
    : ot_super(proto::AKEYTYPE_SECP256K1, proto::KEYROLE_ERROR, version)
{
}

Secp256k1::Secp256k1(
    const proto::KeyRole role,
    const VersionNumber version) noexcept
    : ot_super(proto::AKEYTYPE_SECP256K1, role, version)
{
}

Secp256k1::Secp256k1(const proto::AsymmetricKey& serializedKey) noexcept
    : ot_super(serializedKey)
{
}

Secp256k1::Secp256k1(
    const String& publicKey,
    const VersionNumber version) noexcept
    : ot_super(proto::AKEYTYPE_SECP256K1, publicKey, version)
{
}

const crypto::EcdsaProvider& Secp256k1::ECDSA() const
{
    return dynamic_cast<const crypto::EcdsaProvider&>(engine());
}

const crypto::AsymmetricProvider& Secp256k1::engine() const

{
    return OT::App().Crypto().SECP256K1();
}
}  // namespace opentxs::crypto::key::implementation
#endif
