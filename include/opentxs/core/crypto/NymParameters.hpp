// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
#define OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"
// IWYU pragma: no_include "opentxs/crypto/Language.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStrength.hpp"
// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/core/Secret.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
namespace proto
{
class ContactData;
class VerificationSet;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
class NymParameters
{
public:
    OTKeypair source_keypair_;

    OPENTXS_EXPORT proto::AsymmetricKeyType AsymmetricKeyType() const;
    OPENTXS_EXPORT NymParameters ChangeType(const NymParameterType type) const;
    OPENTXS_EXPORT std::shared_ptr<proto::ContactData> ContactData() const;
    OPENTXS_EXPORT proto::CredentialType credentialType() const;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT Bip32Index CredIndex() const;
    OPENTXS_EXPORT Bip32Index Credset() const;
    OPENTXS_EXPORT bool Default() const;
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT ReadView DHParams() const;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT const Secret& Entropy() const;
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT std::int32_t keySize() const;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT Bip32Index Nym() const;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT NymParameterType nymParameterType() const;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT std::string Seed() const;
    OPENTXS_EXPORT crypto::Language SeedLanguage() const;
    OPENTXS_EXPORT crypto::SeedStrength SeedStrength() const;
    OPENTXS_EXPORT crypto::SeedStyle SeedStyle() const;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT proto::SourceProofType SourceProofType() const;
    OPENTXS_EXPORT proto::SourceType SourceType() const;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT bool UseAutoIndex() const;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT std::shared_ptr<proto::VerificationSet> VerificationSet()
        const;

    OPENTXS_EXPORT void SetContactData(const proto::ContactData& contactData);
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT void SetCredIndex(const Bip32Index path);
    OPENTXS_EXPORT void SetCredset(const Bip32Index path);
    OPENTXS_EXPORT void SetDefault(const bool in);
    OPENTXS_EXPORT void SetEntropy(const Secret& entropy);
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT void setKeySize(std::int32_t keySize);
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT void SetNym(const Bip32Index path);
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT void SetDHParams(const ReadView bytes);
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT void SetSeed(const std::string& seed);
    OPENTXS_EXPORT void SetSeedLanguage(const crypto::Language lang);
    OPENTXS_EXPORT void SetSeedStrength(const crypto::SeedStrength value);
    OPENTXS_EXPORT void SetSeedStyle(const crypto::SeedStyle type);
#endif  // OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT void SetUseAutoIndex(const bool use);
#endif
    OPENTXS_EXPORT void SetVerificationSet(
        const proto::VerificationSet& verificationSet);

    OPENTXS_EXPORT NymParameters(
        const NymParameterType type =
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            NymParameterType::secp256k1
#elif OT_CRYPTO_SUPPORTED_KEY_ED25519
            NymParameterType::ed25519
#elif OT_CRYPTO_SUPPORTED_KEY_RSA
            NymParameterType::rsa
#else
            NymParameterType::error
#endif
        ,
        const proto::CredentialType credential =
#if OT_CRYPTO_WITH_BIP32
            proto::CREDTYPE_HD
#else
            proto::CREDTYPE_LEGACY
#endif
        ,
        const proto::SourceType source =
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
            proto::SOURCETYPE_BIP47
#else
            proto::SOURCETYPE_PUBKEY
#endif
        ) noexcept;
    OPENTXS_EXPORT NymParameters(
        proto::AsymmetricKeyType key,
        proto::CredentialType credential =
#if OT_CRYPTO_WITH_BIP32
            proto::CREDTYPE_HD
#else
            proto::CREDTYPE_LEGACY
#endif
        ,
        const proto::SourceType source =
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
            proto::SOURCETYPE_BIP47
#else
            proto::SOURCETYPE_PUBKEY
#endif
        ) noexcept;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT explicit NymParameters(const std::int32_t keySize) noexcept;
#endif
    OPENTXS_EXPORT NymParameters(
        const std::string& seedID,
        const int index) noexcept;
    OPENTXS_EXPORT NymParameters(const NymParameters&) noexcept;

    OPENTXS_EXPORT ~NymParameters() = default;

private:
    const NymParameterType nymType_;
    const proto::CredentialType credentialType_;
    const proto::SourceType sourceType_;
    const proto::SourceProofType sourceProofType_;
    std::shared_ptr<proto::ContactData> contact_data_;
    std::shared_ptr<proto::VerificationSet> verification_set_;
#if OT_CRYPTO_WITH_BIP32
    crypto::SeedStyle seed_style_;
    crypto::Language seed_language_;
    crypto::SeedStrength seed_strength_;
    OTSecret entropy_;
    std::string seed_;
    Bip32Index nym_;
    Bip32Index credset_;
    Bip32Index cred_index_;
    bool default_;
    bool use_auto_index_;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    std::int32_t nBits_;
    Space params_;
#endif
};
}  // namespace opentxs
#endif
