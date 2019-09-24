// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/NymParameters.hpp"

#include "opentxs/crypto/key/Asymmetric.hpp"

#include <cstdint>
#include <memory>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{
NymParameters::NymParameters()
    : sourceType_(proto::SOURCETYPE_PUBKEY)
    , sourceProofType_(proto::SOURCEPROOFTYPE_SELF_SIGNATURE)
    , contact_data_()
    , verification_set_()
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    , nymType_(NymParameterType::ed25519)
#elif OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    , nymType_(NymParameterType::SECP256K1)
#elif OT_CRYPTO_SUPPORTED_KEY_RSA
    , nymType_(NymParameterType::RSA)
#else
    , nymType_(NymParameterType::ERROR)
#endif
    , credentialType_()
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , entropy_()
    , seed_()
    , nym_(0)
    , credset_(0)
    , cred_index_(0)
    , default_(true)
    , use_auto_index_(false)
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    , nBits_(1024)
#endif
{
}

NymParameters::NymParameters(proto::CredentialType theCredentialtype)
    : NymParameters()
{
    setCredentialType(theCredentialtype);
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
NymParameters::NymParameters(const std::int32_t keySize)
    : NymParameters()
{
    nymType_ = NymParameterType::rsa;
    credentialType_ = proto::CREDTYPE_LEGACY;
    nBits_ = keySize;
}
#endif

NymParameters::NymParameters(const NymParameters& rhs)
    : NymParameters()
{
    sourceType_ = rhs.sourceType_;
    sourceProofType_ = rhs.sourceProofType_;
    contact_data_ = rhs.contact_data_;
    verification_set_ = rhs.verification_set_;
    nymType_ = rhs.nymType_;
    credentialType_ = rhs.credentialType_;
#if OT_CRYPTO_SUPPORTED_KEY_HD

    if (rhs.entropy_) { entropy_.reset(new OTPassword(*rhs.entropy_)); }

    seed_ = rhs.seed_;
    nym_ = rhs.nym_;
    credset_ = rhs.credset_;
    cred_index_ = rhs.cred_index_;
    default_ = rhs.default_;
    use_auto_index_ = rhs.use_auto_index_;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    nBits_ = rhs.nBits_;
#endif
}

NymParameterType NymParameters::nymParameterType() { return nymType_; }

proto::AsymmetricKeyType NymParameters::AsymmetricKeyType() const
{
    proto::AsymmetricKeyType newKeyType;

    switch (nymType_) {
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case NymParameterType::rsa:
            newKeyType = proto::AKEYTYPE_LEGACY;
            break;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case NymParameterType::secp256k1:
            newKeyType = proto::AKEYTYPE_SECP256K1;
            break;
#endif
        case NymParameterType::ed25519:
            newKeyType = proto::AKEYTYPE_ED25519;
            break;
        default:
            newKeyType = proto::AKEYTYPE_ERROR;
    }
    return newKeyType;
}

void NymParameters::setNymParameterType(NymParameterType theKeytype)
{
    nymType_ = theKeytype;
}

proto::CredentialType NymParameters::credentialType() const
{
    return credentialType_;
}

void NymParameters::setCredentialType(proto::CredentialType theCredentialtype)
{
    credentialType_ = theCredentialtype;

    switch (theCredentialtype) {
        case (proto::CREDTYPE_LEGACY):
            SetSourceType(proto::SOURCETYPE_PUBKEY);
            SetSourceProofType(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

            break;
#if OT_CRYPTO_SUPPORTED_KEY_HD
        case (proto::CREDTYPE_HD):
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            SetSourceType(proto::SOURCETYPE_BIP47);
            SetSourceProofType(proto::SOURCEPROOFTYPE_SIGNATURE);
#else
            SetSourceType(proto::SOURCETYPE_PUBKEY);
            SetSourceProofType(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);
#endif

            break;
#endif
        default:

            break;
    }
}

void NymParameters::SetContactData(const proto::ContactData& contactData)
{
    contact_data_.reset(new proto::ContactData(contactData));
}

void NymParameters::SetVerificationSet(
    const proto::VerificationSet& verificationSet)
{
    verification_set_.reset(new proto::VerificationSet(verificationSet));
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
std::int32_t NymParameters::keySize() { return nBits_; }

void NymParameters::setKeySize(std::int32_t keySize) { nBits_ = keySize; }
#endif

#if OT_CRYPTO_SUPPORTED_KEY_HD
const std::unique_ptr<OTPassword>& NymParameters::Entropy() const
{
    return entropy_;
}

void NymParameters::SetEntropy(const OTPassword& entropy)
{
    entropy_.reset(new OTPassword(entropy));
}
#endif
}  // namespace opentxs
