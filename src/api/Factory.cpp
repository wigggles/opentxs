// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Factory.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif

#include "Factory.hpp"

//#define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs
{
api::Factory* Factory::FactoryAPI(
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds
#endif
)
{
    return new api::implementation::Factory(
#if OT_CRYPTO_WITH_BIP39
        seeds
#endif
    );
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
Factory::Factory(
#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds
#endif
    )
    :
#if OT_CRYPTO_WITH_BIP39
    seeds_(seeds)
#endif
{
}

#if OT_CRYPTO_WITH_BIP39
OTPaymentCode Factory::PaymentCode(const std::string& base58) const
{
    return opentxs::PaymentCode::Factory(seeds_, base58);
}

OTPaymentCode Factory::PaymentCode(const proto::PaymentCode& serialized) const
{
    return opentxs::PaymentCode::Factory(seeds_, serialized);
}

OTPaymentCode Factory::PaymentCode(
    const std::string& seed,
    const std::uint32_t nym,
    const std::uint8_t version,
    const bool bitmessage,
    const std::uint8_t bitmessageVersion,
    const std::uint8_t bitmessageStream) const
{
    return opentxs::PaymentCode::Factory(
        seeds_,
        seed,
        nym,
        version,
        bitmessage,
        bitmessageVersion,
        bitmessageStream);
}
#endif
}  // namespace opentxs::api::implementation
