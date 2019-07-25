// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::crypto::implementation
{
class Asymmetric final : virtual public api::crypto::internal::Asymmetric
{
public:
#if OT_CRYPTO_SUPPORTED_KEY_HD
    ECKey InstantiateECKey(
        const proto::AsymmetricKey& serialized,
        const PasswordPrompt& reason) const final;
    HDKey InstantiateHDKey(
        const proto::AsymmetricKey& serialized,
        const PasswordPrompt& reason) const final;
    HDKey InstantiateKey(
        const proto::AsymmetricKeyType type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Key InstantiateKey(
        const proto::AsymmetricKey& serialized,
        const PasswordPrompt& reason) const final;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    HDKey NewHDKey(
        const std::string& seedID,
        const OTPassword& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role,
        const VersionNumber version) const final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    Key NewKey(
        const NymParameters& params,
        const proto::KeyRole role,
        const VersionNumber version) const final;

    ~Asymmetric() final = default;

private:
    friend opentxs::Factory;

    using TypeMap = std::map<EcdsaCurve, proto::AsymmetricKeyType>;

    static const VersionNumber serialized_path_version_;
    static const TypeMap curve_to_key_type_;

    const api::internal::Core& api_;

#if OT_CRYPTO_SUPPORTED_KEY_HD
    static proto::HDPath serialize_path(
        const std::string& seedID,
        const opentxs::crypto::Bip32::Path& children);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    Asymmetric(const api::internal::Core& api);
    Asymmetric() = delete;
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace opentxs::api::crypto::implementation
