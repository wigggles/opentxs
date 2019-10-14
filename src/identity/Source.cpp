// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/Core.hpp"
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
#include "opentxs/identity/Source.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"

#include "internal/api/Api.hpp"

#include <memory>
#include <ostream>

#include "Source.hpp"

#define OT_METHOD "opentxs::identity::Source::"

namespace opentxs
{
identity::Source* Factory::NymIDSource(
    const api::internal::Core& api,
    NymParameters& params,
    const opentxs::PasswordPrompt& reason)
{
    using ReturnType = identity::implementation::Source;

    switch (params.SourceType()) {
        case proto::SOURCETYPE_BIP47: {
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            const auto paymentCode = api.Factory().PaymentCode(
                params.Seed(),
                params.Nym(),
                PaymentCode::DefaultVersion,
                reason);

            return new ReturnType{api.Factory(), paymentCode};
#else
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": opentxs was build without bip47 support")
                .Flush();

            return nullptr;
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        }
        case proto::SOURCETYPE_PUBKEY:
            switch (params.credentialType()) {
                case proto::CREDTYPE_LEGACY: {
                    params.source_keypair_ = api.Factory().Keypair(
                        params,
                        crypto::key::Asymmetric::DefaultVersion,
                        proto::KEYROLE_SIGN,
                        reason);
                } break;
                case proto::CREDTYPE_HD:
#if OT_CRYPTO_SUPPORTED_KEY_HD
                {
                    const auto curve =
                        crypto::AsymmetricProvider::KeyTypeToCurve(
                            params.AsymmetricKeyType());

                    if (EcdsaCurve::invalid == curve) {
                        throw std::runtime_error("Invalid curve type");
                    }

                    params.source_keypair_ = api.Factory().Keypair(
                        params.Seed(),
                        params.Nym(),
                        params.Credset(),
                        params.CredIndex(),
                        curve,
                        proto::KEYROLE_SIGN,
                        reason);
                } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
                case proto::CREDTYPE_ERROR:
                default: {
                    throw std::runtime_error("Unsupported credential type");
                }
            }

            if (false == bool(params.source_keypair_.get())) {
                LogOutput("opentxs::Factory::")(__FUNCTION__)(
                    ": Failed to generate signing keypair")
                    .Flush();

                return nullptr;
            }

            return new ReturnType{api.Factory(), params, reason};
        case proto::SOURCETYPE_ERROR:
        default: {
            LogOutput("opentxs::Factory::")(__FUNCTION__)(
                ": Unsupported source type.")
                .Flush();

            return nullptr;
        }
    }
}

identity::Source* Factory::NymIDSource(
    const api::internal::Core& api,
    const proto::NymIDSource& serialized,
    const opentxs::PasswordPrompt& reason)
{
    using ReturnType = identity::implementation::Source;

    return new ReturnType{api.Factory(), serialized, reason};
}
}  // namespace opentxs

