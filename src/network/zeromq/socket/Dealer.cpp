// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "Bidirectional.tpp"

#include <zmq.h>

#include "Dealer.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>;

#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Dealer::"

namespace opentxs
{
network::zeromq::socket::Dealer* Factory::DealerSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
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

Dealer* Dealer::clone() const noexcept
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
