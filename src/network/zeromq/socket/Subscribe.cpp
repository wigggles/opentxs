// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

#include "Subscribe.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>;

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::Subscribe::"

namespace opentxs
{
network::zeromq::socket::Subscribe* Factory::SubscribeSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback)
{
    using ReturnType = network::zeromq::socket::implementation::Subscribe;

    return new ReturnType(context, callback);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Subscribe::Subscribe(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback) noexcept
    : Receiver(context, SocketType::Subscribe, Socket::Direction::Connect, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

Subscribe* Subscribe::clone() const noexcept
{
    return new Subscribe(context_, callback_);
}

bool Subscribe::have_callback() const noexcept { return true; }

void Subscribe::init() noexcept
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);
    Receiver::init();

    OT_ASSERT(0 == set);
}

void Subscribe::process_incoming(const Lock& lock, Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(" Done.").Flush();
}

bool Subscribe::SetSocksProxy(const std::string& proxy) const noexcept
{
    return set_socks_proxy(proxy);
}

Subscribe::~Subscribe() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
