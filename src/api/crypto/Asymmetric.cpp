// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/HD.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/util/Timer.hpp"  // TODO asymmetric key implementation
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Ed25519.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/RSA.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/Bip32.hpp"

#include "crypto/key/Asymmetric.hpp"  // TODO asymmetric key implementation
#include "crypto/key/Null.hpp"
#include "Factory.hpp"

#include "Asymmetric.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Asymmetric::"

namespace opentxs
{
api::crypto::Asymmetric* Factory::AsymmetricAPI(
    const api::crypto::Encode& encode,
    const api::crypto::Hash& hash,
    const api::crypto::Util& util,
    const api::crypto::Symmetric& symmetric,
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const crypto::Bip32& bip32,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    const crypto::AsymmetricProvider& ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const crypto::AsymmetricProvider& rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const crypto::AsymmetricProvider& secp256k1
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
)
{
    return new api::crypto::implementation::Asymmetric(
        encode,
        hash,
        util,
        symmetric,
#if OT_CRYPTO_SUPPORTED_KEY_HD
        bip32,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        secp256k1
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    );
}
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
Asymmetric::Asymmetric(
    const crypto::Encode& encode,
    const crypto::Hash& hash,
    const crypto::Util& util,
    const crypto::Symmetric& symmetric,
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const opentxs::crypto::Bip32& bip32,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    const opentxs::crypto::AsymmetricProvider& ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const opentxs::crypto::AsymmetricProvider& rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const opentxs::crypto::AsymmetricProvider& secp256k1
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    )
    : encode_(encode)
    , hash_(hash)
    , util_(util)
    , symmetric_(symmetric)
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , bip32_(bip32)
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    , ed25519_(ed25519)
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    , rsa_(rsa)
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    , secp256k1_(secp256k1)
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
{
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Asymmetric::HDKey Asymmetric::InstantiateHDKey(
    const proto::AsymmetricKey& serialized) const
{
    const auto keyType = serialized.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return HDKey{opentxs::Factory::Ed25519Key(serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return HDKey{opentxs::Factory::Secp256k1Key(serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong key type (RSA)")
                .Flush();
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with support for this key "
                "type.")
                .Flush();
        }
    }

    return HDKey{new opentxs::crypto::key::implementation::NullHD};
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

Asymmetric::Key Asymmetric::InstantiateKey(
    const proto::AsymmetricKey& serialized) const
{
    const auto keyType = serialized.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return Key{opentxs::Factory::Ed25519Key(serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return Key{opentxs::Factory::Secp256k1Key(serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return Key{opentxs::Factory::RSAKey(serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with support for this key "
                "type.")
                .Flush();
        }
    }

    return Key{new opentxs::crypto::key::implementation::Null};
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Asymmetric::HDKey Asymmetric::NewHDKey(
    const OTPassword& seed,
    const EcdsaCurve& curve,
    const Path& path,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    proto::HDPath ppath;

    for (const auto& index : path) { ppath.add_child(index); }

    auto pSerialized = bip32_.GetHDKey(curve, seed, ppath, version);

    if (false == bool(pSerialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive key").Flush();

        return {};
    }

    auto& serialized = *pSerialized;
    serialized.set_role(role);

    return InstantiateHDKey(serialized);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

Asymmetric::Key Asymmetric::NewKey(
    const NymParameters& params,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    const auto keyType = params.AsymmetricKeyType();

    return Key{opentxs::crypto::key::implementation::Asymmetric::KeyFactory(
        keyType, role, version)};
}
}  // namespace opentxs::api::crypto::implementation
