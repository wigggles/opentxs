// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/credential/Base.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/verify/Credential.hpp"

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

class NymParameters;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
template identity::credential::internal::Secondary* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Contact* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary& master,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Verification* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary& master,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Secondary* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);
template identity::credential::internal::Contact* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);
template identity::credential::internal::Verification* Factory::Credential(
    const api::internal::Core&,
    identity::internal::Authority&,
    const identity::Source&,
    const identity::credential::internal::Primary&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole);

template <typename C>
struct deserialize_credential {
    static C* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized);
};
template <typename C>
struct make_credential {
    static C* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const NymParameters& nymParameter,
        const opentxs::PasswordPrompt& reason);
};

template <>
struct deserialize_credential<identity::credential::internal::Contact> {
    static identity::credential::internal::Contact* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::ContactCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Secondary> {
    static identity::credential::internal::Secondary* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::SecondaryCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Verification> {
    static identity::credential::internal::Verification* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::VerificationCredential(
            api, parent, source, master, serialized);
    }
};
template <>
struct make_credential<identity::credential::internal::Contact> {
    static identity::credential::internal::Contact* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::ContactCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Secondary> {
    static identity::credential::internal::Secondary* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::SecondaryCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Verification> {
    static identity::credential::internal::Verification* Get(
        const api::internal::Core& api,
        identity::internal::Authority& parent,
        const identity::Source& source,
        const identity::credential::internal::Primary& master,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::VerificationCredential(
            api, parent, source, master, nymParameters, version, reason);
    }
};

template <class C>
C* Factory::Credential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const std::uint32_t version,
    const NymParameters& nymParameters,
    const proto::CredentialRole role,
    const opentxs::PasswordPrompt& reason)
{
    std::unique_ptr<C> output{make_credential<C>::Get(
        api, parent, source, master, version, nymParameters, reason)};

    if (!output) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ":Failed to construct credential.")
            .Flush();

        return nullptr;
    }

    if (output->Role() != role) { return nullptr; }

    return output.release();
}

template <class C>
C* Factory::Credential(
    const api::internal::Core& api,
    identity::internal::Authority& parent,
    const identity::Source& source,
    const identity::credential::internal::Primary& master,
    const proto::Credential& serialized,
    const proto::KeyMode mode,
    const proto::CredentialRole role)
{
    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Validate(serialized, VERBOSE, mode, role)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Invalid serialized credential.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<C> output{deserialize_credential<C>::Get(
        api, parent, source, master, serialized)};

    if (false == bool(output)) { return nullptr; }

    if (output->Role() != serialized.role()) { return nullptr; }

    if (false == output->Validate()) { return nullptr; }

    return output.release();
}
}  // namespace opentxs
