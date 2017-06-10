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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/network/ServerConnection.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif

#include <chrono>
#include <cstdint>

namespace opentxs
{
ServerConnection::ServerConnection(
    const std::string& server,
    std::atomic<bool>& shutdown,
    std::atomic<std::chrono::seconds>& keepAlive)
        : remote_endpoint_(GetRemoteEndpoint(server, remote_contract_))
        , request_socket_(zsock_new_req(nullptr))
        , lock_(new std::mutex)
        , shutdown_(shutdown)
        , keep_alive_(keepAlive)
{
    if (!zsys_has_curve()) {
        otErr << __FUNCTION__ << ": libzmq has no libsodium support."
              << std::endl;

        OT_FAIL;
    }

    OT_ASSERT(lock_);

    ResetTimer();
    Init();
    thread_.reset(new std::thread(&ServerConnection::Thread, this));
}

ServerConnection::~ServerConnection()
{
    if (thread_) {
        thread_->join();
    }

    zsock_destroy(&request_socket_);
}

void ServerConnection::Init()
{
    shutdown_.store(false);
    status_.store(false);
    SetProxy();
    SetTimeouts();
    SetRemoteKey();

    if (0 != zsock_connect(request_socket_, "%s", remote_endpoint_.c_str())) {
        otErr << __FUNCTION__ << ": Failed to connect to " << remote_endpoint_
              << std::endl;
    }
}

void ServerConnection::ResetSocket()
{
    zsock_destroy(&request_socket_);
    request_socket_ = zsock_new_req(nullptr);

    if (nullptr == request_socket_) {
        otErr << __FUNCTION__ << ": Failed trying to reset socket."
              << std::endl;

        OT_FAIL;
    }

    Init();
}

std::string ServerConnection::GetRemoteEndpoint(
    const std::string& server,
    std::shared_ptr<const ServerContract>& contract)
{
    bool changed = false;
    std::int64_t preferred = 0;
    OT::App().Config().CheckSet_long(
        "Connection",
        "preferred_address_type",
        static_cast<std::int64_t>(proto::ADDRESSTYPE_IPV4),
        preferred,
        changed);

    if (changed) {
        OT::App().Config().Save();
    }

    contract = OT::App().Contract().Server(Identifier(server));

    std::uint32_t port = 0;
    std::string hostname;

    if (!contract->ConnectInfo(
        hostname,
        port,
        static_cast<proto::AddressType>(preferred))) {
            otErr << __FUNCTION__ << ": Failed retrieving connection info from "
                  << "server contract." << std::endl;

            OT_FAIL;
    }

    const std::string endpoint =
        "tcp://" + hostname + ":" + std::to_string(port);
    otErr << "Establishing connection to: " << endpoint << std::endl;

    return endpoint;
}

bool ServerConnection::Receive(std::string& reply)
{
    char* message = zstr_recv(request_socket_);

    if (nullptr == message) { return false; }

    reply.assign(message);
    zstr_free(&message);

    return true;
}

void ServerConnection::ResetTimer()
{
    last_activity_.store(std::time(nullptr));
}

NetworkReplyRaw ServerConnection::Send(const std::string& message)
{
    OT_ASSERT(lock_);

    std::lock_guard<std::mutex> lock(*lock_);

    NetworkReplyRaw output{SendResult::ERROR_SENDING, nullptr};
    output.second.reset(new std::string);

    OT_ASSERT(output.second);

    const bool sent = (0 == zstr_send(request_socket_, message.c_str()));

    if (!sent) {
        ResetSocket();

        return output;
    }

    ResetTimer();
    const bool received = Receive(*output.second);

    if (received) {
        status_.store(true);
        output.first = SendResult::HAVE_REPLY;
    } else {
        status_.store(false);
        output.first = SendResult::TIMEOUT_RECEIVING;
    }

    return output;
}

NetworkReplyString ServerConnection::Send(const String& message)
{
    OTASCIIArmor envelope(message);
    NetworkReplyString output{SendResult::ERROR_SENDING, nullptr};
    output.second.reset(new String);

    OT_ASSERT(output.second);

    if (!envelope.Exists()) { return output; }

    auto rawOutput = Send(std::string(envelope.Get()));
    output.first = rawOutput.first;

    if (SendResult::HAVE_REPLY == output.first) {
        OTASCIIArmor reply;
        reply.Set(rawOutput.second->c_str());
        reply.GetString(*output.second);
    }

    return output;
}

NetworkReplyMessage ServerConnection::Send(const Message& message)
{
    NetworkReplyMessage output{SendResult::ERROR_SENDING, nullptr};
    output.second.reset(new Message);

    OT_ASSERT(output.second);

    String input;
    message.SaveContractRaw(input);
    auto rawOutput = Send(input);
    output.first = rawOutput.first;

    if (SendResult::HAVE_REPLY == output.first) {
        output.second->LoadContractFromString(*rawOutput.second);
    }

    return output;
}

void ServerConnection::SetRemoteKey()
{
    zsock_set_curve_serverkey_bin(
        request_socket_, remote_contract_->PublicTransportKey());
}

void ServerConnection::SetProxy()
{
    std::string proxy;

    if (OT::App().ZMQ().SocksProxy(proxy)) {
        OT_ASSERT(nullptr != request_socket_);

        zsock_set_socks_proxy(request_socket_, proxy.c_str());
    }
}

void ServerConnection::SetTimeouts()
{
    zsock_set_linger(
        request_socket_,
        OT::App().ZMQ().Linger().count());
    zsock_set_sndtimeo(
        request_socket_,
        OT::App().ZMQ().SendTimeout().count());
    zsock_set_rcvtimeo(
        request_socket_,
        OT::App().ZMQ().ReceiveTimeout().count());
    zcert_apply(zcert_new(), request_socket_);
}

bool ServerConnection::Status() const
{
    return status_.load();
}

void ServerConnection::Thread()
{
    while (!shutdown_.load()) {
        const auto limit = keep_alive_.load();
        const auto now = std::chrono::seconds(std::time(nullptr));
        const auto last = std::chrono::seconds(last_activity_.load());
        const auto duration = now - last;

        if (duration > limit) {
            if (limit > std::chrono::seconds(0)) {
                Send(std::string(""));
            } else {
                status_.store(false);
            }
        }

        Log::Sleep(std::chrono::seconds(1));
    }
}
}  // namespace opentxs
