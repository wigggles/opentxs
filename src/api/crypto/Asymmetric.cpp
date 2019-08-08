// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
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

#include "internal/api/crypto/Crypto.hpp"
#include "internal/api/Api.hpp"
#include "crypto/key/Null.hpp"
#include "Factory.hpp"

#include "Asymmetric.hpp"

#define OT_METHOD "opentxs::api::crypto::implementation::Asymmetric::"

namespace opentxs
{
api::crypto::internal::Asymmetric* Factory::AsymmetricAPI(
    const api::internal::Core& api)
{
    return new api::crypto::implementation::Asymmetric(api);
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

Asymmetric::Asymmetric(const api::internal::Core& api)
    : api_(api)
{
}

Asymmetric::ECKey Asymmetric::InstantiateECKey(
    const proto::AsymmetricKey& serialized,
    const PasswordPrompt& reason) const
{
    const auto keyType = serialized.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return ECKey{opentxs::Factory::Ed25519Key(
                api_, api_.Crypto().ED25519(), serialized, reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return ECKey{opentxs::Factory::Secp256k1Key(
                api_, api_.Crypto().SECP256K1(), serialized, reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_LEGACY): {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong key type (RSA)")
                .Flush();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with support for this key "
                "type.")
                .Flush();
        }
    }

    return ECKey{new opentxs::crypto::key::implementation::NullHD};
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
Asymmetric::HDKey Asymmetric::InstantiateHDKey(
    const proto::AsymmetricKey& serialized,
    const PasswordPrompt& reason) const
{
    const auto keyType = serialized.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return HDKey{opentxs::Factory::Ed25519Key(
                api_, api_.Crypto().ED25519(), serialized, reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return HDKey{opentxs::Factory::Secp256k1Key(
                api_, api_.Crypto().SECP256K1(), serialized, reason)};
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
    const PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    const auto& [privkey, ccode, pubkey, path, parent] = serialized;

    switch (type) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return HDKey{opentxs::Factory::Ed25519Key(
                api_,
                api_.Crypto().ED25519(),
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version,
                reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return HDKey{opentxs::Factory::Secp256k1Key(
                api_,
                api_.Crypto().SECP256K1(),
                privkey,
                ccode,
                pubkey,
                serialize_path(seedID, path),
                parent,
                role,
                version,
                reason)};
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
    const proto::AsymmetricKey& serialized,
    const PasswordPrompt& reason) const
{
    const auto keyType = serialized.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return Key{opentxs::Factory::Ed25519Key(
                api_, api_.Crypto().ED25519(), serialized, reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return Key{opentxs::Factory::Secp256k1Key(
                api_, api_.Crypto().SECP256K1(), serialized, reason)};
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return Key{opentxs::Factory::RSAKey(
                api_, api_.Crypto().RSA(), serialized)};
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
    const PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const
{
    return InstantiateKey(
        curve_to_key_type_.at(curve),
        seedID,
        api_.Crypto().BIP32().DeriveKey(
            api_.Crypto().Hash(), curve, seed, path),
        reason,
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
            return Key{opentxs::Factory::Ed25519Key(
                api_, api_.Crypto().ED25519(), role, version)};
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return Key{opentxs::Factory::Secp256k1Key(
                api_, api_.Crypto().SECP256K1(), role, version)};
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return Key{
                opentxs::Factory::RSAKey(api_, api_.Crypto().RSA(), role)};
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
