// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/PaymentCode.hpp"

#include "Deterministic.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class PaymentCode : virtual public Deterministic
{
public:
    OPENTXS_EXPORT virtual bool IsNotified() const noexcept = 0;
    OPENTXS_EXPORT virtual OTPaymentCode LocalPaymentCode() const noexcept = 0;
    OPENTXS_EXPORT virtual OTPaymentCode RemotePaymentCode() const noexcept = 0;

    OPENTXS_EXPORT ~PaymentCode() override = default;

protected:
    PaymentCode() noexcept = default;

private:
    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    PaymentCode& operator=(const PaymentCode&) = delete;
    PaymentCode& operator=(PaymentCode&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODECHAIN_HPP
