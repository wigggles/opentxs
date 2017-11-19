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

#include "opentxs/network/zeromq/implementation/RequestSocket.hpp"

#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#include <array>

#define OT_METHOD "opentxs::network::zeromq::implementation::RequestSocket::"

namespace opentxs::network::zeromq::implementation
{
RequestSocket::RequestSocket(const zeromq::Context& context)
    : ot_super(context, SocketType::Request)
{
}

Socket::MessageSendResult RequestSocket::SendRequest(opentxs::Data& input)
{
    auto message = context_.NewMessage(input);

    OT_ASSERT(message);

    return SendRequest(*message);
}

Socket::MessageSendResult RequestSocket::SendRequest(std::string& input)
{
    auto message = context_.NewMessage(input);

    OT_ASSERT(message);

    return SendRequest(*message);
}

Socket::MessageSendResult RequestSocket::SendRequest(zeromq::Message& request)
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    MessageSendResult output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply = context_.NewMessage();

    OT_ASSERT(reply);

    const bool sent = (-1 != zmq_msg_send(request, socket_, 0));

    if (false == sent) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to send." << std::endl;

        return output;
    }

    const bool received = (-1 != zmq_msg_recv(*reply, socket_, 0));

    if (false == received) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Timeout waiting for server reply." << std::endl;
        status = SendResult::TIMEOUT;

        return output;
    }

    status = SendResult::VALID_REPLY;

    return output;
}

bool RequestSocket::set_local_keys(const Lock&)
{
    OT_ASSERT(nullptr != socket_);

    std::array<char, CURVE_KEY_Z85_BYTES + 1> publicKey{};
    std::array<char, CURVE_KEY_Z85_BYTES + 1> secretKey{};
    auto* pubkey = &publicKey[0];
    auto* privkey = &secretKey[0];
    auto set = zmq_curve_keypair(pubkey, privkey);

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to generate keypair."
              << std::endl;

        return false;
    }

    set =
        zmq_setsockopt(socket_, ZMQ_CURVE_PUBLICKEY, pubkey, publicKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set public key."
              << std::endl;

        return false;
    }

    set =
        zmq_setsockopt(socket_, ZMQ_CURVE_SECRETKEY, privkey, secretKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

bool RequestSocket::set_remote_key(const Lock&, const ServerContract& contract)
{
    OT_ASSERT(nullptr != socket_);

    const auto& key = contract.TransportKey();

    if (CURVE_KEY_BYTES != key.GetSize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server key."
              << std::endl;

        return false;
    }

    const auto set = zmq_setsockopt(
        socket_, ZMQ_CURVE_SERVERKEY, key.GetPointer(), key.GetSize());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set server key."
              << std::endl;

        return false;
    }

    return true;
}

bool RequestSocket::SetCurve(const ServerContract& contract)
{
    Lock lock(lock_);

    if (false == set_remote_key(lock, contract)) {

        return false;
    }

    return set_local_keys(lock);
}

bool RequestSocket::SetSocksProxy(const std::string& proxy)
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    const auto set =
        zmq_setsockopt(socket_, ZMQ_SOCKS_PROXY, proxy.data(), proxy.size());

    return (0 == set);
}

bool RequestSocket::Start(const std::string& endpoint)
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
}  // namespace opentxs::network::zeromq::implementation
