// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "DealerSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>;

#define OT_METHOD "opentxs::network::zeromq::implementation::DealerSocket::"

namespace opentxs::network::zeromq
{
OTZMQDealerSocket DealerSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQDealerSocket(
        new implementation::DealerSocket(context, direction, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
DealerSocket::DealerSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : ot_super(context, SocketType::Dealer, direction)
    , CurveClient(lock_, socket_)
    , Bidirectional(context, lock_, running_, socket_, true)
    , callback_(callback)
{
}

DealerSocket* DealerSocket::clone() const
{
    return new DealerSocket(context_, direction_, callback_);
}

bool DealerSocket::have_callback() const { return true; }

void DealerSocket::process_incoming(
    const Lock& lock,
    opentxs::network::zeromq::Message& message)
{
    OT_ASSERT(verify_lock(lock))

    otWarn << OT_METHOD << __FUNCTION__
           << ": Incoming messaged received. Triggering callback." << std::endl;
    callback_.Process(message);
    otWarn << "Done." << std::endl;
}

bool DealerSocket::Send(opentxs::Data& input) const
{
    return Send(Message::Factory(input));
}

bool DealerSocket::Send(const std::string& input) const
{
    auto copy = input;

    return Send(Message::Factory(copy));
}

bool DealerSocket::Send(zeromq::Message& message) const
{
    return queue_message(message);
}

bool DealerSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool DealerSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (Socket::Direction::Connect == direction_) {

        return start_client(lock, endpoint);
    } else {

        return Socket::bind(lock, endpoint);
    }
}

DealerSocket::~DealerSocket() { shutdown(); }
}  // namespace opentxs::network::zeromq::implementation
