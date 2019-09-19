// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/NymIDSource.hpp"

#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <ostream>

#define OT_METHOD "opentxs::NymIDSource::"

namespace opentxs
{
const VersionConversionMap NymIDSource::key_to_source_version_{
    {1, 1},
    {2, 2},
};

NymIDSource::NymIDSource(
    const api::Factory& factory,
    const proto::NymIDSource& serializedSource,
    const PasswordPrompt& reason) noexcept
    : factory_(factory)
    , version_(serializedSource.version())
    , type_(serializedSource.type())
    , pubkey_(crypto::key::Asymmetric::Factory())
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(factory_.PaymentCode("", reason))
#endif
{
    switch (type_) {
        case proto::SOURCETYPE_PUBKEY: {
            pubkey_ = factory_.AsymmetricKey(serializedSource.key(), reason);

            break;
        }
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47: {
            payment_code_ =
                factory_.PaymentCode(serializedSource.paymentcode(), reason);

            break;
        }
#endif
        default: {
        }
    }
}

NymIDSource::NymIDSource(
    const api::Factory& factory,
    const String& source,
    const PasswordPrompt& reason) noexcept
    : NymIDSource(
          factory,
          *ExtractArmoredSource(Armored::Factory(source)),
          reason)
{
}

NymIDSource::NymIDSource(
    const api::Factory& factory,
    const NymParameters& nymParameters,
    proto::AsymmetricKey& pubkey,
    const PasswordPrompt& reason) noexcept
    : factory_{factory}
    , version_(key_to_source_version_.at(pubkey.version()))
    , type_(nymParameters.SourceType())
    , pubkey_(factory_.AsymmetricKey(pubkey, reason))
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(factory_.PaymentCode("", reason))
#endif

{
    OT_ASSERT(pubkey_.get());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
NymIDSource::NymIDSource(
    const api::Factory& factory,
    const PaymentCode& source) noexcept
    : factory_{factory}
    , version_(1)
    , type_(proto::SOURCETYPE_BIP47)
    , pubkey_(crypto::key::Asymmetric::Factory())
    , payment_code_{source}
{
}
#endif

NymIDSource::NymIDSource(
    const NymIDSource& rhs,
    const PasswordPrompt& reason) noexcept
    : NymIDSource(rhs.factory_, *rhs.Serialize(), reason)
{
}

OTData NymIDSource::asData() const
{
    serializedNymIDSource serializedSource = Serialize();

    return factory_.Data(*serializedSource);
}

std::unique_ptr<proto::AsymmetricKey> NymIDSource::ExtractKey(
    const proto::Credential& credential,
    const proto::KeyRole role)
{
    std::unique_ptr<proto::AsymmetricKey> output;

    const bool master = (proto::CREDROLE_MASTERKEY == credential.role());
    const bool child = (proto::CREDROLE_CHILDKEY == credential.role());
    const bool keyCredential = master || child;

    if (!keyCredential) { return output; }

    const auto& publicCred = credential.publiccredential();

    for (auto& key : publicCred.key()) {
        if (role == key.role()) {
            output.reset(new proto::AsymmetricKey(key));

            break;
        }
    }

    return output;
}

OTNymID NymIDSource::NymID() const
{
    auto nymID = factory_.NymID();
    auto dataVersion = Data::Factory();

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            dataVersion = asData();
            nymID->CalculateDigest(dataVersion);

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            nymID = payment_code_->ID();

            break;
#endif
        default:
            break;
    }

    return nymID;
}

serializedNymIDSource NymIDSource::Serialize() const
{
    serializedNymIDSource source = std::make_shared<proto::NymIDSource>();
    source->set_version(version_);
    source->set_type(type_);

    std::shared_ptr<proto::AsymmetricKey> key;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    SerializedPaymentCode paycode;
#endif

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            OT_ASSERT(pubkey_.get())

            key = pubkey_->Serialize();
            key->set_role(proto::KEYROLE_SIGN);
            *(source->mutable_key()) = *key;

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            paycode = payment_code_->Serialize();
            *(source->mutable_paymentcode()) = *paycode;

            break;
#endif
        default:
            break;
    }

    return source;
}

// This function assumes that all internal verification checks are complete
// except for the source proof
bool NymIDSource::Verify(
    const proto::Credential& master,
    [[maybe_unused]] const proto::Signature& sourceSignature,
    [[maybe_unused]] const PasswordPrompt& reason) const
{
    bool isSelfSigned, sameSource;
    std::unique_ptr<proto::AsymmetricKey> signingKey;
    std::shared_ptr<proto::AsymmetricKey> sourceKey;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            if (!pubkey_.get()) { return false; }

            isSelfSigned =
                (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 master.masterdata().sourceproof().type());

            if (!isSelfSigned) {
                OT_ASSERT_MSG(false, "Not yet implemented");

                return false;
            }

            signingKey = ExtractKey(master, proto::KEYROLE_SIGN);

            if (!signingKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed to extract signing key.")
                    .Flush();

                return false;
            }

            sourceKey = pubkey_->Serialize();
            sameSource = (sourceKey->key() == signingKey->key());

            if (!sameSource) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Master credential was not"
                                                   " derived from this source.")
                    .Flush();

                return false;
            }

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            if (!payment_code_->Verify(master, sourceSignature, reason)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Invalid source signature.")
                    .Flush();

                return false;
            }

            break;
#endif
        default:
            break;
    }

    return true;
}

bool NymIDSource::Sign(
    [[maybe_unused]] const identity::credential::Primary& credential,
    [[maybe_unused]] proto::Signature& sig,
    [[maybe_unused]] const PasswordPrompt& reason) const
{
    bool goodsig = false;

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            OT_ASSERT_MSG(false, "This is not implemented yet.");

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47):
            goodsig = payment_code_->Sign(credential, sig, reason);

            break;
#endif
        default:
            break;
    }

    return goodsig;
}

OTString NymIDSource::asString() const
{
    return OTString(factory_.Armored(asData()));
}

// static
serializedNymIDSource NymIDSource::ExtractArmoredSource(
    const Armored& armoredSource)
{
    auto dataSource = Data::Factory(armoredSource);

    OT_ASSERT(dataSource->size() > 0);

    auto protoSource = std::make_shared<proto::NymIDSource>();
    protoSource->ParseFromArray(dataSource->data(), dataSource->size());

    return protoSource;
}

OTString NymIDSource::Description() const
{
    auto description = String::Factory();
    auto keyID = factory_.Identifier();

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            if (pubkey_.get()) {
                pubkey_->CalculateID(keyID);
                description = String::Factory(keyID);
            }

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47):
            description = String::Factory(payment_code_->asBase58());

            break;
#endif
        default:
            break;
    }

    return description;
}

proto::SourceType NymIDSource::Type() const { return type_; }
}  // namespace opentxs
