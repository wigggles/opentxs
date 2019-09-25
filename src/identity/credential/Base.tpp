// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Base.hpp"

namespace opentxs
{
template identity::credential::internal::Secondary* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Contact* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Primary* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Verification* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const std::uint32_t,
    const NymParameters&,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);

template identity::credential::internal::Secondary* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Contact* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Primary* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);
template identity::credential::internal::Verification* Factory::Credential(
    const api::Core&,
    identity::internal::Authority&,
    const proto::Credential&,
    const proto::KeyMode,
    const proto::CredentialRole,
    const opentxs::PasswordPrompt&);

template <typename C>
struct deserialize_credential {
    static C* Get(
        const api::Core& api,
        identity::internal::Authority& parent,
        const proto::Credential& serialized);
};
template <typename C>
struct make_credential {
    static C* Get(
        const api::Core& api,
        identity::internal::Authority& owner,
        const std::uint32_t version,
        const NymParameters& nymParameter,
        const opentxs::PasswordPrompt& reason);
};

template <>
struct deserialize_credential<identity::credential::internal::Contact> {
    static identity::credential::internal::Contact* Get(
        const api::Core& api,
        const opentxs::PasswordPrompt& reason,
        identity::internal::Authority& parent,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::ContactCredential(
            api, reason, parent, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Primary> {
    static identity::credential::internal::Primary* Get(
        const api::Core& api,
        const opentxs::PasswordPrompt& reason,
        identity::internal::Authority& parent,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::PrimaryCredential(
            api, reason, parent, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Secondary> {
    static identity::credential::internal::Secondary* Get(
        const api::Core& api,
        const opentxs::PasswordPrompt& reason,
        identity::internal::Authority& parent,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::SecondaryCredential(
            api, reason, parent, serialized);
    }
};
template <>
struct deserialize_credential<identity::credential::internal::Verification> {
    static identity::credential::internal::Verification* Get(
        const api::Core& api,
        const opentxs::PasswordPrompt& reason,
        identity::internal::Authority& parent,
        const proto::Credential& serialized)
    {
        return opentxs::Factory::VerificationCredential(
            api, parent, serialized);
    }
};
template <>
struct make_credential<identity::credential::internal::Contact> {
    static identity::credential::internal::Contact* Get(
        const api::Core& api,
        identity::internal::Authority& owner,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::ContactCredential(
            api, owner, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Primary> {
    static identity::credential::internal::Primary* Get(
        const api::Core& api,
        identity::internal::Authority& owner,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::PrimaryCredential(
            api, owner, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Secondary> {
    static identity::credential::internal::Secondary* Get(
        const api::Core& api,
        identity::internal::Authority& owner,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::SecondaryCredential(
            api, owner, nymParameters, version, reason);
    }
};
template <>
struct make_credential<identity::credential::internal::Verification> {
    static identity::credential::internal::Verification* Get(
        const api::Core& api,
        identity::internal::Authority& owner,
        const std::uint32_t version,
        const NymParameters& nymParameters,
        const opentxs::PasswordPrompt& reason)
    {
        return opentxs::Factory::VerificationCredential(
            api, owner, nymParameters, version, reason);
    }
};

template <class C>
C* Factory::Credential(
    const api::Core& api,
    identity::internal::Authority& owner,
    const std::uint32_t version,
    const NymParameters& nymParameters,
    const proto::CredentialRole role,
    const opentxs::PasswordPrompt& reason)
{
    std::unique_ptr<C> output{
        make_credential<C>::Get(api, owner, version, nymParameters, reason)};

    if (!output) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ":Failed to construct credential.")
            .Flush();

        return nullptr;
    }

    if (output->Role() != role) { return nullptr; }

    if (!output->New(nymParameters, reason)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ":Failed to sign credential.")
            .Flush();

        return nullptr;
    }

    if (!output->Save()) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ":Failed to save credential.")
            .Flush();

        return nullptr;
    }

    return output.release();
}

template <class C>
C* Factory::Credential(
    const api::Core& api,
    identity::internal::Authority& parent,
    const proto::Credential& serialized,
    const proto::KeyMode mode,
    const proto::CredentialRole role,
    const opentxs::PasswordPrompt& reason)
{
    // This check allows all constructors to assume inputs are well-formed
    if (!proto::Validate(serialized, VERBOSE, mode, role)) {
        LogOutput("opentxs::Factory::")(__FUNCTION__)(
            ": Invalid serialized credential.")
            .Flush();

        return nullptr;
    }

    std::unique_ptr<C> output{
        deserialize_credential<C>::Get(api, reason, parent, serialized)};

    if (false == bool(output)) { return nullptr; }

    if (output->Role() != serialized.role()) { return nullptr; }

    if (false == output->Validate(reason)) { return nullptr; }

    return output.release();
}
}  // namespace opentxs
