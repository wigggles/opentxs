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

#include "opentxs/network/zeromq/implementation/ReplySocket.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::ReplySocket::"

namespace opentxs::network::zeromq::implementation
{
ReplySocket::ReplySocket(const zeromq::Context& context)
    : ot_super(context, SocketType::Reply)
{
}

Socket::MessageReceiveResult ReplySocket::ReceiveRequest(BlockMode block)
{
    Lock lock(lock_);
    MessageReceiveResult output{false, nullptr};
    auto& status = output.first;
    auto& request = output.second;
    request = context_.NewMessage();

    OT_ASSERT(request);

    const int flag = (block) ? 0 : ZMQ_DONTWAIT;
    status = (-1 != zmq_msg_recv(*request, socket_, flag));

    return output;
}

bool ReplySocket::SendReply(const std::string& reply)
{
    auto message = context_.NewMessage(reply);

    OT_ASSERT(message);

    return SendReply(*message);
}

bool ReplySocket::SendReply(const opentxs::Data& reply)
{
    auto message = context_.NewMessage(reply);

    OT_ASSERT(message);

    return SendReply(*message);
}

bool ReplySocket::SendReply(zeromq::Message& reply)
{
    Lock lock(lock_);

    return (-1 != zmq_msg_send(reply, socket_, 0));
}

bool ReplySocket::SetCurve(const OTPassword& key)
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    if (CURVE_KEY_BYTES != key.getMemorySize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid private key."
              << std::endl;

        return false;
    }

    const int server{1};
    auto set =
        zmq_setsockopt(socket_, ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_CURVE_SERVER"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket_, ZMQ_CURVE_SECRETKEY, key.getMemory(), key.getMemorySize());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

bool ReplySocket::Start(const std::string& endpoint)
{
    Lock lock(lock_);

    return (0 == zmq_bind(socket_, endpoint.c_str()));
}
}  // namespace opentxs::network::zeromq::implementation
