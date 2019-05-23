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
    const crypto::EcdsaProvider& ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const crypto::AsymmetricProvider& rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const crypto::EcdsaProvider& secp256k1
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
const VersionNumber Asymmetric::serialized_path_version_{1};

const Asymmetric::TypeMap Asymmetric::curve_to_key_type_{
    {EcdsaCurve::ERROR, proto::AKEYTYPE_ERROR},
    {EcdsaCurve::SECP256K1, proto::AKEYTYPE_SECP256K1},
    {EcdsaCurve::ED25519, proto::AKEYTYPE_ED25519},
};

Asymmetric::Asymmetric(
    const crypto::Encode& encode,
    const crypto::Hash& hash,
    const crypto::Util& util,
    const crypto::Symmetric& symmetric,
#if OT_CRYPTO_SUPPORTED_KEY_HD
    const opentxs::crypto::Bip32& bip32,
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    const opentxs::crypto::EcdsaProvider& ed25519,
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const opentxs::crypto::AsymmetricProvider& rsa,
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const opentxs::crypto::EcdsaProvider& secp256k1
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
            return HDKey{
                opentxs::Factory::Ed25519Key(*this, ed25519_, serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return HDKey{
                opentxs::Factory::Secp256k1Key(*this, secp256k1_, serialized)};
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

Asymmetric::HDKey Asymmetric::InstantiateKey(
    const proto::AsymmetricKeyType type,
    const std::string& seedID,
    const opentxs::crypto::Bip32::Key& serialized,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    const auto& [privkey, ccode, pubkey, path, parent] = serialized;

    switch (type) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return HDKey{opentxs::Factory::Ed25519Key(
                *this,
                ed25519_,
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return HDKey{opentxs::Factory::Secp256k1Key(
                *this,
                secp256k1_,
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version)};
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
            return Key{
                opentxs::Factory::Ed25519Key(*this, ed25519_, serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return Key{
                opentxs::Factory::Secp256k1Key(*this, secp256k1_, serialized)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return Key{opentxs::Factory::RSAKey(*this, rsa_, serialized)};
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
    const std::string& seedID,
    const OTPassword& seed,
    const EcdsaCurve& curve,
    const opentxs::crypto::Bip32::Path& path,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    return InstantiateKey(
        curve_to_key_type_.at(curve),
        seedID,
        bip32_.DeriveKey(hash_, curve, seed, path),
        role,
        version);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

Asymmetric::Key Asymmetric::NewKey(
    const NymParameters& params,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    switch (params.AsymmetricKeyType()) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return Key{
                opentxs::Factory::Ed25519Key(*this, ed25519_, role, version)};
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return Key{opentxs::Factory::Secp256k1Key(
                *this, secp256k1_, role, version)};
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return Key{opentxs::Factory::RSAKey(*this, rsa_, role)};
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with support for this key "
                "type.")
                .Flush();
        }
    }

    return {};
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
proto::HDPath Asymmetric::serialize_path(
    const std::string& seedID,
    const opentxs::crypto::Bip32::Path& children)
{
    proto::HDPath output;
    output.set_version(serialized_path_version_);
    output.set_root(seedID);

    for (const auto& index : children) { output.add_child(index); }

    return output;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
}  // namespace opentxs::api::crypto::implementation
