// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "internal/identity/credential/Credential.hpp"

namespace opentxs::identity::credential::implementation
{
class Base : virtual public credential::internal::Base
{
public:
    using SerializedType = proto::Credential;

    std::string asString(const bool asPrivate = false) const override;
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const override;
    bool GetVerificationSet(std::unique_ptr<proto::VerificationSet>&
                                verificationSet) const override;
    bool hasCapability(const NymCapability& capability) const override;
    const std::string& MasterID() const override { return master_id_; }
    SerializedSignature MasterSignature() const override;
    proto::KeyMode Mode() const override { return mode_; }
    const std::string& NymID() const override { return nym_id_; }
    proto::CredentialRole Role() const override { return role_; }
    bool Private() const override { return (proto::KEYMODE_PRIVATE == mode_); }
    bool Save() const override;
    SerializedSignature SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const override;
    OTData Serialize() const override;
    std::shared_ptr<SerializedType> Serialized(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;
    SerializedSignature SourceSignature() const override;
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const override;
    proto::CredentialType Type() const override;
    bool Validate(const PasswordPrompt& reason) const override;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const PasswordPrompt& reason,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const override;
    bool Verify(
        const proto::Credential& credential,
        const proto::CredentialRole& role,
        const Identifier& masterID,
        const proto::Signature& masterSig,
        const PasswordPrompt& reason) const override;

    void ReleaseSignatures(const bool onlyPrivate) override;

    ~Base() override = default;

protected:
    const api::Core& api_;
    const identity::internal::Authority& parent_;
    const proto::CredentialType type_;
    const proto::CredentialRole role_;
    const proto::KeyMode mode_;
    const std::string master_id_;
    const std::string nym_id_;

    virtual std::shared_ptr<SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const;
    bool validate(const Lock& lock, const PasswordPrompt& reason)
        const override;
    virtual bool verify_internally(
        const Lock& lock,
        const PasswordPrompt& reason) const;

    bool AddMasterSignature(const Lock& lock, const PasswordPrompt& reason);
    bool New(const NymParameters& nymParameters, const PasswordPrompt& reason)
        override;

    Base(
        const api::Core& api,
        const identity::internal::Authority& owner,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::CredentialRole role,
        const proto::KeyMode mode,
        const std::string& masterID,
        const std::string& nymID) noexcept;
    Base(
        const api::Core& api,
        const identity::internal::Authority& owner,
        const proto::Credential& serializedCred,
        const std::string& masterID) noexcept;

private:
    OTIdentifier GetID(const Lock& lock) const override;
    // Syntax (non cryptographic) validation
    bool isValid(const Lock& lock) const;
    // Returns the serialized form to prevent unnecessary serializations
    bool isValid(const Lock& lock, std::shared_ptr<SerializedType>& credential)
        const;
    std::string Name() const override;
    bool VerifyMasterID() const;
    bool VerifyNymID() const;
    bool verify_master_signature(const Lock& lock, const PasswordPrompt& reason)
        const;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    Base& operator=(const Base&) = delete;
    Base& operator=(Base&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
