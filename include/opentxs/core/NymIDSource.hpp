// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NYMIDSOURCE_HPP
#define OPENTXS_CORE_NYMIDSOURCE_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
typedef std::shared_ptr<proto::NymIDSource> serializedNymIDSource;

class NymIDSource
{
public:
    static serializedNymIDSource ExtractArmoredSource(
        const Armored& armoredSource);

    OTString asString() const;
    OTString Description() const;
    proto::SourceType Type() const;
    OTNymID NymID() const;
    serializedNymIDSource Serialize() const;
    bool Verify(
        const proto::Credential& master,
        const proto::Signature& sourceSignature,
        const PasswordPrompt& reason) const;
    bool Sign(
        const identity::credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const;

    NymIDSource(
        const api::Factory& factory,
        const proto::NymIDSource& serializedSource,
        const PasswordPrompt& reason) noexcept;
    NymIDSource(
        const api::Factory& factory,
        const String& stringSource,
        const PasswordPrompt& reason) noexcept;
    NymIDSource(
        const api::Factory& factory,
        const NymParameters& nymParameters,
        proto::AsymmetricKey& pubkey,
        const PasswordPrompt& reason) noexcept;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    NymIDSource(
        const api::Factory& factory,
        const PaymentCode& source) noexcept;
#endif
    NymIDSource(const NymIDSource& rhs, const PasswordPrompt& reason) noexcept;

private:
    static const VersionConversionMap key_to_source_version_;

    const api::Factory& factory_;

    VersionNumber version_ = 0;
    proto::SourceType type_ = proto::SOURCETYPE_ERROR;
    OTAsymmetricKey pubkey_;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    OTPaymentCode payment_code_;
#endif

    OTData asData() const;

    static std::unique_ptr<proto::AsymmetricKey> ExtractKey(
        const proto::Credential& credential,
        const proto::KeyRole role);

    NymIDSource() = delete;
    NymIDSource(const NymIDSource&) = delete;
    NymIDSource(NymIDSource&&) = delete;
    NymIDSource& operator=(const NymIDSource&);
    NymIDSource& operator=(NymIDSource&&);
};
}  // namespace opentxs

#endif
