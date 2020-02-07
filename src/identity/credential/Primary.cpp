// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/Proto.tpp"

#include "internal/api/Api.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"
#include "identity/credential/Key.hpp"

#include <memory>
#include <ostream>

#include "Primary.hpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Primary::"

namespace opentxs
{
using ReturnType = identity::credential::implementation::Primary;

identity::credential::internal::Primary* Factory::PrimaryCredential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const NymParameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    try {

        return new ReturnType(api, parent, source, parameters, version, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to create credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}

identity::credential::internal::Primary* Factory::PrimaryCredential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const proto::Credential& serialized)
{
    try {

        return new ReturnType(api, parent, source, serialized);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to deserialize credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
const VersionConversionMap Primary::credential_to_master_params_{
    {1, 1},
    {2, 1},
    {3, 1},
    {4, 1},
    {5, 1},
    {6, 2},
};

Primary::Primary(
    const api::internal::Core& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const NymParameters& params,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : credential::implementation::Key(
          api,
          parent,
          source,
          params,
          version,
          proto::CREDROLE_MASTERKEY,
          reason,
          "",
          proto::SOURCETYPE_PUBKEY == params.SourceType())
    , source_proof_(source_proof(params))
{
    {
        Lock lock(lock_);
        first_time_init(lock);
    }

    init(*this, reason);
}

Primary::Primary(
    const api::internal::Core& api,
    const identity::internal::Authority& parent,
    const identity::Source& source,
    const proto::Credential& serialized) noexcept(false)
    : credential::implementation::Key(api, parent, source, serialized, "")
    , source_proof_(serialized.masterdata().sourceproof())
{
    Lock lock(lock_);
    init_serialized(lock);
}

bool Primary::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
            return signing_key_->CheckCapability(capability);
        }
        default: {
        }
    }

    return false;
}

bool Primary::Path(proto::HDPath& output) const
{
    try {
        const bool found = signing_key_->GetPrivateKey().Path(output);

        if (found) { output.mutable_child()->RemoveLast(); }

        return found;
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": No private key.").Flush();

        return false;
    }
}

std::string Primary::Path() const
{
    return signing_key_->GetPrivateKey().Path();
}

std::shared_ptr<identity::credential::Base::SerializedType> Primary::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto output = Key::serialize(lock, asPrivate, asSigned);

    OT_ASSERT(output);

    auto& serialized = *output;
    serialized.set_role(proto::CREDROLE_MASTERKEY);
    auto& masterData = *serialized.mutable_masterdata();
    masterData.set_version(credential_to_master_params_.at(version_));
    *masterData.mutable_source() = *(source_.Serialize());
    *masterData.mutable_sourceproof() = source_proof_;

    return output;
}

void Primary::sign(
    const identity::credential::internal::Primary& master,
    const PasswordPrompt& reason) noexcept(false)
{
    Key::sign(master, reason);

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE != source_proof_.type()) {
        auto sig = std::make_shared<proto::Signature>();

        OT_ASSERT(sig);

        if (false == source_.Sign(*this, *sig, reason)) {
            throw std::runtime_error("Failed to obtain source signature");
        }

        signatures_.push_back(sig);
    }
}

proto::SourceProof Primary::source_proof(const NymParameters& params)
{
    auto output = proto::SourceProof{};
    output.set_version(1);
    output.set_type(params.SourceProofType());

    return output;
}

bool Primary::Verify(
    const proto::Credential& credential,
    const proto::CredentialRole& role,
    const Identifier& masterID,
    const proto::Signature& masterSig) const
{
    if (!proto::Validate<proto::Credential>(
            credential, VERBOSE, proto::KEYMODE_PUBLIC, role, false)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid credential syntax.")
            .Flush();

        return false;
    }

    bool sameMaster = (id_ == masterID);

    if (!sameMaster) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Credential does not designate this credential as its master.")
            .Flush();

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(credential);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(masterSig);
    signature.clear_signature();

    return Verify(api_.Factory().Data(copy), masterSig);
}

bool Primary::verify_against_source(const Lock& lock) const
{
    auto pSerialized = std::shared_ptr<proto::Credential>{};
    auto hasSourceSignature{true};

    switch (source_.Type()) {
        case proto::SOURCETYPE_PUBKEY: {
            pSerialized = serialize(lock, AS_PUBLIC, WITH_SIGNATURES);
            hasSourceSignature = false;
        } break;
        case proto::SOURCETYPE_BIP47: {
            pSerialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        } break;
        default: {
            return false;
        }
    }

    if (false == bool(pSerialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize credentials")
            .Flush();

        return false;
    }

    const auto& serialized = *pSerialized;
    const auto pSig = hasSourceSignature ? SourceSignature() : SelfSignature();

    if (false == bool(pSig)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credential not signed by its source.")
            .Flush();

        return false;
    }

    const auto& sig = *pSig;

    return source_.Verify(serialized, sig);
}

bool Primary::verify_internally(const Lock& lock) const
{
    // Perform common Key Credential verifications
    if (!Key::verify_internally(lock)) { return false; }

    // Check that the source validates this credential
    if (!verify_against_source(lock)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying master credential against "
            "nym id source.")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::identity::credential::implementation
