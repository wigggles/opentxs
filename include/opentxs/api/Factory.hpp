// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_FACTORY_HPP
#define OPENTXS_API_FACTORY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <string>

namespace opentxs
{
namespace api
{
class Factory
{
public:
#if OT_CRYPTO_WITH_BIP39
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& base58) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const = 0;
    EXPORT virtual OTPaymentCode PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const = 0;
#endif

    EXPORT virtual ~Factory() = default;

protected:
    Factory() = default;

private:
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
