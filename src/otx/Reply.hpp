// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/otx/Reply.cpp"

#pragma once

#include <memory>
#include <string>

#include "core/contract/Signable.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/otx/Reply.hpp"

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
class Reply final : public otx::Reply,
                    public opentxs::contract::implementation::Signable
{
public:
    proto::ServerReply Contract() const final;
    RequestNumber Number() const final { return number_; }
    std::shared_ptr<const proto::OTXPush> Push() const final
    {
        return payload_;
    }
    const identifier::Nym& Recipient() const final { return recipient_; }
    const identifier::Server& Server() const final { return server_; }
    bool Success() const final { return success_; }
    proto::ServerReplyType Type() const final { return type_; }

    ~Reply() final = default;

private:
    friend otx::Reply;

    const OTNymID recipient_;
    const OTServerID server_;
    const proto::ServerReplyType type_;
    const bool success_;
    const RequestNumber number_;
    const std::shared_ptr<const proto::OTXPush> payload_;

    static Nym_p extract_nym(
        const api::internal::Core& api,
        const proto::ServerReply serialized);

    Reply* clone() const noexcept final { return new Reply(*this); }
    OTIdentifier GetID(const Lock& lock) const final;
    proto::ServerReply full_version(const Lock& lock) const;
    proto::ServerReply id_version(const Lock& lock) const;
    std::string Name() const final { return {}; }
    OTData Serialize() const final;
    proto::ServerReply signature_version(const Lock& lock) const;
    bool update_signature(const Lock& lock, const PasswordPrompt& reason) final;
    bool validate(const Lock& lock) const final;
    bool verify_signature(const Lock& lock, const proto::Signature& signature)
        const final;

    Reply(
        const api::internal::Core& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Server& server,
        const proto::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        std::shared_ptr<const proto::OTXPush>&& push);
    Reply(const api::internal::Core& api, const proto::ServerReply serialized);
    Reply() = delete;
    Reply(const Reply& rhs);
    Reply(Reply&& rhs) = delete;
    Reply& operator=(const Reply& rhs) = delete;
    Reply& operator=(Reply&& rhs) = delete;
};
}  // namespace opentxs::otx::implementation
