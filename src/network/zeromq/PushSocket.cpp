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

#include "PushSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::PushSocket::"

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
    auto sent = zmq_msg_send(data, socket_, 0);

    if (-1 == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Send error:\n"
              << zmq_strerror(zmq_errno()) << std::endl;
    }

    return (-1 != sent);
}

PushSocket* PushSocket::clone() const
{
    return new PushSocket(context_, client_);
}

bool PushSocket::Start(const std::string& endpoint) const
{
    Lock lock(lock_);

    if (client_) {

        return start_client(endpoint);
    } else {

        return bind(endpoint);
    }
}
}  // namespace opentxs::network::zeromq::implementation
