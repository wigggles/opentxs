// Copyright (c) 2019 The Open-Transactions developers
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
    VersionNumber ContactCredentialVersion() const override
    {
        return authority_to_contact_.at(version_);
    }
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const override;
    const credential::Primary& GetMasterCredential() const override
    {
        return *master_;
    }
    const std::string GetMasterCredID() const override;
    const std::string& GetNymID() const override { return m_strNymID; }
    const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const override;
    const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Keypair& GetAuthKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Keypair& GetEncrKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    const crypto::key::Keypair& GetSignKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const override;
    bool GetVerificationSet(std::unique_ptr<proto::VerificationSet>&
                                verificationSet) const override;
    bool hasCapability(const NymCapability& capability) const override;
    bool Path(proto::HDPath& output) const override;
    std::shared_ptr<Serialized> Serialize(
        const CredentialIndexModeFlag mode) const override;
    bool Sign(
        const credential::Primary& credential,
        proto::Signature& sig,
        const PasswordPrompt& reason) const override;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const proto::HashType hash = proto::HASHTYPE_BLAKE2B256) const override;
    const NymIDSource& Source() const override { return *nym_id_source_; }
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const override;
    VersionNumber VerificationCredentialVersion() const override
    {
        return authority_to_verification_.at(version_);
    }
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const PasswordPrompt& reason,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const override;
    bool Verify(const proto::Verification& item, const PasswordPrompt& reason)
        const override;
    bool VerifyInternally(const PasswordPrompt& reason) const override;

    std::string AddChildKeyCredential(
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) override;
    bool AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) override;
    bool AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) override;
    void RevokeContactCredentials(
        std::list<std::string>& contactCredentialIDs) override;
    void RevokeVerificationCredentials(
        std::list<std::string>& verificationCredentialIDs) override;
    void SetSource(const std::shared_ptr<NymIDSource>& source) override;
    bool WriteCredentials() const override;

    ~Authority() override = default;

private:
    friend opentxs::Factory;

    using ContactCredentialMap =
        std::map<OTIdentifier, std::unique_ptr<credential::internal::Contact>>;
    using KeyCredentialMap = std::
        map<OTIdentifier, std::unique_ptr<credential::internal::Secondary>>;
    using VerificationCredentialMap = std::
        map<OTIdentifier, std::unique_ptr<credential::internal::Verification>>;
    using mapOfCredentials =
        std::map<std::string, std::unique_ptr<credential::internal::Base>>;

    static const VersionConversionMap authority_to_contact_;
    static const VersionConversionMap authority_to_primary_;
    static const VersionConversionMap authority_to_secondary_;
    static const VersionConversionMap authority_to_verification_;
    static const VersionConversionMap nym_to_authority_;

    const api::Core& api_;
    std::unique_ptr<credential::internal::Primary> master_;
    KeyCredentialMap key_credentials_;
    ContactCredentialMap contact_credentials_;
    VerificationCredentialMap verification_credentials_;
    mapOfCredentials m_mapRevokedCredentials;
    std::string m_strNymID;
    std::shared_ptr<NymIDSource> nym_id_source_;
    const OTPassword* m_pImportPassword = nullptr;
    VersionNumber version_{0};
    Bip32Index index_{0};
    proto::KeyMode mode_{proto::KEYMODE_ERROR};

    static bool is_revoked(
        const std::string& id,
        const String::List* plistRevokedIDs);

    const crypto::key::Keypair& get_keypair(
        const proto::AsymmetricKeyType type,
        const proto::KeyRole role,
        const String::List* plistRevokedIDs) const;
    const credential::Base* get_secondary_credential(
        const std::string& strSubID,
        const String::List* plistRevokedIDs = nullptr) const;

    template <typename Item>
    bool validate_credential(const Item& item, const PasswordPrompt& reason)
        const;

    bool CreateMasterCredential(
        const NymParameters& nymParameters,
        const PasswordPrompt& reason);
    bool Load_Master(
        const String& strNymID,
        const String& strMasterCredID,
        const PasswordPrompt& reason);
    bool LoadChildKeyCredential(
        const String& strSubID,
        const PasswordPrompt& reason);
    bool LoadChildKeyCredential(
        const proto::Credential& serializedCred,
        const PasswordPrompt& reason);

    Authority() = delete;
    Authority(
        const api::Core& api,
        const VersionNumber version,
        const Bip32Index index = 0,
        const proto::KeyMode mode = proto::KEYMODE_PRIVATE,
        const std::string& nymID = "") noexcept;
    Authority(
        const api::Core& api,
        const proto::KeyMode mode,
        const Serialized& serializedAuthority,
        const PasswordPrompt& reason) noexcept;
    Authority(
        const api::Core& api,
        const NymParameters& nymParameters,
        VersionNumber nymVersion,
        const PasswordPrompt& reason) noexcept;
    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    Authority& operator=(const Authority&) = delete;
    Authority& operator=(Authority&&) = delete;
};
}  // namespace opentxs::identity::implementation
