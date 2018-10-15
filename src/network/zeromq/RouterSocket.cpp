// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "RouterSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>;

#define OT_METHOD "opentxs::network::zeromq::implementation::RouterSocket::"

namespace opentxs::network::zeromq
{
OTZMQRouterSocket RouterSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQRouterSocket(
        new implementation::RouterSocket(context, direction, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
RouterSocket::RouterSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : ot_super(context, SocketType::Router, direction)
    , CurveClient(lock_, socket_)
    , CurveServer(lock_, socket_)
    , Bidirectional(context, lock_, running_, socket_, true)
    , callback_(callback)
{
}

RouterSocket* RouterSocket::clone() const
{
    return new RouterSocket(context_, direction_, callback_);
}

bool RouterSocket::have_callback() const { return true; }

void RouterSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();

    // RouterSocket prepends an identity frame to the message.  This makes sure
    // there is an empty frame between the identity frame(s) and the frames that
    // make up the rest of the message.
    message.EnsureDelimiter();

    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)("Done.").Flush();
}

bool RouterSocket::Send(opentxs::Data& input) const
{
    return Send(Message::Factory(input));
}

bool RouterSocket::Send(const std::string& input) const
{
    auto copy = input;

    return Send(Message::Factory(copy));
}

bool RouterSocket::Send(zeromq::Message& message) const
{
    return queue_message(message);
}

bool RouterSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool RouterSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (Socket::Direction::Connect == direction_) {

        return start_client(lock, endpoint);
    } else {

        return Socket::bind(lock, endpoint);
    }
}

RouterSocket::~RouterSocket() { shutdown(); }
}  // namespace opentxs::network::zeromq::implementation
