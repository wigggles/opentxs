// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP
#define OPENTXS_CORE_CRYPTO_NYMPARAMETERS_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/core/crypto/OTPassword.hpp"
#endif
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <string>
#include <memory>

namespace opentxs
{
class NymParameters
{
public:
    OTKeypair source_keypair_;

    OPENTXS_EXPORT proto::AsymmetricKeyType AsymmetricKeyType() const;
    OPENTXS_EXPORT std::shared_ptr<proto::ContactData> ContactData() const
    {
        return contact_data_;
    }
    OPENTXS_EXPORT proto::CredentialType credentialType() const
    {
        return credentialType_;
    }
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT Bip32Index CredIndex() const { return cred_index_; }
    OPENTXS_EXPORT Bip32Index Credset() const { return credset_; }
    OPENTXS_EXPORT bool Default() const { return default_; }
    OPENTXS_EXPORT const std::unique_ptr<OTPassword>& Entropy() const
    {
        return entropy_;
    }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT std::int32_t keySize() { return nBits_; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT Bip32Index Nym() const { return nym_; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT NymParameterType nymParameterType() const
    {
        return nymType_;
    }
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT std::string Seed() const { return seed_; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT proto::SourceProofType SourceProofType() const
    {
        return sourceProofType_;
    }
    OPENTXS_EXPORT proto::SourceType SourceType() const { return sourceType_; }
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT bool UseAutoIndex() const { return use_auto_index_; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT std::shared_ptr<proto::VerificationSet> VerificationSet()
        const
    {
        return verification_set_;
    }

    OPENTXS_EXPORT void SetContactData(const proto::ContactData& contactData);
    OPENTXS_EXPORT void setCredentialType(
        proto::CredentialType theCredentialtype);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void SetCredIndex(const Bip32Index path)
    {
        cred_index_ = path;
    }
    OPENTXS_EXPORT void SetCredset(const Bip32Index path) { credset_ = path; }
    OPENTXS_EXPORT void SetDefault(const bool in) { default_ = in; }
    OPENTXS_EXPORT void SetEntropy(const OTPassword& entropy);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT void setKeySize(std::int32_t keySize) { nBits_ = keySize; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void SetNym(const Bip32Index path);
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void setNymParameterType(NymParameterType theKeytype);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void SetSeed(const std::string& seed) { seed_ = seed; }
#endif  // OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void SetSourceType(proto::SourceType sType);
#if OT_CRYPTO_SUPPORTED_KEY_HD
    OPENTXS_EXPORT void SetUseAutoIndex(const bool use)
    {
        use_auto_index_ = use;
    }
#endif
    OPENTXS_EXPORT void SetVerificationSet(
        const proto::VerificationSet& verificationSet);

    OPENTXS_EXPORT NymParameters();
    OPENTXS_EXPORT explicit NymParameters(
        proto::CredentialType theCredentialtype);
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT explicit NymParameters(const std::int32_t keySize);
#endif
    OPENTXS_EXPORT NymParameters(const std::string& seedID, const int index);
    OPENTXS_EXPORT NymParameters(const NymParameters& rhs);

    OPENTXS_EXPORT ~NymParameters() = default;

private:
    proto::SourceType sourceType_;
    proto::SourceProofType sourceProofType_;
    std::shared_ptr<proto::ContactData> contact_data_;
    std::shared_ptr<proto::VerificationSet> verification_set_;
    NymParameterType nymType_;
    proto::CredentialType credentialType_;
#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::unique_ptr<OTPassword> entropy_;
    std::string seed_;
    Bip32Index nym_;
    Bip32Index credset_;
    Bip32Index cred_index_;
    bool default_;
    bool use_auto_index_;
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    std::int32_t nBits_;
#endif
};
}  // namespace opentxs
#endif
