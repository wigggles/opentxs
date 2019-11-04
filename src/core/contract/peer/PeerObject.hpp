// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace opentxs::peer::implementation
{
class Object : virtual public opentxs::PeerObject
{
public:
    const std::unique_ptr<std::string>& Message() const override
    {
        return message_;
    }
    const Nym_p& Nym() const override { return nym_; }
    const std::unique_ptr<std::string>& Payment() const override
    {
        return payment_;
    }
#if OT_CASH
    std::shared_ptr<blind::Purse> Purse() const override { return purse_; }
#endif
    const OTPeerRequest Request() const override { return request_; }
    const OTPeerReply Reply() const override { return reply_; }
    proto::PeerObject Serialize(const PasswordPrompt& reason) const override;
    proto::PeerObjectType Type() const override { return type_; }
    bool Validate(const PasswordPrompt& reason) const override;

    std::unique_ptr<std::string>& Message() override { return message_; }
    std::unique_ptr<std::string>& Payment() override { return payment_; }
#if OT_CASH
    std::shared_ptr<blind::Purse>& Purse() override { return purse_; }
#endif

    ~Object() override = default;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    Nym_p nym_{nullptr};
    std::unique_ptr<std::string> message_{nullptr};
    std::unique_ptr<std::string> payment_{nullptr};
    OTPeerReply reply_{nullptr};
    OTPeerRequest request_{nullptr};
#if OT_CASH
    std::shared_ptr<blind::Purse> purse_{nullptr};
#endif
    proto::PeerObjectType type_{proto::PEEROBJECT_ERROR};
    VersionNumber version_{0};

    Object(
        const api::client::Contacts& contacts,
        const api::internal::Core& api,
        const Nym_p& signerNym,
        const proto::PeerObject serialized,
        const PasswordPrompt& reason);
    Object(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& message,
        const PasswordPrompt& reason);
#if OT_CASH
    Object(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse,
        const PasswordPrompt& reason);
#endif
    Object(
        const api::internal::Core& api,
        const std::string& payment,
        const Nym_p& senderNym,
        const PasswordPrompt& reason);
    Object(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version,
        const PasswordPrompt& reason);
    Object(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const VersionNumber version,
        const PasswordPrompt& reason);
    Object(
        const api::internal::Core& api,
        const Nym_p& nym,
        const std::string& message,
        const std::string& payment,
        const OTPeerReply reply,
        const OTPeerRequest request,
#if OT_CASH
        const std::shared_ptr<blind::Purse> purse,
#endif
        const proto::PeerObjectType type,
        const VersionNumber version,
        const PasswordPrompt& reason);
    Object() = delete;
};
}  // namespace opentxs::peer::implementation
