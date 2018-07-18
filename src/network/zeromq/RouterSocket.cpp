/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

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
    const bool client,
    const ListenCallback& callback)
{
    return OTZMQRouterSocket(
        new implementation::RouterSocket(context, client, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
RouterSocket::RouterSocket(
    const zeromq::Context& context,
    const bool client,
    const zeromq::ListenCallback& callback)
    : ot_super(context, SocketType::Router)
    , CurveClient(lock_, socket_)
    , Receiver(lock_, socket_, true)
    , callback_(callback)
    , client_(client)
{
}

RouterSocket* RouterSocket::clone() const
{
    return new RouterSocket(context_, client_, callback_);
}

bool RouterSocket::have_callback() const { return true; }

void RouterSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    otWarn << OT_METHOD << __FUNCTION__
           << ": Incoming messaged received. Triggering callback." << std::endl;

    // RouterSocket prepends an identity frame to the message.  This makes sure
    // there is an empty frame between the identity frame(s) and the frames that
    // make up the rest of the message.
    message.EnsureDelimiter();

    callback_.Process(message);
    otWarn << "Done." << std::endl;
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
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    bool sent{true};
    const auto parts = message.size();
    std::size_t counter{0};

    for (auto& frame : message) {
        int flags{0};

        if (++counter < parts) { flags = ZMQ_SNDMORE; }

        sent |= (-1 != zmq_msg_send(frame, socket_, flags));
    }

    if (false == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;
    }

    return (false != sent);
}

bool RouterSocket::SetCurve(const ServerContract& contract) const
{
    return set_curve(contract);
}

bool RouterSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool RouterSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (client_) {

        return start_client(lock, endpoint);
    } else {

        return bind(lock, endpoint);
    }
}

RouterSocket::~RouterSocket() {}
}  // namespace opentxs::network::zeromq::implementation
