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
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::SubscribeSocket::"

namespace opentxs::network::zeromq
{
OTZMQSubscribeSocket SubscribeSocket::Factory(const Context& context)
{
    return OTZMQSubscribeSocket(new implementation::SubscribeSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
SubscribeSocket::SubscribeSocket(const zeromq::Context& context)
    : ot_super(context, SocketType::Subscribe)
    , CurveClient(lock_, socket_)
    , Receiver(lock_, socket_)
    , callback_(nullptr)
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);

    OT_ASSERT(0 == set);
}

bool SubscribeSocket::have_callback() const
{
    Lock lock(lock_);

    return bool(callback_);
}

void SubscribeSocket::process_incoming(const Lock&, Message& message)
{
    callback_(message);
}

void SubscribeSocket::RegisterCallback(ReceiveCallback callback) const
{
    Lock lock(lock_);
    callback_ = callback;
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
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    if (false == bool(callback_)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Callback not registered. "
              << std::endl;

        return false;
    }

    if (0 != zmq_connect(socket_, endpoint.c_str())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to connect to "
              << endpoint << std::endl;

        return false;
    }

    return true;
}

SubscribeSocket::~SubscribeSocket() {}
}  // namespace opentxs::network::zeromq::implementation
