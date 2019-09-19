// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/Proto.tpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"
#include "Base.hpp"

#include <memory>
#include <ostream>

#include "Contact.hpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Contact::"

namespace opentxs
{
identity::credential::internal::Contact* Factory::ContactCredential(
    const api::Core& api,
    const opentxs::PasswordPrompt& reason,
    identity::internal::Authority& parent,
    const proto::Credential& serialized)
{
    return new identity::credential::implementation::Contact(
        api, parent, serialized);
}

identity::credential::internal::Contact* Factory::ContactCredential(
    const api::Core& api,
    identity::internal::Authority& parent,
    const NymParameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    return new identity::credential::implementation::Contact(
        api, parent, parameters, version);
}
}  // namespace opentxs

namespace opentxs::identity::credential
{
// static
std::string Contact::ClaimID(
    const api::Core& api,
    const std::string& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item)
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(section);
    preimage.set_type(item.type());
    preimage.set_start(item.start());
    preimage.set_end(item.end());
    preimage.set_value(item.value());
    preimage.set_subtype(item.subtype());

    return String::Factory(ClaimID(api, preimage))->Get();
}

// static
std::string Contact::ClaimID(
    const api::Core& api,
    const std::string& nymid,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::int64_t start,
    const std::int64_t end,
    const std::string& value,
    const std::string& subtype)
{
    proto::Claim preimage;
    preimage.set_version(1);
    preimage.set_nymid(nymid);
    preimage.set_section(section);
    preimage.set_type(type);
    preimage.set_start(start);
    preimage.set_end(end);
    preimage.set_value(value);
    preimage.set_subtype(subtype);

    return ClaimID(api, preimage)->str();
}

// static
OTIdentifier Contact::ClaimID(
    const api::Core& api,
    const proto::Claim& preimage)
{
    auto output = Identifier::Factory();
    output->CalculateDigest(api.Factory().Data(preimage));

    return output;
}

// static
Claim Contact::asClaim(
    const api::Core& api,
    const String& nymid,
    const std::uint32_t section,
    const proto::ContactItem& item)
{
    std::set<std::uint32_t> attributes;

    for (auto& attrib : item.attribute()) { attributes.insert(attrib); }

    return Claim{ClaimID(api, nymid.Get(), section, item),
                 section,
                 item.type(),
                 item.value(),
                 item.start(),
                 item.end(),
                 attributes};
}
}  // namespace opentxs::identity::credential

namespace opentxs::identity::credential::implementation
{
Contact::Contact(
    const api::Core& api,
    identity::internal::Authority& parent,
    const proto::Credential& serialized)
    : Signable({}, serialized.version())  // TODO Signable
    , credential::implementation::Base(api, parent, serialized)
{
    mode_ = proto::KEYMODE_NULL;
    master_id_ = serialized.childdata().masterid();
    data_.reset(new proto::ContactData(serialized.contactdata()));
}

Contact::Contact(
    const api::Core& api,
    identity::internal::Authority& parent,
    const NymParameters& nymParameters,
    const VersionNumber version)
    : Signable({}, version)  // TODO Signable
    , credential::implementation::Base(api, parent, nymParameters, version)
{
    mode_ = proto::KEYMODE_NULL;
    role_ = proto::CREDROLE_CONTACT;
    nym_id_ = parent.GetNymID();
    master_id_ = parent.GetMasterCredID();
    auto contacts = nymParameters.ContactData();

    if (contacts) { data_.reset(new proto::ContactData(*contacts)); }
}

bool Contact::GetContactData(
    std::unique_ptr<proto::ContactData>& contactData) const
{
    if (!data_) { return false; }

    contactData.reset(new proto::ContactData(*data_));

    return bool(contactData);
}

std::shared_ptr<Base::SerializedType> Contact::serialize(
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

    *(serializedCredential->mutable_contactdata()) = *data_;

    return serializedCredential;
}

}  // namespace opentxs::identity::credential::implementation
