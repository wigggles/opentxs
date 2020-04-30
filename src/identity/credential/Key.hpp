// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "identity/credential/Base.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/key/Keypair.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace identity
{
namespace internal
{
struct Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace proto
{
class Credential;
class KeyCredential;
class Signature;
}  // namespace proto

class Data;
class NymParameters;
class OTPassword;
class PasswordPrompt;
class Signature;
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
class Key : virtual public credential::internal::Key,
            public credential::implementation::Base
{
public:
    const crypto::key::Keypair& GetKeypair(
        const proto::KeyRole role) const final
    {
        return GetKeypair(proto::AKEYTYPE_NULL, role);
    }
    const crypto::key::Keypair& GetKeypair(
        const proto::AsymmetricKeyType type,
        const proto::KeyRole role) const final;
    std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const opentxs::Signature& theSignature,
        char cKeyType = '0') const final;
    bool hasCapability(const NymCapability& capability) const override;
    using Base::Verify;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key) const final;
    bool Sign(
        const GetPreimage input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        proto::KeyRole key,
        const proto::HashType hash) const final;
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const final;

    bool SelfSign(
        const PasswordPrompt& reason,
        const OTPassword* exportPassword = nullptr,
        const bool onlyPrivate = false) final;

    ~Key() override = default;

protected:
    const VersionNumber subversion_;
    const OTKeypair signing_key_;
    const OTKeypair authentication_key_;
    const OTKeypair encryption_key_;

    std::shared_ptr<Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;
    bool verify_internally(const Lock& lock) const override;

    void sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false) override;

    Key(const api::internal::Core& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::CredentialRole role,
        const PasswordPrompt& reason,
        const std::string& masterID,
        const bool useProvidedSigningKey = false) noexcept(false);
    Key(const api::internal::Core& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const proto::Credential& serializedCred,
        const std::string& masterID) noexcept(false);

private:
    static const VersionConversionMap credential_subversion_;
    static const VersionConversionMap subversion_to_key_version_;

    static OTKeypair deserialize_key(
        const api::internal::Core& api,
        const int index,
        const proto::Credential& credential);
    static OTKeypair new_key(
        const api::internal::Core& api,
        const proto::KeyRole role,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason,
        const ReadView dh = {}) noexcept(false);
    static OTKeypair signing_key(
        const api::internal::Core& api,
        const NymParameters& params,
        const VersionNumber subversion,
        const bool useProvided,
        const PasswordPrompt& reason) noexcept(false);

    bool addKeytoSerializedKeyCredential(
        proto::KeyCredential& credential,
        const bool getPrivate,
        const proto::KeyRole role) const;
    bool addKeyCredentialtoSerializedCredential(
        std::shared_ptr<Base::SerializedType> credential,
        const bool addPrivate) const;
    bool VerifySig(
        const Lock& lock,
        const proto::Signature& sig,
        const CredentialModeFlag asPrivate = PRIVATE_VERSION) const;
    bool VerifySignedBySelf(const Lock& lock) const;

    Key() = delete;
    Key(const Key&) = delete;
    Key(Key&&) = delete;
    Key& operator=(const Key&) = delete;
    Key& operator=(Key&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
