// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "network/zeromq/socket/Dealer.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Bidirectional.tpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.tpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Dealer;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>;

#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Dealer::"

namespace opentxs
{
auto Factory::DealerSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
    -> network::zeromq::socket::Dealer*
{
    using ReturnType = network::zeromq::socket::implementation::Dealer;

    return new ReturnType(
        context,
        static_cast<network::zeromq::socket::Socket::Direction>(direction),
        callback);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Dealer::Dealer(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback) noexcept
    : Receiver(context, SocketType::Dealer, direction, false)
    , Bidirectional(context, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

auto Dealer::clone() const noexcept -> Dealer*
{
    return new Dealer(context_, direction_, callback_);
}

void Dealer::process_incoming(
    const Lock& lock,
    opentxs::network::zeromq::Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Done.").Flush();
}

Dealer::~Dealer() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
