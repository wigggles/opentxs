// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "PushSocket.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PushSocket>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::PushSocket::"

namespace opentxs::network::zeromq
{
OTZMQPushSocket PushSocket::Factory(
    const class Context& context,
    const bool client)
{
    return OTZMQPushSocket(new implementation::PushSocket(context, client));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PushSocket::PushSocket(const zeromq::Context& context, const bool client)
    : ot_super(context, SocketType::Push)
    , CurveClient(lock_, socket_)
    , client_(client)
{
}

bool PushSocket::Push(const std::string& data) const
{
    return Push(Message::Factory(data));
}

bool PushSocket::Push(const opentxs::Data& data) const
{
    return Push(Message::Factory(data));
}

bool PushSocket::Push(zeromq::Message& data) const
{
    Lock lock(lock_);

    return send_message(lock, data);
}

PushSocket* PushSocket::clone() const
{
    return new PushSocket(context_, client_);
}

bool PushSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (client_) {

        return start_client(lock, endpoint);
    } else {

        return bind(lock, endpoint);
    }
}
}  // namespace opentxs::network::zeromq::implementation
