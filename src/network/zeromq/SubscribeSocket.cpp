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

#include "SubscribeSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::SubscribeSocket::"

namespace opentxs::network::zeromq
{
OTZMQSubscribeSocket SubscribeSocket::Factory(
    const class Context& context,
    const ListenCallback& callback)
{
    return OTZMQSubscribeSocket(
        new implementation::SubscribeSocket(context, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
SubscribeSocket::SubscribeSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback)
    : ot_super(context, SocketType::Subscribe)
    , CurveClient(lock_, socket_)
    , Receiver(lock_, socket_, true)
    , callback_(callback)
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);

    OT_ASSERT(0 == set);
}

SubscribeSocket* SubscribeSocket::clone() const
{
    return new SubscribeSocket(context_, callback_);
}

bool SubscribeSocket::have_callback() const { return true; }

void SubscribeSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    otInfo << OT_METHOD << __FUNCTION__
           << ": Incoming messaged received. Triggering callback." << std::endl;
    callback_.Process(message);
}

bool SubscribeSocket::SetCurve(const ServerContract& contract) const
{
    return set_curve(contract);
}

bool SubscribeSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool SubscribeSocket::Start(const std::string& endpoint) const
{
    return start_client(endpoint);
}

SubscribeSocket::~SubscribeSocket() {}
}  // namespace opentxs::network::zeromq::implementation
