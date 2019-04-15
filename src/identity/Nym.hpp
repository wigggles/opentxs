// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs
{
namespace identity::implementation
{
class Nym final : virtual public identity::internal::Nym, Lockable
{
public:
    std::string Alias() const override;
    const Serialized asPublicNym() const override;
    std::string BestEmail() const override;
    std::string BestPhoneNumber() const override;
    std::string BestSocialMediaProfile(
        const proto::ContactItemType type) const override;
    const class ContactData& Claims() const override;
    bool CompareID(const identity::Nym& RHS) const override;
    bool CompareID(const identifier::Nym& rhs) const override;
    std::set<OTIdentifier> Contracts(
        const proto::ContactItemType currency,
        const bool onlyActive) const override;
    std::string EmailAddresses(bool active) const override;
    const String& GetDescription() const override { return m_strDescription; }
    void GetIdentifier(identifier::Nym& theIdentifier) const override;
    void GetIdentifier(String& theIdentifier) const override;
    const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const override;
    const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const override;
    const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const override;
    const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const override;
    const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const override;
    std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType) const override;
    const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype) const override;
    bool HasCapability(const NymCapability& capability) const override;
    const identifier::Nym& ID() const override { return m_nymID; }
    bool Lock(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::Ciphertext& output) const override;
    std::string Name() const override;
    bool Open(
        const proto::SessionKey& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const override;
    bool Path(proto::HDPath& output) const override;
    std::string PaymentCode() const override;
    std::string PhoneNumbers(bool active) const override;
    bool ReEncryptPrivateCredentials(
        bool bImporting,
        const OTPasswordData* pPWData,
        const OTPassword* pImportPassword) const override;
    std::uint64_t Revision() const override;
    bool Seal(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::SessionKey& output) const override;
    Serialized SerializeCredentialIndex(const Mode mode) const override;
    void SerializeNymIDSource(Tag& parent) const override;
    std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active) const override;
    const std::set<proto::ContactItemType> SocialMediaProfileTypes()
        const override;
    const NymIDSource& Source() const override { return *source_; }
    std::unique_ptr<OTPassword> TransportKey(Data& pubkey) const override;
    bool Unlock(
        const proto::Ciphertext& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const;
    std::unique_ptr<proto::VerificationSet> VerificationSet() const override;
    bool VerifyPseudonym() const override;
    bool WriteCredentials() const override;

    std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters);
    bool AddClaim(const Claim& claim) override;
    bool AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active) override;
    bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active) override;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    bool AddPaymentCode(
        const class PaymentCode& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active) override;
#endif
    bool AddPreferredOTServer(const Identifier& id, const bool primary)
        override;
    bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) override;
    bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active) override;
    bool DeleteClaim(const Identifier& id) override;
    bool LoadCredentialIndex(const Serialized& index) override;
    void SetAlias(const std::string& alias) override;
    void SetAliasStartup(const std::string& alias) override { alias_ = alias; }
    bool SetCommonName(const std::string& name) override;
    bool SetContactData(const proto::ContactData& data) override;
    void SetDescription(const String& strLocation) override
    {
        eLock lock(shared_lock_);

        m_strDescription = strLocation;
    }
    bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const bool primary) override;
    bool SetVerificationSet(const proto::VerificationSet& data) override;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        const proto::HashType hash,
        proto::Signature& signature,
        const OTPasswordData* pPWData) const override;
    bool Verify(const Data& plaintext, const proto::Signature& sig)
        const override;

    ~Nym() override;

private:
    using mapOfCredentialSets =
        std::map<std::string, identity::internal::Authority*>;

    friend opentxs::Factory;

    const api::Core& api_;
    std::int32_t version_{0};
    std::uint32_t index_{0};
    std::string alias_;
    std::atomic<std::uint64_t> revision_{0};
    proto::CredentialIndexMode mode_{proto::CREDINDEX_ERROR};
    OTString m_strVersion;
    OTString m_strDescription;
    const OTNymID m_nymID;
    std::shared_ptr<NymIDSource> source_{nullptr};
    mutable std::unique_ptr<class ContactData> contact_data_;
    // The credentials for this Nym. (Each with a master key credential and
    // various child credentials.)
    mapOfCredentialSets m_mapCredentialSets;
    mapOfCredentialSets m_mapRevokedSets;
    // Revoked child credential IDs
    String::List m_listRevokedIDs;

    template <typename T>
    const crypto::key::Asymmetric& get_private_auth_key(
        const T& lock,
        proto::AsymmetricKeyType keytype) const;
    template <typename T>
    const crypto::key::Asymmetric& get_private_sign_key(
        const T& lock,
        proto::AsymmetricKeyType keytype) const;
    template <typename T>
    const crypto::key::Asymmetric& get_public_sign_key(
        const T& lock,
        proto::AsymmetricKeyType keytype) const;
    bool has_capability(const eLock& lock, const NymCapability& capability)
        const;
    void init_claims(const eLock& lock) const;
    bool set_contact_data(const eLock& lock, const proto::ContactData& data);
    bool verify_pseudonym(const eLock& lock) const;

    bool add_contact_credential(
        const eLock& lock,
        const proto::ContactData& data);
    bool add_verification_credential(
        const eLock& lock,
        const proto::VerificationSet& data);
    void clear_credentials(const eLock& lock);
    void ClearCredentials();
    bool load_credential_index(const eLock& lock, const Serialized& index);
    void revoke_contact_credentials(const eLock& lock);
    void revoke_verification_credentials(const eLock& lock);
    bool update_nym(const eLock& lock, const std::int32_t version);

    Nym(const api::Core& api, const NymParameters& nymParameters);
    Nym(const api::Core& api,
        const identifier::Nym& nymID,
        const proto::CredentialIndexMode mode);
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace identity::implementation
}  // namespace opentxs
