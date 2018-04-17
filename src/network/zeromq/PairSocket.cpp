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

#include "opentxs/stdafx.hpp"

#include "PairSocket.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::PairSocket::"

namespace opentxs::network::zeromq
{
OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback)
{
    return OTZMQPairSocket(new implementation::PairSocket(context, callback));
}

OTZMQPairSocket PairSocket::Factory(
    const ListenCallback& callback,
    const PairSocket& peer)
{
    return OTZMQPairSocket(new implementation::PairSocket(callback, peer));
}

OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback,
    const std::string& endpoint)
{
    return OTZMQPairSocket(
        new implementation::PairSocket(context, callback, endpoint));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint,
    const bool listener,
    const bool startThread)
    : ot_super(context, SocketType::Pair)
    , Receiver(lock_, socket_, startThread)
    , callback_(callback)
    , endpoint_(endpoint)
    , bind_(listener)
{
    OT_ASSERT(false == endpoint_.empty())

    bool init{false};
    Lock lock(lock_);

    if (bind_) {
        init = bind(lock, endpoint_);
        otInfo << OT_METHOD << __FUNCTION__ << ": Bound to " << endpoint_
               << std::endl;
    } else {
        init = start_client(lock, endpoint_);
        otInfo << OT_METHOD << __FUNCTION__ << ": Connected to " << endpoint_
               << std::endl;
    }

    OT_ASSERT(init)
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : PairSocket(
          context,
          callback,
          opentxs::network::zeromq::Socket::PairEndpointPrefix +
              Identifier::Random()->str(),
          true,
          startThread)
{
}

PairSocket::PairSocket(
    const zeromq::ListenCallback& callback,
    const zeromq::PairSocket& peer,
    const bool startThread)
    : PairSocket(peer.Context(), callback, peer.Endpoint(), false, startThread)
{
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint)
    : PairSocket(context, callback, endpoint, false, true)
{
}

PairSocket* PairSocket::clone() const
{
    return new PairSocket(context_, callback_, endpoint_, bind_, false);
}

const std::string& PairSocket::Endpoint() const { return endpoint_; }

bool PairSocket::have_callback() const { return true; }

void PairSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PairSocket::Send(const std::string& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(const opentxs::Data& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(zeromq::Message& data) const
{
    Lock lock(lock_);
    auto sent = zmq_msg_send(data, socket_, 0);

    if (-1 == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;
    }

    return (-1 != sent);
}

bool PairSocket::Start(const std::string&) const { return false; }

PairSocket::~PairSocket() {}
}  // namespace opentxs::network::zeromq::implementation
