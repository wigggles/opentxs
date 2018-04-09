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

#include "ReplySocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::ReplySocket::"

namespace opentxs::network::zeromq
{
OTZMQReplySocket ReplySocket::Factory(
    const class Context& context,
    const ReplyCallback& callback)
{
    return OTZMQReplySocket(new implementation::ReplySocket(context, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ReplySocket::ReplySocket(
    const zeromq::Context& context,
    const ReplyCallback& callback)
    : ot_super(context, SocketType::Reply)
    , CurveServer(lock_, socket_)
    , Receiver(lock_, socket_, true)
    , callback_(callback)
{
}

ReplySocket* ReplySocket::clone() const
{
    return new ReplySocket(context_, callback_);
}

bool ReplySocket::have_callback() const { return true; }

void ReplySocket::process_incoming(const Lock&, Message& message)
{
    auto output = callback_.Process(message);
    Message& reply = output;
    auto sent = zmq_msg_send(reply, socket_, 0);

    if (-1 == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno())
              << "\nRequest: " << std::string(message) << "\nReply: " << reply
              << std::endl;
    }
}

bool ReplySocket::SetCurve(const OTPassword& key) const
{
    return set_curve(key);
}

bool ReplySocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    return bind(lock, endpoint);
}

ReplySocket::~ReplySocket() {}
}  // namespace opentxs::network::zeromq::implementation
