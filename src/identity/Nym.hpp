// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::implementation
{
class Nym final : virtual public identity::internal::Nym, Lockable
{
public:
    std::string Alias() const final;
    const Serialized asPublicNym() const final;
    std::string BestEmail() const final;
    std::string BestPhoneNumber() const final;
    std::string BestSocialMediaProfile(
        const proto::ContactItemType type) const final;
    const opentxs::ContactData& Claims() const final;
    bool CompareID(const identity::Nym& RHS) const final;
    bool CompareID(const identifier::Nym& rhs) const final;
    VersionNumber ContactCredentialVersion() const final;
    VersionNumber ContactDataVersion() const final
    {
        return contact_credential_to_contact_data_version_.at(
            ContactCredentialVersion());
    }
    std::set<OTIdentifier> Contracts(
        const proto::ContactItemType currency,
        const bool onlyActive) const final;
    std::string EmailAddresses(bool active) const final;
    const String& GetDescription() const final { return m_strDescription; }
    void GetIdentifier(identifier::Nym& theIdentifier) const final;
    void GetIdentifier(String& theIdentifier) const final;
    const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const final;
    const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const final;
    const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const final;
    const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const final;
    const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const final;
    std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType) const final;
    const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype) const final;
    bool HasCapability(const NymCapability& capability) const final;
    const identifier::Nym& ID() const final { return m_nymID; }
    bool Lock(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::Ciphertext& output) const final;
    std::string Name() const final;
    bool Open(
        const proto::SessionKey& input,
        crypto::key::Symmetric& key,
        OTPassword& password,
        const PasswordPrompt& reason) const final;
    bool Path(proto::HDPath& output) const final;
    std::string PaymentCode(const PasswordPrompt& reason) const final;
    std::string PhoneNumbers(bool active) const final;
    std::uint64_t Revision() const final;
    bool Seal(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::SessionKey& output,
        const PasswordPrompt& reason) const final;
    Serialized SerializeCredentialIndex(const Mode mode) const final;
    void SerializeNymIDSource(Tag& parent) const final;
    std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active) const final;
    const std::set<proto::ContactItemType> SocialMediaProfileTypes()
        const final;
    const identity::Source& Source() const final { return *source_; }
    std::unique_ptr<OTPassword> TransportKey(
        Data& pubkey,
        const PasswordPrompt& reason) const final;
    bool Unlock(
        const proto::Ciphertext& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const final;
    VersionNumber VerificationCredentialVersion() const final;
    std::unique_ptr<proto::VerificationSet> VerificationSet() const final;
    bool VerifyPseudonym(const PasswordPrompt& reason) const final;
    bool WriteCredentials() const final;

    std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) final;
    bool AddClaim(const Claim& claim, const PasswordPrompt& reason) final;
    bool AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) final;
    bool AddEmail(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) final;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    bool AddPaymentCode(
        const class PaymentCode& code,
        const proto::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) final;
#endif
    bool AddPreferredOTServer(
        const Identifier& id,
        const PasswordPrompt& reason,
        const bool primary) final;
    bool AddPhoneNumber(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) final;
    bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) final;
    bool DeleteClaim(const Identifier& id, const PasswordPrompt& reason) final;
    bool LoadCredentialIndex(
        const Serialized& index,
        const PasswordPrompt& reason) final;
    void SetAlias(const std::string& alias) final;
    void SetAliasStartup(const std::string& alias) final { alias_ = alias; }
    bool SetCommonName(const std::string& name, const PasswordPrompt& reason)
        final;
    bool SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) final;
    void SetDescription(const String& strLocation) final
    {
        eLock lock(shared_lock_);

        m_strDescription = strLocation;
    }
    bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const PasswordPrompt& reason,
        const bool primary) final;
    bool SetVerificationSet(
        const proto::VerificationSet& data,
        const PasswordPrompt& reason) final;
    bool Sign(
        const ProtobufType& input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        const proto::HashType hash) const final;
    bool Verify(
        const ProtobufType& input,
        proto::Signature& signature,
        const PasswordPrompt& reason) const final;

    ~Nym() final = default;

private:
    using mapOfCredentialSets =
        std::map<std::string, std::unique_ptr<identity::internal::Authority>>;

    friend opentxs::Factory;

    static const VersionConversionMap akey_to_session_key_version_;
    static const VersionConversionMap
        contact_credential_to_contact_data_version_;

    const api::Core& api_;
    std::int32_t version_;
    Bip32Index index_;
    std::string alias_;
    std::atomic<std::uint64_t> revision_;
    proto::CredentialIndexMode mode_;
    OTString m_strVersion;
    OTString m_strDescription;
    const OTNymID m_nymID;
    std::unique_ptr<identity::Source> source_;
    mutable std::unique_ptr<opentxs::ContactData> contact_data_;
    // The credentials for this Nym. (Each with a master key credential and
    // various child credentials.)
    mapOfCredentialSets m_mapCredentialSets;
    mapOfCredentialSets m_mapRevokedSets;
    // Revoked child credential IDs
    String::List m_listRevokedIDs;

    static NymParameters normalize(
        const api::Core& api,
        const NymParameters& in,
        const PasswordPrompt& reason) noexcept(false);

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
    bool set_contact_data(
        const eLock& lock,
        const proto::ContactData& data,
        const PasswordPrompt& reason);
    bool verify_pseudonym(const eLock& lock, const PasswordPrompt& reason)
        const;

    bool add_contact_credential(
        const eLock& lock,
        const proto::ContactData& data,
        const PasswordPrompt& reason);
    bool add_verification_credential(
        const eLock& lock,
        const proto::VerificationSet& data,
        const PasswordPrompt& reason);
    bool load_credential_index(
        const eLock& lock,
        const Serialized& index,
        const PasswordPrompt& reason);
    void revoke_contact_credentials(const eLock& lock);
    void revoke_verification_credentials(const eLock& lock);
    bool update_nym(
        const eLock& lock,
        const std::int32_t version,
        const PasswordPrompt& reason);

    Nym(const api::Core& api,
        NymParameters& nymParameters,
        std::unique_ptr<identity::Source> source,
        const PasswordPrompt& reason) noexcept(false);
    Nym(const api::Core& api,
        const identifier::Nym& nymID,
        const proto::CredentialIndexMode mode,
        const VersionNumber version,
        const Bip32Index index = 0);
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace opentxs::identity::implementation
