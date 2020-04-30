// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "api/client/Factory.hpp"  // IWYU pragma: associated

#include "api/Factory.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"

//#define OT_METHOD "opentxs::api::client::implementation::Factory::"

namespace opentxs
{
api::internal::Factory* Factory::FactoryAPIClient(
    const api::client::internal::Manager& api)
{
    return new api::client::implementation::Factory(api);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Factory::Factory(const api::client::internal::Manager& client)
    : api::implementation::Factory(client)
    , client_(client)
{
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const Nym_p& senderNym,
    const std::string& message) const
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, message)};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const Nym_p& senderNym,
    const std::string& payment,
    const bool isPayment) const
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, payment, isPayment)};
}

#if OT_CASH
std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const Nym_p& senderNym,
    const std::shared_ptr<blind::Purse> purse) const
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, senderNym, purse)};
}
#endif

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const OTPeerRequest request,
    const OTPeerReply reply,
    const VersionNumber version) const
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, request, reply, version)};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const OTPeerRequest request,
    const VersionNumber version) const
{
    return std::unique_ptr<opentxs::PeerObject>{
        opentxs::Factory::PeerObject(client_, request, version)};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const Nym_p& signerNym,
    const proto::PeerObject& serialized) const
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::Factory::PeerObject(
        client_.Contacts(), client_, signerNym, serialized)};
}

std::unique_ptr<opentxs::PeerObject> Factory::PeerObject(
    const Nym_p& recipientNym,
    const opentxs::Armored& encrypted,
    const opentxs::PasswordPrompt& reason) const
{
    return std::unique_ptr<opentxs::PeerObject>{opentxs::Factory::PeerObject(
        client_.Contacts(), client_, recipientNym, encrypted, reason)};
}
}  // namespace opentxs::api::client::implementation
