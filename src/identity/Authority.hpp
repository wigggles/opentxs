// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::implementation
{
class Authority final : virtual public identity::internal::Authority
{
public:
    VersionNumber ContactCredentialVersion() const final
    {
        return authority_to_contact_.at(version_);
    }
    AuthorityKeys EncryptionTargets() const noexcept final;
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const final;
    const credential::Primary& GetMasterCredential() const final
    {
        return *master_;
    }
    OTIdentifier GetMasterCredID() const final;
    const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const final;
    const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Keypair& GetAuthKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Keypair& GetEncrKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const crypto::key::Keypair& GetSignKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const final;
    const credential::Key& GetTagCredential(
        proto::AsymmetricKeyType keytype) const noexcept(false) final;
    bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& verificationSet) const final;
    bool hasCapability(const NymCapability& capability) const final;
    ReadView Params(const proto::AsymmetricKeyType type) const noexcept final;
    bool Path(proto::HDPath& output) const final;
    std::shared_ptr<Serialized> Serialize(
        const CredentialIndexModeFlag mode) const final;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        proto::KeyRole key,
        const proto::HashType hash) const final;
    const identity::Source& Source() const final { return parent_.Source(); }
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const final;
    bool Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const proto::AsymmetricKeyType type,
        const crypto::key::Symmetric& key,
        PasswordPrompt& reason) const noexcept final;
    VersionNumber VerificationCredentialVersion() const final
    {
        return authority_to_verification_.at(version_);
    }
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key) const final;
    bool Verify(const proto::Verification& item) const final;
    bool VerifyInternally() const final;

    std::string AddChildKeyCredential(
        const NymParameters& parameters,
        const PasswordPrompt& reason) final;
    bool AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) final;
    bool AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) final;
    void RevokeContactCredentials(
        std::list<std::string>& contactCredentialIDs) final;
    void RevokeVerificationCredentials(
        std::list<std::string>& verificationCredentialIDs) final;
    bool WriteCredentials() const final;

    ~Authority() final = default;

private:
    friend opentxs::Factory;
    friend internal::Authority;

    using ContactCredentialMap =
        std::map<OTIdentifier, std::unique_ptr<credential::internal::Contact>>;
    using KeyCredentialMap = std::
        map<OTIdentifier, std::unique_ptr<credential::internal::Secondary>>;
    using KeyCredentialItem = std::
        pair<OTIdentifier, std::unique_ptr<credential::internal::Secondary>>;
    using VerificationCredentialMap = std::
        map<OTIdentifier, std::unique_ptr<credential::internal::Verification>>;
    using mapOfCredentials =
        std::map<std::string, std::unique_ptr<credential::internal::Base>>;

    static const VersionConversionMap authority_to_contact_;
    static const VersionConversionMap authority_to_primary_;
    static const VersionConversionMap authority_to_secondary_;
    static const VersionConversionMap authority_to_verification_;
    static const VersionConversionMap nym_to_authority_;

    const api::internal::Core& api_;
    const identity::Nym& parent_;
    const VersionNumber version_{0};
    std::uint32_t index_{0};
    std::unique_ptr<credential::internal::Primary> master_;
    KeyCredentialMap key_credentials_;
    ContactCredentialMap contact_credentials_;
    VerificationCredentialMap verification_credentials_;
    mapOfCredentials m_mapRevokedCredentials;
    proto::KeyMode mode_{proto::KEYMODE_ERROR};

    static bool is_revoked(
        const std::string& id,
        const String::List* plistRevokedIDs);
    static KeyCredentialMap create_child_credential(
        const api::internal::Core& api,
        const NymParameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        Bip32Index& index,
        const opentxs::PasswordPrompt& reason) noexcept(false);
    static ContactCredentialMap create_contact_credental(
        const api::internal::Core& api,
        const NymParameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        const opentxs::PasswordPrompt& reason) noexcept(false);
    static KeyCredentialItem create_key_credential(
        const api::internal::Core& api,
        const NymParameters& parameters,
        const identity::Source& source,
        const credential::internal::Primary& master,
        internal::Authority& parent,
        const VersionNumber parentVersion,
        Bip32Index& index,
        const opentxs::PasswordPrompt& reason) noexcept(false);
    static std::unique_ptr<credential::internal::Primary> create_master(
        const api::internal::Core& api,
        identity::internal::Authority& owner,
        const identity::Source& source,
        const VersionNumber version,
        const NymParameters& parameters,
        const Bip32Index index,
        const opentxs::PasswordPrompt& reason) noexcept(false);
    template <typename Type>
    static void extract_child(
        const api::internal::Core& api,
        const identity::Source& source,
        internal::Authority& authority,
        const credential::internal::Primary& master,
        const credential::Base::SerializedType& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role,
        std::map<OTIdentifier, std::unique_ptr<Type>>& map) noexcept(false);
    static std::unique_ptr<credential::internal::Primary> load_master(
        const api::internal::Core& api,
        identity::internal::Authority& owner,
        const identity::Source& source,
        const proto::KeyMode mode,
        const Serialized& serialized) noexcept(false);
    template <typename Type>
    static std::map<OTIdentifier, std::unique_ptr<Type>> load_child(
        const api::internal::Core& api,
        const identity::Source& source,
        internal::Authority& authority,
        const credential::internal::Primary& master,
        const Serialized& serialized,
        const proto::KeyMode mode,
        const proto::CredentialRole role) noexcept(false);

    const crypto::key::Keypair& get_keypair(
        const proto::AsymmetricKeyType type,
        const proto::KeyRole role,
        const String::List* plistRevokedIDs) const;
    const credential::Base* get_secondary_credential(
        const std::string& strSubID,
        const String::List* plistRevokedIDs = nullptr) const;

    template <typename Item>
    bool validate_credential(const Item& item) const;

    bool LoadChildKeyCredential(const String& strSubID);
    bool LoadChildKeyCredential(const proto::Credential& serializedCred);

    Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const proto::KeyMode mode,
        const Serialized& serialized) noexcept(false);
    Authority(
        const api::internal::Core& api,
        const identity::Nym& parent,
        const identity::Source& source,
        const NymParameters& parameters,
        VersionNumber nymVersion,
        const PasswordPrompt& reason) noexcept(false);
    Authority() = delete;
    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    Authority& operator=(const Authority&) = delete;
    Authority& operator=(Authority&&) = delete;
};
}  // namespace opentxs::identity::implementation
