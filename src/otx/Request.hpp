// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::otx::implementation
{
class Request final : otx::Request
{
public:
    proto::ServerRequest Contract() const final;
    const identifier::Nym& Initiator() const final { return initiator_; }
    RequestNumber Number() const final;
    const identifier::Server& Server() const final { return server_; }
    proto::ServerRequestType Type() const final { return type_; }

    bool SetIncludeNym(const bool include, const PasswordPrompt& reason) final;
    bool SetNumber(const RequestNumber number, const PasswordPrompt& reason)
        final;

    ~Request() final = default;

private:
    friend otx::Request;

    const api::internal::Core& api_;
    const OTNymID initiator_;
    const OTServerID server_;
    const proto::ServerRequestType type_{proto::SERVERREQUEST_ERROR};
    RequestNumber number_{0};
    OTFlag include_nym_;

    static Nym_p extract_nym(
        const api::internal::Core& api,
        const proto::ServerRequest serialized,
        const PasswordPrompt& reason);

    Request* clone() const final { return new Request(*this); }
    OTIdentifier GetID(const Lock& lock) const final;
    proto::ServerRequest full_version(const Lock& lock) const;
    proto::ServerRequest id_version(const Lock& lock) const;
    std::string Name() const final { return {}; }
    OTData Serialize() const final;
    proto::ServerRequest signature_version(const Lock& lock) const;
    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;
    bool validate(const Lock& lock, const PasswordPrompt& reason) const final;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const final;

    Request(
        const api::internal::Core& api,
        const Nym_p signer,
        const identifier::Nym& initiator,
        const identifier::Server& server,
        const proto::ServerRequestType type);
    Request(
        const api::internal::Core& api,
        const proto::ServerRequest serialized,
        const PasswordPrompt& reason);
    Request() = delete;
    Request(const Request& rhs);
    Request(Request&& rhs) = delete;
    Request& operator=(const Request& rhs) = delete;
    Request& operator=(Request&& rhs) = delete;
};
}  // namespace opentxs::otx::implementation
