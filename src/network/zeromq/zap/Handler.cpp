// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/Forward.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"

#include "network/zeromq/CurveServer.hpp"
#include "network/zeromq/Receiver.hpp"
#include "network/zeromq/Socket.hpp"

#include "Handler.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Handler>;

#define ZAP_ENDPOINT "inproc://zeromq.zap.01"

#define OT_METHOD "opentxs::network::zeromq::zap::implementation::Handler::"

namespace opentxs::network::zeromq::zap
{
OTZMQZAPHandler Handler::Factory(
    const zeromq::Context& context,
    const zap::Callback& callback)
{
    return OTZMQZAPHandler(new implementation::Handler(context, callback));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
Handler::Handler(const zeromq::Context& context, const zap::Callback& callback)
    : ot_super(context, SocketType::Router, Socket::Direction::Bind)
    , CurveServer(lock_, socket_)
    , Receiver(lock_, socket_, true)
    , callback_(callback)
{
    Lock lock(lock_);
    const auto running = bind(lock, ZAP_ENDPOINT);

    OT_ASSERT(running);

    otWarn << OT_METHOD << __FUNCTION__ << ": Listening on " << ZAP_ENDPOINT
           << std::endl;
}

void Handler::process_incoming(const Lock& lock, zap::Request& message)
{
    auto output = callback_.Process(message);
    Message& reply = output;
    send_message(lock, reply);
}
}  // namespace opentxs::network::zeromq::zap::implementation
