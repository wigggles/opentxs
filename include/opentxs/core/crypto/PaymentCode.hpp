// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP
#define OPENTXS_CORE_CRYPTO_PAYMENTCODE_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs
{
using OTPaymentCode = Pimpl<PaymentCode>;

class PaymentCode
{
public:
    using Serialized = proto::PaymentCode;

    OPENTXS_EXPORT static const VersionNumber DefaultVersion;

    OPENTXS_EXPORT virtual operator const crypto::key::Asymmetric&() const
        noexcept = 0;

    OPENTXS_EXPORT virtual bool operator==(const proto::PaymentCode& rhs) const
        noexcept = 0;

    OPENTXS_EXPORT virtual const identifier::Nym& ID() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string asBase58() const noexcept = 0;
    OPENTXS_EXPORT virtual Serialized Serialize() const noexcept = 0;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual bool Sign(
        const identity::credential::Base& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Sign(
        const Data& data,
        Data& output,
        const PasswordPrompt& reason) const noexcept = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual bool Valid() const noexcept = 0;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature) const noexcept = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual VersionNumber Version() const noexcept = 0;

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual bool AddPrivateKeys(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) noexcept = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    OPENTXS_EXPORT virtual ~PaymentCode() = default;

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
#endif
