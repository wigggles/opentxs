// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "opentxs/core/crypto/NymParameters.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>

#include "2_Factory.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/protobuf/ContactData.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/VerificationSet.pb.h"
#include "util/Container.hpp"

namespace opentxs
{
const std::map<proto::AsymmetricKeyType, NymParameterType> key_to_nym_
{
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    {proto::AKEYTYPE_LEGACY, NymParameterType::rsa},
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        {proto::AKEYTYPE_SECP256K1, NymParameterType::secp256k1},
#endif
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        {proto::AKEYTYPE_ED25519, NymParameterType::ed25519},
#endif
};
const auto nym_to_key_{reverse_map(key_to_nym_)};

NymParameters::NymParameters(
    const NymParameterType type,
    const proto::CredentialType credential,
    const proto::SourceType source) noexcept
    : source_keypair_(Factory::Keypair().release())
    , nymType_(type)
    , credentialType_(
          (NymParameterType::rsa == nymType_) ? proto::CREDTYPE_LEGACY
                                              : credential)
    , sourceType_(
          (NymParameterType::rsa == nymType_) ? proto::SOURCETYPE_PUBKEY
                                              : source)
    , sourceProofType_(
          (proto::SOURCETYPE_BIP47 == sourceType_)
              ? proto::SOURCEPROOFTYPE_SIGNATURE
              : proto::SOURCEPROOFTYPE_SELF_SIGNATURE)
    , contact_data_(nullptr)
    , verification_set_(nullptr)
#if OT_CRYPTO_WITH_BIP32
    , seed_style_(crypto::SeedStyle::BIP39)
    , seed_language_(crypto::Language::en)
    , seed_strength_(crypto::SeedStrength::TwentyFour)
    , entropy_(Context().Factory().Secret(0))
    , seed_("")
    , nym_(0)
    , credset_(0)
    , cred_index_(0)
    , default_(true)
    , use_auto_index_(true)
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    , nBits_(1024)
    , params_()
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
{
}

NymParameters::NymParameters(
    proto::AsymmetricKeyType key,
    proto::CredentialType credential,
    const proto::SourceType source) noexcept
    : NymParameters(key_to_nym_.at(key), credential, source)
{
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
NymParameters::NymParameters(const std::int32_t keySize) noexcept
    : NymParameters(NymParameterType::rsa, proto::CREDTYPE_LEGACY)
{
    nBits_ = keySize;
}
#endif

NymParameters::NymParameters(
    [[maybe_unused]] const std::string& seedID,
    [[maybe_unused]] const int index) noexcept
    : NymParameters()
{
#if OT_CRYPTO_WITH_BIP32
    if (0 < seedID.size()) { SetSeed(seedID); }

    if (index >= 0) { SetNym(static_cast<Bip32Index>(index)); }
#endif  // OT_CRYPTO_WITH_BIP32
}

NymParameters::NymParameters(const NymParameters& rhs) noexcept
    : NymParameters(rhs.nymType_, rhs.credentialType_, rhs.sourceType_)
{
    contact_data_ = rhs.contact_data_;
    verification_set_ = rhs.verification_set_;
#if OT_CRYPTO_WITH_BIP32
    entropy_ = rhs.entropy_;
    seed_ = rhs.seed_;
    nym_ = rhs.nym_;
    credset_ = rhs.credset_;
    cred_index_ = rhs.cred_index_;
    default_ = rhs.default_;
    use_auto_index_ = rhs.use_auto_index_;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    nBits_ = rhs.nBits_;
    params_ = rhs.params_;
#endif
}

auto NymParameters::AsymmetricKeyType() const -> proto::AsymmetricKeyType
{
    try {
        return nym_to_key_.at(nymType_);
    } catch (...) {
        return proto::AKEYTYPE_ERROR;
    }
}

auto NymParameters::ChangeType(const NymParameterType type) const
    -> NymParameters
{
    auto output{*this};
    const_cast<NymParameterType&>(output.nymType_) = type;

    if (NymParameterType::rsa == output.nymType_) {
        const_cast<proto::CredentialType&>(output.credentialType_) =
            proto::CREDTYPE_LEGACY;
        const_cast<proto::SourceType&>(output.sourceType_) =
            proto::SOURCETYPE_PUBKEY;
        const_cast<proto::SourceProofType&>(output.sourceProofType_) =
            proto::SOURCEPROOFTYPE_SELF_SIGNATURE;
    }

    return output;
}

auto NymParameters::ContactData() const -> std::shared_ptr<proto::ContactData>
{
    return contact_data_;
}

auto NymParameters::credentialType() const -> proto::CredentialType
{
    return credentialType_;
}

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::CredIndex() const -> Bip32Index { return cred_index_; }
auto NymParameters::Credset() const -> Bip32Index { return credset_; }
auto NymParameters::Default() const -> bool { return default_; }
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto NymParameters::DHParams() const -> ReadView { return reader(params_); }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_WITH_BIP32
auto NymParameters::Entropy() const -> const Secret& { return entropy_; }
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto NymParameters::keySize() const -> std::int32_t { return nBits_; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::Nym() const -> Bip32Index { return nym_; }
#endif  // OT_CRYPTO_WITH_BIP32

auto NymParameters::nymParameterType() const -> NymParameterType
{
    return nymType_;
}
#if OT_CRYPTO_WITH_BIP32
auto NymParameters::Seed() const -> std::string { return seed_; }
auto NymParameters::SeedLanguage() const -> crypto::Language
{
    return seed_language_;
}
auto NymParameters::SeedStrength() const -> crypto::SeedStrength
{
    return seed_strength_;
}
auto NymParameters::SeedStyle() const -> crypto::SeedStyle
{
    return seed_style_;
}
#endif  // OT_CRYPTO_WITH_BIP32
auto NymParameters::SourceProofType() const -> proto::SourceProofType
{
    return sourceProofType_;
}
auto NymParameters::SourceType() const -> proto::SourceType
{
    return sourceType_;
}
#if OT_CRYPTO_WITH_BIP32
auto NymParameters::UseAutoIndex() const -> bool { return use_auto_index_; }
#endif  // OT_CRYPTO_WITH_BIP32
auto NymParameters::VerificationSet() const
    -> std::shared_ptr<proto::VerificationSet>
{
    return verification_set_;
}

auto NymParameters::SetContactData(const proto::ContactData& contactData)
    -> void
{
    contact_data_.reset(new proto::ContactData(contactData));
}

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::SetCredIndex(const Bip32Index path) -> void
{
    cred_index_ = path;
}
auto NymParameters::SetCredset(const Bip32Index path) -> void
{
    credset_ = path;
}
auto NymParameters::SetDefault(const bool in) -> void { default_ = in; }
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto NymParameters::SetDHParams(const ReadView bytes) -> void
{
    auto start = reinterpret_cast<const std::byte*>(bytes.data());
    auto end = start + bytes.size();

    params_.assign(start, end);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::SetEntropy(const Secret& entropy) -> void
{
    entropy_ = entropy;
}
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto NymParameters::setKeySize(std::int32_t keySize) -> void
{
    nBits_ = keySize;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::SetNym(const Bip32Index path) -> void
{
    nym_ = path;
    use_auto_index_ = false;
}
#endif

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::SetSeed(const std::string& seed) -> void { seed_ = seed; }
auto NymParameters::SetSeedLanguage(const crypto::Language lang) -> void
{
    seed_language_ = lang;
}
auto NymParameters::SetSeedStrength(const crypto::SeedStrength type) -> void
{
    seed_strength_ = type;
}
auto NymParameters::SetSeedStyle(const crypto::SeedStyle type) -> void
{
    seed_style_ = type;
}
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_WITH_BIP32
auto NymParameters::SetUseAutoIndex(const bool use) -> void
{
    use_auto_index_ = use;
}
#endif

auto NymParameters::SetVerificationSet(
    const proto::VerificationSet& verificationSet) -> void
{
    verification_set_.reset(new proto::VerificationSet(verificationSet));
}
}  // namespace opentxs
