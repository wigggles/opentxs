// Copyright (c) 2018 The Open-Transactions developers
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
    proto::ServerRequest Contract() const override;
    const identifier::Nym& Initiator() const override { return initiator_; }
    RequestNumber Number() const override;
    const identifier::Server& Server() const override { return server_; }
    proto::ServerRequestType Type() const override { return type_; }

    bool SetIncludeNym(const bool include) override;
    bool SetNumber(const RequestNumber number) override;

    ~Request() = default;

private:
    friend otx::Request;

    const OTNymID initiator_;
    const OTServerID server_;
    const proto::ServerRequestType type_{proto::SERVERREQUEST_ERROR};
    RequestNumber number_{0};
    OTFlag include_nym_;

    static std::shared_ptr<const opentxs::Nym> extract_nym(
        const api::Core& api,
        const proto::ServerRequest serialized);

    Request* clone() const override { return new Request(*this); }
    OTIdentifier GetID(const Lock& lock) const override;
    proto::ServerRequest full_version(const Lock& lock) const;
    proto::ServerRequest id_version(const Lock& lock) const;
    std::string Name() const override { return {}; }
    OTData Serialize() const override;
    proto::ServerRequest signature_version(const Lock& lock) const;
    bool update_signature(const Lock& lock) override;
    bool validate(const Lock& lock) const override;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const override;

    Request(
        const std::shared_ptr<const opentxs::Nym> signer,
        const identifier::Nym& initiator,
        const identifier::Server& server,
        const proto::ServerRequestType type);
    Request(const api::Core& api, const proto::ServerRequest serialized);
    Request() = delete;
    Request(const Request& rhs);
    Request(Request&& rhs) = delete;
    Request& operator=(const Request& rhs) = delete;
    Request& operator=(Request&& rhs) = delete;
};
}  // namespace opentxs::otx::implementation
