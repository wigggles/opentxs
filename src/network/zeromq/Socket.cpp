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

#include "Socket.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"

#include <zmq.h>

#define CONTACT_UPDATE_ENDPOINT "inproc://opentxs/contactupdate/1"
#define NYM_UPDATE_ENDPOINT "inproc://opentxs/nymupdate/1"
#define PENDING_BAILMENT_ENDPOINT                                              \
    "inproc://opentxs/peerrequest/pendingbailment/1"
#define THREAD_UPDATE_ENDPOINT "inproc://opentxs/threadupdate/1/"

#define OT_METHOD "opentxs::network::zeromq::implementation::Socket::"

namespace opentxs::network::zeromq
{
const std::string Socket::ContactUpdateEndpoint{CONTACT_UPDATE_ENDPOINT};
const std::string Socket::NymDownloadEndpoint{NYM_UPDATE_ENDPOINT};
const std::string Socket::PendingBailmentEndpoint{PENDING_BAILMENT_ENDPOINT};
const std::string Socket::ThreadUpdateEndpoint{THREAD_UPDATE_ENDPOINT};
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
const std::map<SocketType, int> Socket::types_{
    {SocketType::Request, ZMQ_REQ},
    {SocketType::Reply, ZMQ_REP},
    {SocketType::Publish, ZMQ_PUB},
    {SocketType::Subscribe, ZMQ_SUB},
};

Socket::Socket(const Context& context, const SocketType type)
    : context_(context)
    , socket_(zmq_socket(context, types_.at(type)))
    , type_(type)
{
    OT_ASSERT(nullptr != socket_);
}

Socket::operator void*() const { return socket_; }

bool Socket::Close() const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);

    return (0 == zmq_close(socket_));
}

bool Socket::set_socks_proxy(const std::string& proxy) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    const auto set =
        zmq_setsockopt(socket_, ZMQ_SOCKS_PROXY, proxy.data(), proxy.size());

    return (0 == set);
}

bool Socket::SetTimeouts(
    const std::chrono::milliseconds& linger,
    const std::chrono::milliseconds& send,
    const std::chrono::milliseconds& receive) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    int value(linger.count());
    auto set = zmq_setsockopt(socket_, ZMQ_LINGER, &value, sizeof(value));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_LINGER"
              << std::endl;

        return false;
    }

    value = send.count();
    set = zmq_setsockopt(socket_, ZMQ_SNDTIMEO, &value, sizeof(value));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_SNDTIMEO"
              << std::endl;

        return false;
    }

    value = receive.count();
    set = zmq_setsockopt(socket_, ZMQ_RCVTIMEO, &value, sizeof(value));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_RCVTIMEO"
              << std::endl;

        return false;
    }

    return true;
}

bool Socket::SetTimeouts(
    const std::uint64_t& lingerMilliseconds,
    const std::uint64_t& sendMilliseconds,
    const std::uint64_t& receiveMilliseconds) const
{
    return SetTimeouts(
        std::chrono::milliseconds(lingerMilliseconds),
        std::chrono::milliseconds(sendMilliseconds),
        std::chrono::milliseconds(receiveMilliseconds));
}

SocketType Socket::Type() const { return type_; }

Socket::~Socket()
{
    Lock lock(lock_);

    if (nullptr != socket_) {
        zmq_close(socket_);
    }
}
}  // namespace opentxs::network::zeromq::implementation
