// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "Bidirectional.tpp"

#include <zmq.h>

#include "Router.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Router>;

#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Router::"

namespace opentxs
{
network::zeromq::socket::Router* Factory::RouterSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
{
    using ReturnType = network::zeromq::socket::implementation::Router;

    return new ReturnType(
        context,
        static_cast<network::zeromq::socket::Socket::Direction>(direction),
        callback);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Router::Router(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback) noexcept
    : Receiver(context, SocketType::Router, direction, false)
    , Bidirectional(context, true)
    , Client(this->get())
    , Server(this->get())
    , callback_(callback)
{
    init();
}

Router* Router::clone() const noexcept
{
    return new Router(context_, direction_, callback_);
}

void Router::process_incoming(const Lock& lock, Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    // Router prepends an identity frame to the message.  This makes sure
    // there is an empty frame between the identity frame(s) and the frames that
    // make up the rest of the message.
    message.EnsureDelimiter();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Done.").Flush();
}

Router::~Router() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
