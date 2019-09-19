// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/Proto.tpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"
#include "identity/credential/Key.hpp"

#include <memory>
#include <ostream>

#include "Primary.hpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Primary::"

namespace opentxs
{
identity::credential::internal::Primary* Factory::PrimaryCredential(
    const api::Core& api,
    const opentxs::PasswordPrompt& reason,
    identity::internal::Authority& parent,
    const proto::Credential& serialized)
{
    return new identity::credential::implementation::Primary(
        api, reason, parent, serialized);
}

identity::credential::internal::Primary* Factory::PrimaryCredential(
    const api::Core& api,
    identity::internal::Authority& parent,
    const NymParameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    return new identity::credential::implementation::Primary(
        api, parent, parameters, version, reason);
}
}  // namespace opentxs

namespace opentxs::identity::credential::implementation
{
Primary::Primary(
    const api::Core& api,
    const opentxs::PasswordPrompt& reason,
    identity::internal::Authority& theOwner,
    const proto::Credential& serialized)
    : Signable({}, serialized.version())  // TODO Signable
    , credential::implementation::Key(api, reason, theOwner, serialized)
{
    role_ = proto::CREDROLE_MASTERKEY;
    auto source = std::make_shared<NymIDSource>(
        api_.Factory(), serialized.masterdata().source(), reason);
    owner_backlink_->SetSource(source);
    source_proof_.reset(
        new proto::SourceProof(serialized.masterdata().sourceproof()));
}

Primary::Primary(
    const api::Core& api,
    identity::internal::Authority& theOwner,
    const NymParameters& nymParameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
    : Signable({}, version)  // TODO Signable
    , credential::implementation::Key(
          api,
          theOwner,
          nymParameters,
          version,
          reason)
{
    role_ = proto::CREDROLE_MASTERKEY;

    std::shared_ptr<NymIDSource> source;
    auto sourceProof = std::make_unique<proto::SourceProof>();

    proto::SourceProofType proofType = nymParameters.SourceProofType();

    if (proto::SOURCETYPE_PUBKEY == nymParameters.SourceType()) {
        OT_ASSERT_MSG(
            proto::SOURCEPROOFTYPE_SELF_SIGNATURE == proofType,
            "non self-signed credentials not yet implemented");

        source = std::make_shared<NymIDSource>(
            api_.Factory(),
            nymParameters,
            *(signing_key_->GetPublicKey().Serialize()),
            reason);
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SELF_SIGNATURE);

    }
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    else if (proto::SOURCETYPE_BIP47 == nymParameters.SourceType()) {
        sourceProof->set_version(1);
        sourceProof->set_type(proto::SOURCEPROOFTYPE_SIGNATURE);

        auto bip47Source = api_.Factory().PaymentCode(
            nymParameters.Seed(),
            nymParameters.Nym(),
            PAYMENT_CODE_VERSION,
            reason);
        source = std::make_shared<NymIDSource>(api_.Factory(), bip47Source);
    }
#endif

    source_proof_.reset(sourceProof.release());
    owner_backlink_->SetSource(source);
    std::string nymID = owner_backlink_->GetNymID();

    nym_id_ = nymID;
}

/** Verify that nym_id_ is the same as the hash of m_strSourceForNymID. Also
 * verify that *this == owner_backlink_->GetMasterCredential() (the master
 * credential.) Verify the (self-signed) signature on *this. */
bool Primary::verify_internally(
    const Lock& lock,
    const opentxs::PasswordPrompt& reason) const
{
    // Perform common Key Credential verifications
    if (!Key::verify_internally(lock, reason)) { return false; }

    // Check that the source validates this credential
    if (!verify_against_source(lock, reason)) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failed verifying master credential against "
            "nym id source.")
            .Flush();

        return false;
    }

    return true;
}

bool Primary::verify_against_source(
    const Lock& lock,
    const opentxs::PasswordPrompt& reason) const
{
    std::shared_ptr<proto::Credential> serialized;

    switch (owner_backlink_->Source().Type()) {
        case proto::SOURCETYPE_PUBKEY: {
            serialized = serialize(lock, AS_PUBLIC, WITH_SIGNATURES);
        } break;
        case proto::SOURCETYPE_BIP47: {
            serialized = serialize(lock, AS_PUBLIC, WITHOUT_SIGNATURES);
        } break;
        default: {
            return false;
        }
    }

    auto sourceSig = SourceSignature();

    if (!sourceSig) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Master credential not signed by its"
            " source.")
            .Flush();

        return false;
    }

    return owner_backlink_->Source().Verify(*serialized, *sourceSig, reason);
}

bool Primary::New(
    const NymParameters& nymParameters,
    const opentxs::PasswordPrompt& reason)
{
    if (!Key::New(nymParameters, reason)) { return false; }

    if (proto::SOURCEPROOFTYPE_SELF_SIGNATURE != source_proof_->type()) {
        SerializedSignature sig = std::make_shared<proto::Signature>();
        bool haveSourceSig = owner_backlink_->Sign(*this, *sig, reason);

        if (haveSourceSig) {
            signatures_.push_back(sig);

            return true;
        }
    }

    return true;
}

std::shared_ptr<identity::credential::Base::SerializedType> Primary::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = Key::serialize(lock, asPrivate, asSigned);

    std::unique_ptr<proto::MasterCredentialParameters> parameters(
        new proto::MasterCredentialParameters);

    OT_ASSERT(parameters);

    parameters->set_version(1);
    *(parameters->mutable_source()) = *(owner_backlink_->Source().Serialize());

    serializedCredential->set_allocated_masterdata(parameters.release());

    serializedCredential->set_role(proto::CREDROLE_MASTERKEY);
    *(serializedCredential->mutable_masterdata()->mutable_sourceproof()) =
        *source_proof_;

    return serializedCredential;
}

bool Primary::Verify(
    const proto::Credential& credential,
    const proto::CredentialRole& role,
    const Identifier& masterID,
    const proto::Signature& masterSig,
    const opentxs::PasswordPrompt& reason) const
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
            ": Credential does not designate this"
            " credential as its master.")
            .Flush();

        return false;
    }

    proto::Credential copy;
    copy.CopyFrom(credential);
    auto& signature = *copy.add_signature();
    signature.CopyFrom(masterSig);
    signature.clear_signature();

    return Verify(api_.Factory().Data(copy), masterSig, reason);
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
}  // namespace opentxs::identity::credential::implementation
