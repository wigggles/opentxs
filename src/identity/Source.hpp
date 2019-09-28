// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>

namespace opentxs::identity::implementation
{
class Source final : virtual public identity::Source
{
public:
    OTString asString() const noexcept final;
    OTString Description() const noexcept final;
    proto::SourceType Type() const noexcept final { return type_; }
    OTNymID NymID() const noexcept final;
    std::shared_ptr<proto::NymIDSource> Serialize() const noexcept final;
    bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature,
        const PasswordPrompt& reason) const noexcept final;
    bool Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const noexcept final;

private:
    friend opentxs::Factory;

    static const VersionConversionMap key_to_source_version_;

    const api::Factory& factory_;

    proto::SourceType type_;
    OTAsymmetricKey pubkey_;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    OTPaymentCode payment_code_;
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    VersionNumber version_;

    static OTAsymmetricKey deserialize_pubkey(
        const api::Factory& factory,
        const proto::SourceType type,
        const proto::NymIDSource& serialized,
        const PasswordPrompt& reason);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    static OTPaymentCode deserialize_paymentcode(
        const api::Factory& factory,
        const proto::SourceType type,
        const proto::NymIDSource& serialized,
        const PasswordPrompt& reason);
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    static std::unique_ptr<proto::AsymmetricKey> extract_key(
        const proto::Credential& credential,
        const proto::KeyRole role);

    OTData asData() const;

    Source(
        const api::Factory& factory,
        const proto::NymIDSource& serializedSource,
        const PasswordPrompt& reason) noexcept;
    Source(
        const api::Factory& factory,
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) noexcept(false);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    Source(const api::Factory& factory, const PaymentCode& source) noexcept;
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    Source(const Source& rhs, const PasswordPrompt& reason) noexcept;
    Source() = delete;
    Source(const Source&) = delete;
    Source(Source&&) = delete;
    Source& operator=(const Source&);
    Source& operator=(Source&&);
};
}  // namespace opentxs::identity::implementation
