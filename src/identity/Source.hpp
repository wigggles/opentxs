// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/PaymentCode.hpp"
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
        const proto::Signature& sourceSignature) const noexcept final;
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
    OTPaymentCode payment_code_;
    VersionNumber version_;

    static OTAsymmetricKey deserialize_pubkey(
        const api::Factory& factory,
        const proto::SourceType type,
        const proto::NymIDSource& serialized);
    static OTPaymentCode deserialize_paymentcode(
        const api::Factory& factory,
        const proto::SourceType type,
        const proto::NymIDSource& serialized);
    static std::unique_ptr<proto::AsymmetricKey> extract_key(
        const proto::Credential& credential,
        const proto::KeyRole role);

    OTData asData() const;

    Source(
        const api::Factory& factory,
        const proto::NymIDSource& serializedSource) noexcept;
    Source(
        const api::Factory& factory,
        const NymParameters& nymParameters) noexcept(false);
    Source(const api::Factory& factory, const PaymentCode& source) noexcept;
    Source(const Source& rhs) noexcept;
    Source() = delete;
    Source(Source&&) = delete;
    Source& operator=(const Source&);
    Source& operator=(Source&&);
};
}  // namespace opentxs::identity::implementation
