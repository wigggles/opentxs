// Copyright (c) 2018 The Open-Transactions developers
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
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const override;
    const MasterCredential& GetMasterCredential() const override
    {
        return *m_MasterCredential;
    }
    const std::string GetMasterCredID() const override;
    const std::string& GetNymID() const override;
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
        const MasterCredential& credential,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr) const override;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const OTPasswordData* pPWData = nullptr,
        const proto::HashType hash = proto::HASHTYPE_BLAKE2B256) const override;
    const NymIDSource& Source() const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const override;
    bool Verify(const proto::Verification& item) const override;
    bool VerifyInternally() const override;

    std::string AddChildKeyCredential(
        const NymParameters& nymParameters) override;
    bool AddVerificationCredential(
        const proto::VerificationSet& verificationSet) override;
    bool AddContactCredential(const proto::ContactData& contactData) override;
    bool ReEncryptPrivateCredentials(
        const OTPassword& theExportPassword,
        bool bImporting) override;
    void RevokeContactCredentials(
        std::list<std::string>& contactCredentialIDs) override;
    void RevokeVerificationCredentials(
        std::list<std::string>& verificationCredentialIDs) override;
    void SetSource(const std::shared_ptr<NymIDSource>& source) override;
    bool WriteCredentials() const override;

    Authority(const api::Core& api);
    Authority(
        const api::Core& api,
        const proto::KeyMode mode,
        const Serialized& serializedAuthority);
    Authority(
        const api::Core& api,
        const NymParameters& nymParameters,
        std::uint32_t version,
        const OTPasswordData* pPWData = nullptr);

    ~Authority() override;

private:
    friend opentxs::Factory;

    using mapOfCredentials = std::map<std::string, std::unique_ptr<Credential>>;

    const api::Core& api_;
    std::unique_ptr<MasterCredential> m_MasterCredential;
    mapOfCredentials m_mapCredentials;
    mapOfCredentials m_mapRevokedCredentials;
    std::string m_strNymID;
    std::shared_ptr<NymIDSource> nym_id_source_;
    const OTPassword* m_pImportPassword = nullptr;
    std::uint32_t version_{0};
    std::uint32_t index_{0};
    proto::KeyMode mode_{proto::KEYMODE_ERROR};

    const Credential* GetChildCredential(
        const String& strSubID,
        const String::List* plistRevokedIDs = nullptr) const;

    void ClearChildCredentials();
    bool CreateMasterCredential(const NymParameters& nymParameters);
    bool Load_Master(
        const String& strNymID,
        const String& strMasterCredID,
        const OTPasswordData* pPWData = nullptr);
    bool LoadChildKeyCredential(const String& strSubID);
    bool LoadChildKeyCredential(const proto::Credential& serializedCred);

    Authority() = delete;
};
}  // namespace opentxs::identity::implementation
