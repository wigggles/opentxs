// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/Proto.tpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"

#include <list>
#include <memory>
#include <ostream>
#include <string>

#include "Base.tpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Base::"

namespace opentxs::identity::credential::implementation
{
Base::Base(
    const api::Core& api,
    identity::internal::Authority& theOwner,
    const NymParameters& nymParameters,
    const VersionNumber version)
    : Signable({}, version)
    , api_(api)
    , type_(nymParameters.credentialType())
    , mode_(proto::KEYMODE_PRIVATE)
    , owner_backlink_(&theOwner)
{
}

Base::Base(
    const api::Core& api,
    identity::internal::Authority& theOwner,
    const proto::Credential& serializedCred)
    : Signable({}, serializedCred.version())
    , api_(api)
    , type_(serializedCred.type())
    , role_(serializedCred.role())
    , mode_(serializedCred.mode())
    , owner_backlink_(&theOwner)
{
    if (serializedCred.has_nymid()) {
        nym_id_ = serializedCred.nymid();
        id_ = Identifier::Factory(serializedCred.id());
    }

    SerializedSignature sig;
    for (auto& it : serializedCred.signature()) {
        sig.reset(new proto::Signature(it));
        signatures_.push_back(sig);
    }
}

bool Base::New(const NymParameters&, const PasswordPrompt& reason)
{
    Lock lock(lock_);
    bool output = false;

    output = CalculateID(lock);

    OT_ASSERT(output);

    if (output && (proto::CREDROLE_MASTERKEY != role_)) {
        output = AddMasterSignature(lock, reason);
    }

    OT_ASSERT(output);

    return output;
}

/** Verify that nym_id_ matches the nymID of the parent credential set */
bool Base::VerifyNymID() const
{

    return (nym_id_ == owner_backlink_->GetNymID());
}

/** Verify that master_id_ matches the MasterID of the parent credential set */
bool Base::VerifyMasterID() const
{
    // This check is not applicable to master credentials
    if (proto::CREDROLE_MASTERKEY == role_) { return true; }

    const std::string parent = owner_backlink_->GetMasterCredID();
    const std::string child = master_id_;

    return (parent == child);
}

/** Verifies the cryptographic integrity of a credential. Assumes the
 * Authority specified by owner_backlink_ is valid. */
bool Base::verify_internally(const Lock& lock, const PasswordPrompt& reason)
    const
{
    OT_ASSERT(nullptr != owner_backlink_);

    if (nullptr == owner_backlink_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This credential is not attached to a Authority. Can not verify.")
            .Flush();

        return false;
    }

    if (!VerifyNymID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": NymID for this credential does not match NymID of parent "
            "Authority.")
            .Flush();

        return false;
    }

    if (!VerifyMasterID()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": MasterID for this credential does not match MasterID of parent "
            "Authority.")
            .Flush();

        return false;
    }

    if (!CheckID(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Purported ID for this credential does not match its actual "
            "contents.")
            .Flush();

        return false;
    }

    bool GoodMasterSignature = false;

    if (proto::CREDROLE_MASTERKEY == role_) {
        GoodMasterSignature = true;  // Covered by VerifySignedBySelf()
    } else {
        GoodMasterSignature = verify_master_signature(lock, reason);
    }

    if (!GoodMasterSignature) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": This credential hasn't been signed by its  master credential.")
            .Flush();

        return false;
    }

    return true;
}

bool Base::verify_master_signature(
    const Lock& lock,
    const PasswordPrompt& reason) const
{
    OT_ASSERT(owner_backlink_);

    auto serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);

    auto masterSig = MasterSignature();

    if (!masterSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing master signature.")
            .Flush();

        return false;
    }

    return (owner_backlink_->GetMasterCredential().Verify(
        *serialized,
        role_,
        Identifier::Factory(MasterID()),
        *masterSig,
        reason));
}

