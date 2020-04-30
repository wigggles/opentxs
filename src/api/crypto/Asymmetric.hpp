// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/crypto/Symmetric.cpp"

#pragma once

#include <map>
#include <memory>
#include <string>

#include "internal/api/crypto/Crypto.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class Factory;
class NymParameters;
class OTPassword;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::crypto::implementation
{
class Asymmetric final : virtual public api::crypto::internal::Asymmetric
{
public:
    ECKey InstantiateECKey(const proto::AsymmetricKey& serialized) const final;
    HDKey InstantiateHDKey(const proto::AsymmetricKey& serialized) const final;
#if OT_CRYPTO_WITH_BIP32
    HDKey InstantiateKey(
        const proto::AsymmetricKeyType type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;
#endif  // OT_CRYPTO_WITH_BIP32
    Key InstantiateKey(const proto::AsymmetricKey& serialized) const final;
#if OT_CRYPTO_WITH_BIP32
    HDKey NewHDKey(
        const std::string& seedID,
        const OTPassword& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    Secp256k1Key NewSecp256k1Key(
        const std::string& seedID,
        const OTPassword& seed,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::Secp256k1::DefaultVersion) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#endif  // OT_CRYPTO_WITH_BIP32
    Key NewKey(
        const NymParameters& params,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;

    ~Asymmetric() final = default;

private:
    friend opentxs::Factory;

    using TypeMap = std::map<EcdsaCurve, proto::AsymmetricKeyType>;

    static const VersionNumber serialized_path_version_;
    static const TypeMap curve_to_key_type_;

    const api::internal::Core& api_;

#if OT_CRYPTO_WITH_BIP32
    static proto::HDPath serialize_path(
        const std::string& seedID,
        const opentxs::crypto::Bip32::Path& children);
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_WITH_BIP32
    template <typename ReturnType, typename NullType>
    auto instantiate_hd_key(
        const proto::AsymmetricKeyType type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const noexcept
        -> std::unique_ptr<ReturnType>;
#endif  // OT_CRYPTO_WITH_BIP32
    template <typename ReturnType, typename NullType>
    auto instantiate_serialized_key(const proto::AsymmetricKey& serialized)
        const noexcept -> std::unique_ptr<ReturnType>;

    Asymmetric(const api::internal::Core& api);
    Asymmetric() = delete;
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
