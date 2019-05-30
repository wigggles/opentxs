// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::otx::implementation
{
class Reply final : otx::Reply
{
public:
    proto::ServerReply Contract() const override;
    RequestNumber Number() const override;
    std::shared_ptr<proto::OTXPush> Push() const override;
    const identifier::Nym& Recipient() const override { return recipient_; }
    const identifier::Server& Server() const override { return server_; }
    bool Success() const override { return success_; }
    proto::ServerReplyType Type() const override { return type_; }

    bool SetNumber(const RequestNumber number, const PasswordPrompt& reason)
        override;
    bool SetPush(const proto::OTXPush& push, const PasswordPrompt& reason)
        override;

    ~Reply() = default;

private:
    friend otx::Reply;

    const OTNymID recipient_;
    const OTServerID server_;
    const proto::ServerReplyType type_{proto::SERVERREPLY_ERROR};
    const bool success_{false};
    RequestNumber number_{0};
    std::shared_ptr<proto::OTXPush> payload_{nullptr};

    static Nym_p extract_nym(
        const api::Core& api,
        const proto::ServerReply serialized,
        const PasswordPrompt& reason);

    Reply* clone() const override { return new Reply(*this); }
    OTIdentifier GetID(const Lock& lock) const override;
    proto::ServerReply full_version(const Lock& lock) const;
    proto::ServerReply id_version(const Lock& lock) const;
    std::string Name() const override { return {}; }
    OTData Serialize() const override;
    proto::ServerReply signature_version(const Lock& lock) const;
    bool update_signature(const Lock& lock, const PasswordPrompt& reason)
        override;
    bool validate(const Lock& lock, const PasswordPrompt& reason)
        const override;
    bool verify_signature(
        const Lock& lock,
        const proto::Signature& signature,
        const PasswordPrompt& reason) const override;

    Reply(
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Server& server,
        const proto::ServerReplyType type,
        const bool success);
    Reply(
        const api::Core& api,
        const proto::ServerReply serialized,
        const PasswordPrompt& reason);
    Reply() = delete;
    Reply(const Reply& rhs);
    Reply(Reply&& rhs) = delete;
    Reply& operator=(const Reply& rhs) = delete;
    Reply& operator=(Reply&& rhs) = delete;
};
}  // namespace opentxs::otx::implementation
