// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_AUTHORITY_HPP
#define OPENTXS_IDENTITY_AUTHORITY_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>

namespace opentxs
{
namespace identity
{
class Authority
{
public:
    using Serialized = proto::Authority;

    OPENTXS_EXPORT virtual VersionNumber ContactCredentialVersion() const = 0;
    OPENTXS_EXPORT virtual bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const = 0;
    OPENTXS_EXPORT virtual OTIdentifier GetMasterCredID() const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Keypair& GetAuthKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Keypair& GetEncrKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual const crypto::key::Keypair& GetSignKeypair(
        proto::AsymmetricKeyType keytype,
        const String::List* plistRevokedIDs = nullptr) const = 0;
    OPENTXS_EXPORT virtual bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& verificationSet) const = 0;
    OPENTXS_EXPORT virtual bool hasCapability(
        const NymCapability& capability) const = 0;
    OPENTXS_EXPORT virtual bool Path(proto::HDPath& output) const = 0;
    OPENTXS_EXPORT virtual std::shared_ptr<Serialized> Serialize(
        const CredentialIndexModeFlag mode) const = 0;
    OPENTXS_EXPORT virtual bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const proto::HashType hash = proto::HASHTYPE_BLAKE2B256) const = 0;
    OPENTXS_EXPORT virtual const identity::Source& Source() const = 0;
    OPENTXS_EXPORT virtual bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual VersionNumber VerificationCredentialVersion()
        const = 0;
    OPENTXS_EXPORT virtual bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const PasswordPrompt& reason,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const = 0;
    OPENTXS_EXPORT virtual bool Verify(
        const proto::Verification& item,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual bool VerifyInternally(
        const PasswordPrompt& reason) const = 0;

    OPENTXS_EXPORT virtual std::string AddChildKeyCredential(
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual bool AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual bool AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual void RevokeContactCredentials(
        std::list<std::string>& contactCredentialIDs) = 0;
    OPENTXS_EXPORT virtual void RevokeVerificationCredentials(
        std::list<std::string>& verificationCredentialIDs) = 0;

    OPENTXS_EXPORT virtual ~Authority() = default;

protected:
    Authority() noexcept = default;

private:
    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    Authority& operator=(const Authority&) = delete;
    Authority& operator=(Authority&&) = delete;
};
}  // namespace identity
}  // namespace opentxs
#endif
