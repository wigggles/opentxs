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

#include "CurveServer.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"

#include "Socket.hpp"

#include <zmq.h>

#define OT_METHOD "opentxs::network::zeromq::implementation::CurveServer::"

namespace opentxs::network::zeromq::implementation
{
CurveServer::CurveServer(std::mutex& lock, void* socket)
    : curve_lock_(lock)
    , curve_socket_(socket)
{
}

bool CurveServer::set_curve(const OTPassword& key) const
{
    OT_ASSERT(nullptr != curve_socket_);

    Lock lock(curve_lock_);

    if (CURVE_KEY_BYTES != key.getMemorySize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid private key."
              << std::endl;

        return false;
    }

    const int server{1};
    auto set = zmq_setsockopt(
        curve_socket_, ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_CURVE_SERVER"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        curve_socket_,
        ZMQ_CURVE_SECRETKEY,
        key.getMemory(),
        key.getMemorySize());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

CurveServer::~CurveServer() { curve_socket_ = nullptr; }
}  // namespace opentxs::network::zeromq::implementation