namespace opentxs::identity::implementation
{
const VersionConversionMap Source::key_to_source_version_{
    {1, 1},
    {2, 2},
};

Source::Source(
    const api::Factory& factory,
    const proto::NymIDSource& serialized,
    const PasswordPrompt& reason) noexcept
    : factory_(factory)
    , type_(serialized.type())
    , pubkey_(deserialize_pubkey(factory, type_, serialized, reason))
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(deserialize_paymentcode(factory, type_, serialized, reason))
#endif
    , version_(serialized.version())
{
}

Source::Source(
    const api::Factory& factory,
    const NymParameters& nymParameters,
    const PasswordPrompt& reason) noexcept(false)
    : factory_{factory}
    , type_(nymParameters.SourceType())
    , pubkey_(nymParameters.source_keypair_->GetPublicKey())
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(factory_.PaymentCode("", reason))
#endif
    , version_(key_to_source_version_.at(pubkey_->Version()))

{
    if (false == bool(pubkey_.get())) {
        throw std::runtime_error("Invalid pubkey");
    }
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
Source::Source(const api::Factory& factory, const PaymentCode& source) noexcept
    : factory_{factory}
    , type_(proto::SOURCETYPE_BIP47)
    , pubkey_(crypto::key::Asymmetric::Factory())
    , payment_code_{source}
    , version_(key_to_source_version_.at(payment_code_->Version()))
{
}
#endif

Source::Source(const Source& rhs, const PasswordPrompt& reason) noexcept
    : Source(rhs.factory_, *rhs.Serialize(), reason)
{
}

OTData Source::asData() const
{
    std::shared_ptr<proto::NymIDSource> serialized = Serialize();

    return factory_.Data(*serialized);
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
OTPaymentCode Source::deserialize_paymentcode(
    const api::Factory& factory,
    const proto::SourceType type,
    const proto::NymIDSource& serialized,
    const PasswordPrompt& reason)
{
    if (proto::SOURCETYPE_BIP47 == type) {

        return factory.PaymentCode(serialized.paymentcode(), reason);
    } else {

        return factory.PaymentCode("", reason);
    }
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

OTAsymmetricKey Source::deserialize_pubkey(
    const api::Factory& factory,
    const proto::SourceType type,
    const proto::NymIDSource& serialized,
    const PasswordPrompt& reason)
{
    if (proto::SOURCETYPE_PUBKEY == type) {

        return factory.AsymmetricKey(serialized.key(), reason);
    } else {

        return crypto::key::Asymmetric::Factory();
    }
}

std::unique_ptr<proto::AsymmetricKey> Source::extract_key(
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

OTNymID Source::NymID() const noexcept
{
    auto nymID = factory_.NymID();
    auto dataVersion = Data::Factory();

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY: {
            dataVersion = asData();
            nymID->CalculateDigest(dataVersion);

        } break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47: {
            nymID = payment_code_->ID();
        } break;
#endif
        default: {
        }
    }

    return nymID;
}

std::shared_ptr<proto::NymIDSource> Source::Serialize() const noexcept
{
    std::shared_ptr<proto::NymIDSource> source =
        std::make_shared<proto::NymIDSource>();
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
bool Source::Verify(
    const proto::Credential& master,
    [[maybe_unused]] const proto::Signature& sourceSignature,
    [[maybe_unused]] const PasswordPrompt& reason) const noexcept
{
    bool isSelfSigned, sameSource;
    std::unique_ptr<proto::AsymmetricKey> signingKey;
    std::shared_ptr<proto::AsymmetricKey> sourceKey;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY: {
            if (!pubkey_.get()) { return false; }

            isSelfSigned =
                (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 master.masterdata().sourceproof().type());

            if (!isSelfSigned) {
                OT_ASSERT_MSG(false, "Not yet implemented");

                return false;
            }

            signingKey = extract_key(master, proto::KEYROLE_SIGN);

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
        } break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47: {
            if (!payment_code_->Verify(master, sourceSignature, reason)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Invalid source signature.")
                    .Flush();

                return false;
            }
        } break;
#endif
        default: {
            return false;
        }
    }

    return true;
}

bool Source::Sign(
    [[maybe_unused]] const identity::credential::Primary& credential,
    [[maybe_unused]] proto::Signature& sig,
    [[maybe_unused]] const PasswordPrompt& reason) const noexcept
{
    bool goodsig = false;

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY): {
            OT_ASSERT_MSG(false, "This is not implemented yet.");

        } break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47): {
            goodsig = payment_code_->Sign(credential, sig, reason);

        } break;
#endif
        default: {
        }
    }

    return goodsig;
}

OTString Source::asString() const noexcept
{
    return OTString(factory_.Armored(asData()));
}

OTString Source::Description() const noexcept
{
    auto description = String::Factory();
    auto keyID = factory_.Identifier();

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY): {
            if (pubkey_.get()) {
                pubkey_->CalculateID(keyID);
                description = String::Factory(keyID);
            }
        } break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47): {
            description = String::Factory(payment_code_->asBase58());

        } break;
#endif
        default: {
        }
    }

    return description;
}
}  // namespace opentxs::identity::implementation
