// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/core/contract/peer/PeerObject.cpp"

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/protobuf/PeerEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
class Contacts;
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blind
{
class Purse;
}  // namespace blind

class Armored;
class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::peer::implementation
{
class Object final : virtual public opentxs::PeerObject
{
public:
    const std::unique_ptr<std::string>& Message() const final
    {
        return message_;
    }
    const Nym_p& Nym() const final { return nym_; }
    const std::unique_ptr<std::string>& Payment() const final
    {
        return payment_;
    }
#if OT_CASH
    std::shared_ptr<blind::Purse> Purse() const final { return purse_; }
#endif
    const OTPeerRequest Request() const final { return request_; }
    const OTPeerReply Reply() const final { return reply_; }
    proto::PeerObject Serialize() const final;
    proto::PeerObjectType Type() const final { return type_; }
    bool Validate() const final;

    std::unique_ptr<std::string>& Message() final { return message_; }
    std::unique_ptr<std::string>& Payment() final { return payment_; }
#if OT_CASH
    std::shared_ptr<blind::Purse>& Purse() final { return purse_; }
#endif

    ~Object() final = default;

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
        const proto::PeerObject serialized);
    Object(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::string& message);
#if OT_CASH
    Object(
        const api::internal::Core& api,
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse);
#endif
    Object(
        const api::internal::Core& api,
        const std::string& payment,
        const Nym_p& senderNym);
    Object(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version);
    Object(
        const api::internal::Core& api,
        const OTPeerRequest request,
        const VersionNumber version);
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
        const VersionNumber version);
    Object() = delete;
};
}  // namespace opentxs::peer::implementation
