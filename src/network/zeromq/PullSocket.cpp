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

#include "PullSocket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::PullSocket::"

namespace opentxs::network::zeromq
{
OTZMQPullSocket PullSocket::Factory(
    const Context& context,
    const ListenCallback& callback)
{
    return OTZMQPullSocket(new implementation::PullSocket(context, callback));
}

OTZMQPullSocket PullSocket::Factory(const Context& context)
{
    return OTZMQPullSocket(new implementation::PullSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PullSocket::PullSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : ot_super(context, SocketType::Subscribe)
    , CurveServer(lock_, socket_)
    , Receiver(lock_, socket_, startThread)
    , callback_(callback)
{
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback)
    : PullSocket(context, callback, true)
{
}

PullSocket::PullSocket(const zeromq::Context& context)
    : PullSocket(context, ListenCallback::Factory(), false)
{
}

PullSocket* PullSocket::clone() const
{
    return new PullSocket(context_, callback_);
}

bool PullSocket::have_callback() const { return true; }

void PullSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PullSocket::SetCurve(const OTPassword& key) const
{
    return set_curve(key);
}

bool PullSocket::Start(const std::string& endpoint) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    if (0 != zmq_connect(socket_, endpoint.c_str())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to connect to "
              << endpoint << std::endl;

        return false;
    }

    return true;
}

PullSocket::~PullSocket() {}
}  // namespace opentxs::network::zeromq::implementation