SerializedSignature Base::MasterSignature() const
{
    SerializedSignature masterSignature;
    proto::SignatureRole targetRole = proto::SIGROLE_PUBCREDENTIAL;

    const std::string master = MasterID();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) && (it->credentialid() == master)) {

            masterSignature = it;
            break;
        }
    }

    return masterSignature;
}

/** Perform syntax (non-cryptographic) verifications of a credential */
bool Base::isValid(const Lock& lock) const
{
    std::shared_ptr<SerializedType> serializedProto;

    return isValid(lock, serializedProto);
}

/** Returns the serialized form to prevent unnecessary serializations */
bool Base::isValid(
    const Lock& lock,
    std::shared_ptr<SerializedType>& credential) const
{
    SerializationModeFlag serializationMode = AS_PUBLIC;

    if (proto::KEYMODE_PRIVATE == mode_) { serializationMode = AS_PRIVATE; }

    credential = serialize(lock, serializationMode, WITH_SIGNATURES);

    return proto::Validate<proto::Credential>(
        *credential,
        VERBOSE,
        mode_,
        role_,
        true);  // with signatures
}

bool Base::validate(const Lock& lock, const PasswordPrompt& reason) const
{
    // Check syntax
    if (!isValid(lock)) { return false; }

    // Check cryptographic requirements
    return verify_internally(lock, reason);
}

bool Base::Validate(const PasswordPrompt& reason) const
{
    Lock lock(lock_);

    return validate(lock, reason);
}

OTIdentifier Base::GetID(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto idVersion = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);

    if (idVersion->has_id()) { idVersion->clear_id(); }

    auto serializedData = api_.Factory().Data(*idVersion);
    auto id = Identifier::Factory();

    if (!id->CalculateDigest(serializedData)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error calculating credential digest.")
            .Flush();
    }

    return id;
}

proto::CredentialType Base::Type() const { return type_; }

std::shared_ptr<Base::SerializedType> Base::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = std::make_shared<proto::Credential>();
    serializedCredential->set_version(version_);
    serializedCredential->set_type(static_cast<proto::CredentialType>(type_));
    serializedCredential->set_role(static_cast<proto::CredentialRole>(role_));

    if (proto::CREDROLE_MASTERKEY != role_) {
        std::unique_ptr<proto::ChildCredentialParameters> parameters;
        parameters.reset(new proto::ChildCredentialParameters);

        parameters->set_version(1);
        parameters->set_masterid(MasterID());
        serializedCredential->set_allocated_childdata(parameters.release());
    }

    if (asPrivate) {
        if (proto::KEYMODE_PRIVATE == mode_) {
            serializedCredential->set_mode(mode_);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Can't serialized a public "
                "credential as a private credential.")
                .Flush();
        }
    } else {
        serializedCredential->set_mode(proto::KEYMODE_PUBLIC);
    }

    if (asSigned) {
        if (asPrivate) {
            auto privateSig = SelfSignature(PRIVATE_VERSION);

            if (privateSig) {
                *serializedCredential->add_signature() = *privateSig;
            }
        }

        auto publicSig = SelfSignature(PUBLIC_VERSION);

        if (publicSig) { *serializedCredential->add_signature() = *publicSig; }

        auto sourceSig = SourceSignature();

        if (sourceSig) { *serializedCredential->add_signature() = *sourceSig; }
    } else {
        serializedCredential->clear_signature();  // just in case...
    }

    serializedCredential->set_id(id(lock)->str());
    serializedCredential->set_nymid(NymID());

    return serializedCredential;
}

SerializedSignature Base::SelfSignature(CredentialModeFlag version) const
{
    proto::SignatureRole targetRole;

    if (PRIVATE_VERSION == version) {
        targetRole = proto::SIGROLE_PRIVCREDENTIAL;
    } else {
        targetRole = proto::SIGROLE_PUBCREDENTIAL;
    }

    const std::string self = String::Factory(id_)->Get();

    for (auto& it : signatures_) {
        if ((it->role() == targetRole) && (it->credentialid() == self)) {

            return it;
        }
    }

    return nullptr;
}

