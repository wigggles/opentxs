// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"

#include "network/zeromq/curve/Server.hpp"
#include "Receiver.tpp"
#include "Socket.hpp"

#include "Reply.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>;
template class opentxs::network::zeromq::socket::implementation::Receiver<
    opentxs::network::zeromq::Message>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::Reply::"

namespace opentxs
{
network::zeromq::socket::Reply* Factory::ReplySocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ReplyCallback& callback)
{
    using ReturnType = network::zeromq::socket::implementation::Reply;

    return new ReturnType(
        context,
        static_cast<network::zeromq::socket::Socket::Direction>(direction),
        callback);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Reply::Reply(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const ReplyCallback& callback) noexcept
    : Receiver(context, SocketType::Reply, direction, true)
    , Server(this->get())
    , callback_(callback)
{
    init();
}

Reply* Reply::clone() const noexcept
{
    return new Reply(context_, direction_, callback_);
}

bool Reply::have_callback() const noexcept { return true; }

void Reply::process_incoming(const Lock& lock, Message& message) noexcept
{
    auto output = callback_.Process(message);
    Message& reply = output;
    send_message(lock, reply);
}

Reply::~Reply() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
