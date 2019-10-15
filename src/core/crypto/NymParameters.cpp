// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/crypto/NymParameters.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
NymParameters::NymParameters()
    : source_keypair_(Factory::Keypair())
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , sourceType_(proto::SOURCETYPE_BIP47)
    , sourceProofType_(proto::SOURCEPROOFTYPE_SIGNATURE)
#else
    , sourceType_(proto::SOURCETYPE_PUBKEY)
    , sourceProofType_(proto::SOURCEPROOFTYPE_SELF_SIGNATURE)
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , contact_data_(nullptr)
    , verification_set_(nullptr)
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    , nymType_(NymParameterType::ed25519)
#elif OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    , nymType_(NymParameterType::SECP256K1)
#elif OT_CRYPTO_SUPPORTED_KEY_RSA
    , nymType_(NymParameterType::RSA)
#else
    , nymType_(NymParameterType::ERROR)
#endif
#if OT_CRYPTO_SUPPORTED_KEY_HD
    , credentialType_(proto::CREDTYPE_HD)
    , entropy_(nullptr)
    , seed_("")
    , nym_(0)
    , credset_(0)
    , cred_index_(0)
    , default_(true)
    , use_auto_index_(true)
#else
    , credentialType_(proto::CREDTYPE_LEGACY)
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

NymParameters::NymParameters(
    [[maybe_unused]] const std::string& seedID,
    [[maybe_unused]] const int index)
    : NymParameters()
{
#if OT_CRYPTO_SUPPORTED_KEY_HD
    if (0 < seedID.size()) { SetSeed(seedID); }

    if (index >= 0) { SetNym(static_cast<Bip32Index>(index)); }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
}

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

proto::AsymmetricKeyType NymParameters::AsymmetricKeyType() const
{
    proto::AsymmetricKeyType newKeyType;

    switch (nymType_) {
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case NymParameterType::rsa: {
            newKeyType = proto::AKEYTYPE_LEGACY;
        } break;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case NymParameterType::secp256k1: {
            newKeyType = proto::AKEYTYPE_SECP256K1;
        } break;
#endif
        case NymParameterType::ed25519: {
            newKeyType = proto::AKEYTYPE_ED25519;
        } break;
        default: {
            newKeyType = proto::AKEYTYPE_ERROR;
        }
    }

    return newKeyType;
}

void NymParameters::SetContactData(const proto::ContactData& contactData)
{
    contact_data_.reset(new proto::ContactData(contactData));
}

void NymParameters::setCredentialType(proto::CredentialType theCredentialtype)
{
    credentialType_ = theCredentialtype;

    switch (theCredentialtype) {
        case (proto::CREDTYPE_LEGACY): {
            SetSourceType(proto::SOURCETYPE_PUBKEY);
        } break;
#if OT_CRYPTO_SUPPORTED_KEY_HD
        case (proto::CREDTYPE_HD): {
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            SetSourceType(proto::SOURCETYPE_BIP47);
#else
            SetSourceType(proto::SOURCETYPE_PUBKEY);
#endif

        } break;
#endif
        default: {
        }
    }
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
void NymParameters::SetEntropy(const OTPassword& entropy)
{
    entropy_.reset(new OTPassword(entropy));
}

void NymParameters::SetNym(const Bip32Index path)
{
    nym_ = path;
    use_auto_index_ = false;
}
#endif

void NymParameters::setNymParameterType(NymParameterType theKeytype)
{
    nymType_ = theKeytype;

    if (theKeytype == NymParameterType::rsa) {
        SetSourceType(proto::SOURCETYPE_PUBKEY);
    }
}

void NymParameters::SetSourceType(proto::SourceType type)
{
    sourceType_ = type;

    switch (type) {
        case proto::SOURCETYPE_PUBKEY: {
            sourceProofType_ = proto::SOURCEPROOFTYPE_SELF_SIGNATURE;
        } break;
        case proto::SOURCETYPE_BIP47: {
            sourceProofType_ = proto::SOURCEPROOFTYPE_SIGNATURE;
        } break;
        case proto::SOURCETYPE_ERROR:
        default: {
            sourceProofType_ = proto::SOURCEPROOFTYPE_ERROR;
        }
    }
}

void NymParameters::SetVerificationSet(
    const proto::VerificationSet& verificationSet)
{
    verification_set_.reset(new proto::VerificationSet(verificationSet));
}
}  // namespace opentxs
