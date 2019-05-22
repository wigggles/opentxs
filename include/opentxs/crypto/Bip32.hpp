// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP32_HPP
#define OPENTXS_CRYPTO_BIP32_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
namespace crypto
{
std::string Print(const proto::HDPath& node);

class Bip32
{
public:
    EXPORT virtual bool DeserializePrivate(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        OTPassword& key) const = 0;
    EXPORT virtual bool DeserializePublic(
        const std::string& serialized,
        Bip32Network& network,
        Bip32Depth& depth,
        Bip32Fingerprint& parent,
        Bip32Index& index,
        Data& chainCode,
        Data& key) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetChild(
        const proto::AsymmetricKey& parent,
        const Bip32Index index) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path,
        const VersionNumber version) const = 0;
    EXPORT virtual std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
    EXPORT virtual std::string SerializePrivate(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const OTPassword& key) const = 0;
    EXPORT virtual std::string SerializePublic(
        const Bip32Network network,
        const Bip32Depth depth,
        const Bip32Fingerprint parent,
        const Bip32Index index,
        const Data& chainCode,
        const Data& key) const = 0;
};
}  // namespace crypto
}  // namespace opentxs
#endif
