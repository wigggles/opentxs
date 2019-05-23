// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_ASYMMETRIC_HPP
#define OPENTXS_API_CRYPTO_ASYMMETRIC_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/Bip32.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric
{
public:
    using Key = std::unique_ptr<opentxs::crypto::key::Asymmetric>;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    using HDKey = std::unique_ptr<opentxs::crypto::key::HD>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD

    EXPORT virtual const opentxs::crypto::Bip32& BIP32() const = 0;
    EXPORT virtual const crypto::Encode& Encode() const = 0;
    EXPORT virtual const crypto::Hash& Hash() const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual HDKey InstantiateHDKey(
        const proto::AsymmetricKey& serialized) const = 0;
    EXPORT virtual HDKey InstantiateKey(
        const proto::AsymmetricKeyType type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual Key InstantiateKey(
        const proto::AsymmetricKey& serialized) const = 0;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual HDKey NewHDKey(
        const std::string& seedID,
        const OTPassword& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    EXPORT virtual Key NewKey(
        const NymParameters& params,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const = 0;
    EXPORT virtual const crypto::Symmetric& Symmetric() const = 0;

    EXPORT virtual ~Asymmetric() = default;

protected:
    Asymmetric() = default;

private:
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    Asymmetric& operator=(const Asymmetric&) = delete;
    Asymmetric& operator=(Asymmetric&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
