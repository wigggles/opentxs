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
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetChild(
        const proto::AsymmetricKey& parent,
        const std::uint32_t index) const = 0;
    EXPORT virtual std::shared_ptr<proto::AsymmetricKey> GetHDKey(
        const EcdsaCurve& curve,
        const OTPassword& seed,
        proto::HDPath& path,
        const VersionNumber version) const = 0;
    EXPORT virtual std::string SeedToFingerprint(
        const EcdsaCurve& curve,
        const OTPassword& seed) const = 0;
};
}  // namespace crypto
}  // namespace opentxs
#endif
