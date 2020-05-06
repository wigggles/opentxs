// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/otx/Request.cpp"

#pragma once

#include <string>

#include "core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/otx/Request.hpp"
#include "opentxs/protobuf/OTXEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::otx::implementation
{
class Request final : public otx::Request,
                      public opentxs::contract::implementation::Signable
{
public:
    proto::ServerRequest Contract() const final;
    const identifier::Nym& Initiator() const final { return initiator_; }
    RequestNumber Number() const final;
    const identifier::Server& Server() const final { return server_; }
    proto::ServerRequestType Type() const final { return type_; }

    bool SetIncludeNym(const bool include, const PasswordPrompt& reason) final;

    ~Request() final = default;

private:
    friend otx::Request;

    const OTNymID initiator_;
    const OTServerID server_;
    const proto::ServerRequestType type_;
    const RequestNumber number_;
    OTFlag include_nym_;

    static Nym_p extract_nym(
        const api::internal::Core& api,
        const proto::ServerRequest serialized);

    Request* clone() const noexcept final { return new Request(*this); }
    OTIdentifier GetID(const Lock& lock) const final;
    proto::ServerRequest full_version(const Lock& lock) const;
    proto::ServerRequest id_version(const Lock& lock) const;
    std::string Name() const final { return {}; }
    OTData Serialize() const final;
    proto::ServerRequest signature_version(const Lock& lock) const;
    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;
    bool validate(const Lock& lock) const final;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    Request(
        const api::internal::Core& api,
        const Nym_p signer,
        const identifier::Nym& initiator,
        const identifier::Server& server,
        const proto::ServerRequestType type,
        const RequestNumber number);
    Request(
        const api::internal::Core& api,
        const proto::ServerRequest serialized);
    Request() = delete;
    Request(const Request& rhs);
    Request(Request&& rhs) = delete;
    Request& operator=(const Request& rhs) = delete;
    Request& operator=(Request&& rhs) = delete;
};
}  // namespace opentxs::otx::implementation
