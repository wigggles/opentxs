// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"
#include "Base.hpp"

#include <memory>
#include <ostream>
#include <string>

#include "Verification.hpp"

#define OT_METHOD "opentxs::identity::credential::Verification::"

namespace opentxs
{
identity::credential::internal::Verification* Factory::VerificationCredential(
    const api::Core& api,
    identity::internal::Authority& parent,
    const proto::Credential& serialized)
{
    return new identity::credential::implementation::Verification(
        api, parent, serialized);
}

identity::credential::internal::Verification* Factory::VerificationCredential(
    const api::Core& api,
    identity::internal::Authority& parent,
    const NymParameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    return new identity::credential::implementation::Verification(
        api, parent, parameters, version);
}
}  // namespace opentxs

namespace opentxs::identity::credential
{
// static
proto::Verification Verification::SigningForm(const proto::Verification& item)
{
    proto::Verification signingForm(item);
    signingForm.clear_sig();

    return signingForm;
}

// static
std::string Verification::VerificationID(
    const api::Core& api,
    const proto::Verification& item)
{
    auto id = Identifier::Factory();
    id->CalculateDigest(api.Factory().Data(item));

    return String::Factory(id)->Get();
}
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential::implementation
{
Verification::Verification(
    const api::Core& api,
    identity::internal::Authority& parent,
    const proto::Credential& serialized)
    : Signable({}, serialized.version())  // TODO Signable
    , credential::implementation::Base(api, parent, serialized)
    , data_()
{
    mode_ = proto::KEYMODE_NULL;
    master_id_ = serialized.childdata().masterid();
    data_.reset(new proto::VerificationSet(serialized.verification()));
}

Verification::Verification(
    const api::Core& api,
    identity::internal::Authority& parent,
    const NymParameters& nymParameters,
    const VersionNumber version)
    : Signable({}, version)  // TODO Signable
    , credential::implementation::Base(api, parent, nymParameters, version)
    , data_()
{
    mode_ = proto::KEYMODE_NULL;
    role_ = proto::CREDROLE_VERIFY;
    nym_id_ = parent.GetNymID();
    master_id_ = parent.GetMasterCredID();
    auto verificationSet = nymParameters.VerificationSet();

    if (verificationSet) {
        data_.reset(new proto::VerificationSet(*verificationSet));
    }
}

bool Verification::GetVerificationSet(
    std::unique_ptr<proto::VerificationSet>& verificationSet) const
{
    if (!data_) { return false; }

    verificationSet.reset(new proto::VerificationSet(*data_));

    return true;
}

std::shared_ptr<Base::SerializedType> Verification::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = Base::serialize(lock, asPrivate, asSigned);
    serializedCredential->set_mode(proto::KEYMODE_NULL);
    serializedCredential->clear_signature();  // this fixes a bug, but shouldn't

    if (asSigned) {
        SerializedSignature masterSignature = MasterSignature();

        if (masterSignature) {
            // We do not own this pointer.
            proto::Signature* serializedMasterSignature =
                serializedCredential->add_signature();
            *serializedMasterSignature = *masterSignature;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to get master signature.")
                .Flush();
        }
    }

    *(serializedCredential->mutable_verification()) = *data_;

    return serializedCredential;
}

bool Verification::verify_internally(
    const Lock& lock,
    const opentxs::PasswordPrompt& reason) const
{
    // Perform common Credential verifications
    if (!Base::verify_internally(lock, reason)) { return false; }

    if (data_) {
        for (auto& nym : data_->internal().identity()) {
            for (auto& claim : nym.verification()) {
                bool valid = owner_backlink_->Verify(claim, reason);

                if (!valid) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Invalid claim verification.")
                        .Flush();

                    return false;
                }
            }
        }
    }

    return true;
}
}  // namespace opentxs::identity::credential::implementation
