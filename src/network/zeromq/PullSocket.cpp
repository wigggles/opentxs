// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PullSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

#include <chrono>

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::PullSocket>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::PullSocket::"

namespace opentxs::network::zeromq
{
OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQPullSocket(
        new implementation::PullSocket(context, direction, callback));
}

OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const Socket::Direction direction)
{
    return OTZMQPullSocket(new implementation::PullSocket(context, direction));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : ot_super(context, SocketType::Pull, direction)
    , CurveServer(lock_, socket_)
    , Receiver(lock_, running_, socket_, startThread)
    , callback_(callback)
{
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : PullSocket(context, direction, callback, true)
{
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction)
    : PullSocket(context, direction, ListenCallback::Factory(), false)
{
}

PullSocket* PullSocket::clone() const
{
    return new PullSocket(context_, direction_, callback_);
}

bool PullSocket::have_callback() const { return true; }

void PullSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PullSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (Socket::Direction::Connect == direction_) {

        return start_client(lock, endpoint);
    } else {

        return bind(lock, endpoint);
    }
}

PullSocket::~PullSocket() { shutdown(); }
}  // namespace opentxs::network::zeromq::implementation
