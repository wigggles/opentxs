// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_ELLIPTICCURVE_HPP
#define OPENTXS_CRYPTO_KEY_ELLIPTICCURVE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"

#include <memory>

namespace opentxs
{
namespace crypto
{
namespace key
{
class EllipticCurve : virtual public Asymmetric
{
public:
    OPENTXS_EXPORT static const VersionNumber DefaultVersion;
    OPENTXS_EXPORT static const VersionNumber MaxVersion;

    OPENTXS_EXPORT virtual std::unique_ptr<EllipticCurve> asPublicEC() const
        noexcept = 0;

    OPENTXS_EXPORT ~EllipticCurve() override = default;

protected:
    EllipticCurve() = default;

private:
    EllipticCurve(const EllipticCurve&) = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    EllipticCurve& operator=(const EllipticCurve&) = delete;
    EllipticCurve& operator=(EllipticCurve&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
