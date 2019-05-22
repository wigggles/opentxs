// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
using SerializedPaymentCode = std::shared_ptr<proto::PaymentCode>;

class PaymentCode
{
public:
    EXPORT static OTPaymentCode Factory(const PaymentCode& rhs);
    EXPORT static OTPaymentCode Factory(
        const api::Core& api,
        const std::string& base58);
    EXPORT static OTPaymentCode Factory(
        const api::Core& api,
        const proto::PaymentCode& serialized);
    EXPORT static OTPaymentCode Factory(
        const api::Core& api,
        const std::string& seed,
        const Bip32Index nym,
        const std::uint8_t version,
        const bool bitmessage = false,
        const std::uint8_t bitmessageVersion = 0,
        const std::uint8_t bitmessageStream = 0);

    EXPORT virtual bool operator==(const proto::PaymentCode& rhs) const = 0;
    EXPORT virtual operator const crypto::key::Asymmetric&() const = 0;

    EXPORT virtual const OTNymID ID() const = 0;
    EXPORT virtual const std::string asBase58() const = 0;
    EXPORT virtual SerializedPaymentCode Serialize() const = 0;
    EXPORT virtual bool VerifyInternally() const = 0;
    EXPORT virtual bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const = 0;
    EXPORT virtual bool Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const = 0;
    EXPORT virtual bool Sign(
        const Data& data,
        Data& output,
        const OTPasswordData* pPWData = nullptr) const = 0;

    EXPORT virtual bool AddPrivateKeys(
        const std::string& seed,
        const Bip32Index index) = 0;

    EXPORT virtual ~PaymentCode() = default;

protected:
    PaymentCode() = default;

private:
    friend OTPaymentCode;

    virtual PaymentCode* clone() const = 0;

    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    PaymentCode& operator=(const PaymentCode&);
    PaymentCode& operator=(PaymentCode&&);
};
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#endif
