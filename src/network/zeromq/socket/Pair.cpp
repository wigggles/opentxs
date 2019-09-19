// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "Bidirectional.tpp"

#include <zmq.h>

#include "Pair.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>;

// #define OT_METHOD
// "opentxs::network::zeromq::socket::implementation::Pair::"

namespace opentxs
{
network::zeromq::socket::Pair* Factory::PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const bool startThread)
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return new ReturnType(context, callback, startThread);
}

network::zeromq::socket::Pair* Factory::PairSocket(
    const network::zeromq::ListenCallback& callback,
    const network::zeromq::socket::Pair& peer,
    const bool startThread)
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return new ReturnType(callback, peer, startThread);
}

network::zeromq::socket::Pair* Factory::PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string& endpoint)
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return new ReturnType(context, callback, endpoint);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint,
    const Socket::Direction direction,
    const bool startThread) noexcept
    : Receiver(context, SocketType::Pair, direction, startThread)
    , Bidirectional(context, true)
    , callback_(callback)
    , endpoint_(endpoint)
{
    init();
}

Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread) noexcept
    : Pair(
          context,
          callback,
          Socket::random_inproc_endpoint(),
          Socket::Direction::Bind,
          startThread)
{
}

Pair::Pair(
    const zeromq::ListenCallback& callback,
    const zeromq::socket::Pair& peer,
    const bool startThread) noexcept
    : Pair(
          peer.Context(),
          callback,
          peer.Endpoint(),
          Socket::Direction::Connect,
          startThread)
{
}

Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint) noexcept
    : Pair(context, callback, endpoint, Socket::Direction::Connect, true)
{
}

Pair* Pair::clone() const noexcept
{
    return new Pair(context_, callback_, endpoint_, direction_, false);
}

const std::string& Pair::Endpoint() const noexcept { return endpoint_; }

bool Pair::have_callback() const noexcept { return true; }

void Pair::init() noexcept
{
    Bidirectional::init();

    OT_ASSERT(false == endpoint_.empty())

    const auto init = Bidirectional::Start(endpoint_);

    OT_ASSERT(init)
}

void Pair::process_incoming(const Lock& lock, Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

Pair::~Pair() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
