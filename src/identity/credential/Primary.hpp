// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::identity::credential::implementation
{
class Primary final : virtual public credential::internal::Primary,
                      credential::implementation::Key
{
public:
    bool hasCapability(const NymCapability& capability) const final;
    bool Path(proto::HDPath& output) const final;
    std::string Path() const final;
    using Base::Verify;
    bool Verify(
        const proto::Credential& credential,
        const proto::CredentialRole& role,
        const Identifier& masterID,
        const proto::Signature& masterSig,
        const PasswordPrompt& reason) const final;

    ~Primary() final = default;

private:
    friend opentxs::Factory;

    static const VersionConversionMap credential_to_master_params_;

    const proto::SourceProof source_proof_;

    static proto::SourceProof source_proof(const NymParameters& params);

    std::shared_ptr<identity::credential::Base::SerializedType> serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const final;
    bool verify_against_source(const Lock& lock, const PasswordPrompt& reason)
        const;
    bool verify_internally(const Lock& lock, const PasswordPrompt& reason)
        const final;

    void sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false) final;

    Primary(
        const api::Core& api,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const NymParameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
    Primary(
        const api::Core& api,
        const PasswordPrompt& reason,
        const identity::internal::Authority& parent,
        const identity::Source& source,
        const proto::Credential& serializedCred) noexcept(false);
    Primary() = delete;
    Primary(const Primary&) = delete;
    Primary(Primary&&) = delete;
    Primary& operator=(const Primary&) = delete;
    Primary& operator=(Primary&&) = delete;
};
}  // namespace opentxs::identity::credential::implementation
