// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PublishSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::PublishSocket::"

namespace opentxs::network::zeromq
{
OTZMQPublishSocket PublishSocket::Factory(const class Context& context)
{
    return OTZMQPublishSocket(new implementation::PublishSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PublishSocket::PublishSocket(const zeromq::Context& context)
    : ot_super(context, SocketType::Publish, Socket::Direction::Bind)
    , CurveServer(lock_, socket_)
{
}

bool PublishSocket::Publish(const std::string& data) const
{
    return Publish(Message::Factory(data));
}

bool PublishSocket::Publish(const opentxs::Data& data) const
{
    return Publish(Message::Factory(data));
}

bool PublishSocket::Publish(zeromq::Message& data) const
{
    Lock lock(lock_);

    return send_message(lock, data);
}

PublishSocket* PublishSocket::clone() const
{
    return new PublishSocket(context_);
}

bool PublishSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    return bind(lock, endpoint);
}
}  // namespace opentxs::network::zeromq::implementation
