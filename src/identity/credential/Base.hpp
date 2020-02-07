// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "core/contract/Signable.hpp"
#include "internal/identity/credential/Credential.hpp"

namespace opentxs::identity::credential::implementation
{
class Base : virtual public credential::internal::Base,
             public opentxs::contract::implementation::Signable
{
public:
    using SerializedType = proto::Credential;

    std::string asString(const bool asPrivate = false) const final;
    const Identifier& CredentialID() const final { return id_.get(); }
    bool GetContactData(
        std::unique_ptr<proto::ContactData>& output) const override
    {
        return false;
    }
    bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& output) const override
    {
        return false;
    }
    bool hasCapability(const NymCapability& capability) const override
    {
        return false;
    }
    Signature MasterSignature() const final;
    proto::KeyMode Mode() const final { return mode_; }
    proto::CredentialRole Role() const final { return role_; }
    bool Private() const final { return (proto::KEYMODE_PRIVATE == mode_); }
    bool Save() const final;
    Signature SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const final;
    OTData Serialize() const final;
    std::shared_ptr<SerializedType> Serialized(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const final;
    Signature SourceSignature() const final;
    bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey,
        const PasswordPrompt& reason) const override;
    proto::CredentialType Type() const final { return type_; }
    bool Validate() const final;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const override
    {
        return false;
    }
    bool Verify(
        const proto::Credential& credential,
        const proto::CredentialRole& role,
        const Identifier& masterID,
        const proto::Signature& masterSig) const override
    {
        return false;
    }

    void ReleaseSignatures(const bool onlyPrivate) final;

    ~Base() override = default;

protected:
    const identity::internal::Authority& parent_;
    const identity::Source& source_;
    const std::string nym_id_;
    const std::string master_id_;
    const proto::CredentialType type_;
    const proto::CredentialRole role_;
    const proto::KeyMode mode_;

    static std::string get_master_id(const internal::Primary& master) noexcept;
    static std::string get_master_id(
        const proto::Credential& serialized,
        const internal::Primary& master) noexcept(false);

    virtual std::shared_ptr<SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const;
    bool validate(const Lock& lock) const final;
    virtual bool verify_internally(const Lock& lock) const;

    void init(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);
    virtual void sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);

    Base(
        const api::internal::Core& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const proto::CredentialRole role,
        const proto::KeyMode mode,
        const std::string& masterID) noexcept;
    Base(
        const api::internal::Core& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const proto::Credential& serialized,
        const std::string& masterID) noexcept(false);

private:
    static Signatures extract_signatures(const SerializedType& serialized);

    Base* clone() const noexcept final { return nullptr; }
    OTIdentifier GetID(const Lock& lock) const final;
    // Syntax (non cryptographic) validation
    bool isValid(const Lock& lock) const;
    // Returns the serialized form to prevent unnecessary serializations
    bool isValid(const Lock& lock, std::shared_ptr<SerializedType>& credential)
        const;
    std::string Name() const final { return id_->str(); }
    bool verify_master_signature(const Lock& lock) const;

    void add_master_signature(
        const Lock& lock,
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    Base& operator=(const Base&) = delete;
    Base& operator=(Base&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
