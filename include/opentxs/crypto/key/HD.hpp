// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_HD_HPP
#define OPENTXS_CRYPTO_KEY_HD_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/Types.hpp"

#include <string>

namespace opentxs
{
namespace crypto
{
namespace key
{
class HD : virtual public EllipticCurve
{
public:
    OPENTXS_EXPORT static Bip32Fingerprint CalculateFingerprint(
        const api::crypto::Hash& hash,
        const Data& pubkey);

    OPENTXS_EXPORT virtual OTData Chaincode(
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual int Depth() const = 0;
    OPENTXS_EXPORT virtual Bip32Fingerprint Fingerprint(
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual std::string Xprv(
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual std::string Xpub(
        const PasswordPrompt& reason) const = 0;

    OPENTXS_EXPORT ~HD() override = default;

protected:
    HD() = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#endif
