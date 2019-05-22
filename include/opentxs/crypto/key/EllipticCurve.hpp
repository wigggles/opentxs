// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_ELLIPTICCURVE_HPP
#define OPENTXS_CRYPTO_KEY_ELLIPTICCURVE_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/Asymmetric.hpp"

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
    EXPORT static const VersionNumber DefaultVersion;
    EXPORT static const VersionNumber MaxVersion;

    EXPORT virtual const crypto::EcdsaProvider& ECDSA() const = 0;
    EXPORT virtual bool GetKey(Data& key) const = 0;
    EXPORT virtual bool GetKey(proto::Ciphertext& key) const = 0;
    using Asymmetric::GetPublicKey;
    EXPORT virtual bool GetPublicKey(Data& key) const = 0;
    EXPORT virtual OTData PrivateKey() const = 0;
    EXPORT virtual OTData PublicKey() const = 0;

    EXPORT virtual bool SetKey(const Data& key) = 0;
    EXPORT virtual bool SetKey(std::unique_ptr<proto::Ciphertext>& key) = 0;

    EXPORT virtual ~EllipticCurve() = default;

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
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#endif