SerializedSignature Base::SourceSignature() const
{
    SerializedSignature signature;

    const std::string source = String::Factory(NymID())->Get();

    for (auto& it : signatures_) {
        if ((it->role() == proto::SIGROLE_NYMIDSOURCE) &&
            (it->credentialid() == source)) {
            signature = std::make_shared<proto::Signature>(*it);

            break;
        }
    }
    return signature;
}

bool Base::Save() const
{
    Lock lock(lock_);

    std::shared_ptr<SerializedType> serializedProto;

    if (!isValid(lock, serializedProto)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to save serialized "
            "credential. Type (")(role_)("), version ")(version_)(".")
            .Flush();

        return false;
    }

    const bool bSaved = api_.Wallet().SaveCredential(*serializedProto);

    if (!bSaved) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error saving credential.")
            .Flush();

        return false;
    }

    return true;
}

OTData Base::Serialize() const
{
    auto serialized =
        Serialized(Private() ? AS_PRIVATE : AS_PUBLIC, WITH_SIGNATURES);

    return api_.Factory().Data(*serialized);
}

std::string Base::asString(const bool asPrivate) const
{
    std::shared_ptr<SerializedType> credenial;
    auto dataCredential = Data::Factory();
    auto stringCredential = String::Factory();
    credenial = Serialized(asPrivate, WITH_SIGNATURES);
    dataCredential = api_.Factory().Data(*credenial);
    auto armoredCredential = api_.Factory().Armored(dataCredential);
    armoredCredential->WriteArmoredString(stringCredential, "Credential");

    return stringCredential->Get();
}

void Base::ReleaseSignatures(const bool onlyPrivate)
{
    for (auto i = signatures_.begin(); i != signatures_.end();) {
        if (!onlyPrivate ||
            (onlyPrivate && (proto::SIGROLE_PRIVCREDENTIAL == (*i)->role()))) {
            i = signatures_.erase(i);
        } else {
            i++;
        }
    }
}

bool Base::AddMasterSignature(const Lock& lock, const PasswordPrompt& reason)
{
    if (nullptr == owner_backlink_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Missing master credential.")
            .Flush();

        return false;
    }

    SerializedSignature serializedMasterSignature =
        std::make_shared<proto::Signature>();
    auto serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
    auto& signature = *serialized->add_signature();

    bool havePublicSig = owner_backlink_->Sign(
        [&serialized]() -> std::string { return proto::ToString(*serialized); },
        proto::SIGROLE_PUBCREDENTIAL,
        signature,
        reason);

    if (!havePublicSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain signature from master credential.")
            .Flush();

        return false;
    }
    serializedMasterSignature->CopyFrom(signature);
    signatures_.push_back(serializedMasterSignature);

    return true;
}

/** Override this method for credentials capable of returning contact data. */
bool Base::GetContactData(std::unique_ptr<proto::ContactData>&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of returning verification sets.
 */
bool Base::GetVerificationSet(std::unique_ptr<proto::VerificationSet>&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of verifying signatures */
bool Base::Verify(
    const Data&,
    const proto::Signature&,
    const PasswordPrompt&,
    const proto::KeyRole) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of verifying other credentials
 */
bool Base::Verify(
    const proto::Credential&,
    const proto::CredentialRole&,
    const Identifier&,
    const proto::Signature&,
    const PasswordPrompt&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

/** Override this method for credentials capable of deriving transport keys */
bool Base::TransportKey(Data&, OTPassword&, const PasswordPrompt&) const
{
    OT_ASSERT_MSG(false, "This method was called on the wrong credential.");

    return false;
}

bool Base::hasCapability(const NymCapability&) const { return false; }

std::string Base::Name() const { return id_->str(); }

std::shared_ptr<Base::SerializedType> Base::Serialized(
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    Lock lock(lock_);

    return serialize(lock, asPrivate, asSigned);
}
}  // namespace opentxs::identity::credential::implementation
