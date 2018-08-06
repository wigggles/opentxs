// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::api::implementation
{
class Factory final : public opentxs::api::Factory
{
public:
#if OT_CRYPTO_WITH_BIP39
    OTPaymentCode PaymentCode(const std::string& base58) const override;
    OTPaymentCode PaymentCode(
        const proto::PaymentCode& serialized) const override;
    OTPaymentCode PaymentCode(
        const std::string& seed,
        const std::uint32_t nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0) const override;
#endif

    ~Factory() = default;

private:
    friend opentxs::Factory;

#if OT_CRYPTO_WITH_BIP39
    const api::HDSeed& seeds_;
#endif

    Factory(
#if OT_CRYPTO_WITH_BIP39
        const api::HDSeed& seeds
#endif
    );
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&) = delete;
};
}  // namespace opentxs::api::implementation
