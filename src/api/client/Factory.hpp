// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/Factory.cpp"

#pragma once

#include <memory>
#include <string>

#include "api/Factory.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace blind
{
class Purse;
}  // namespace blind

namespace proto
{
class PeerObject;
}  // namespace proto

class Armored;
class Factory;
class PasswordPrompt;
class PeerObject;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class Factory final : public opentxs::api::implementation::Factory
{
public:
    auto PeerObject(const Nym_p& senderNym, const std::string& message) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& senderNym,
        const std::string& payment,
        const bool isPayment) const
        -> std::unique_ptr<opentxs::PeerObject> final;
#if OT_CASH
    auto PeerObject(
        const Nym_p& senderNym,
        const std::shared_ptr<blind::Purse> purse) const
        -> std::unique_ptr<opentxs::PeerObject> final;
#endif
    auto PeerObject(
        const OTPeerRequest request,
        const OTPeerReply reply,
        const VersionNumber version) const
        -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const OTPeerRequest request, const VersionNumber version)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(const Nym_p& signerNym, const proto::PeerObject& serialized)
        const -> std::unique_ptr<opentxs::PeerObject> final;
    auto PeerObject(
        const Nym_p& recipientNym,
        const opentxs::Armored& encrypted,
        const opentxs::PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::PeerObject> final;

    ~Factory() final = default;

private:
    friend opentxs::Factory;

    const api::client::internal::Manager& client_;

    Factory(const api::client::internal::Manager& client);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory &&) -> Factory& = delete;
};
}  // namespace opentxs::api::client::implementation
