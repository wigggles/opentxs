// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/Authority.hpp"
#include "opentxs/identity/Source.hpp"
#include "opentxs/Proto.hpp"

#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/Identity.hpp"
#include "identity/credential/Key.hpp"

#include <memory>
#include <ostream>

#include "Secondary.hpp"

#define OT_METHOD "opentxs::identity::credential::implementation::Secondary::"

namespace opentxs
{
using ReturnType = identity::credential::implementation::Secondary;

identity::credential::internal::Secondary* Factory::SecondaryCredential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const NymParameters& parameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason)
{
    try {

        return new ReturnType(
            api, parent, source, master, parameters, version, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Failed to create credential: ")(e.what())
            .Flush();

        return nullptr;
    }
}

identity::credential::internal::Secondary* Factory::SecondaryCredential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized,
    const opentxs::PasswordPrompt& reason)
{
    try {

        return new ReturnType(api, reason, parent, source, master, serialized);
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
Secondary::Secondary(
    const api::internal::Core& api,
    const identity::internal::Authority& owner,
    const identity::Source& source,
    const internal::Primary& master,
    const NymParameters& nymParameters,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept(false)
    : Signable({}, version)  // TODO Signable
    , credential::implementation::Key(
          api,
          owner,
          source,
          nymParameters,
          version,
          proto::CREDROLE_CHILDKEY,
          reason,
          get_master_id(master))
{
    init(master, reason);
}

Secondary::Secondary(
    const api::internal::Core& api,
    const opentxs::PasswordPrompt& reason,
    const identity::internal::Authority& owner,
    const identity::Source& source,
    const internal::Primary& master,
    const proto::Credential& serialized) noexcept(false)
    : Signable({}, serialized.version())  // TODO Signable
    , credential::implementation::Key(
          api,
          reason,
          owner,
          source,
          serialized,
          get_master_id(serialized, master))
{
}

std::shared_ptr<Base::SerializedType> Secondary::serialize(
    const Lock& lock,
    const SerializationModeFlag asPrivate,
    const SerializationSignatureFlag asSigned) const
{
    auto serializedCredential = Key::serialize(lock, asPrivate, asSigned);

    if (asSigned) {
        auto masterSignature = MasterSignature();

        if (masterSignature) {
            *serializedCredential->add_signature() = *masterSignature;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to get master signature.")
                .Flush();
        }
    }

    serializedCredential->set_role(proto::CREDROLE_CHILDKEY);

    return serializedCredential;
}
}  // namespace opentxs::identity::credential::implementation
