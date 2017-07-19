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
#include "opentxs/network/ZMQ.hpp"

#include <chrono>
#include <cstdint>

#define OT_METHOD "opentxs::ServerConnection::"

namespace opentxs
{
ServerConnection::ServerConnection(
    const std::string& server,
    std::atomic<bool>& shutdown,
    std::atomic<std::chrono::seconds>& keepAlive,
    ZMQ& zmq,
    Settings& config)
    : shutdown_(shutdown)
    , keep_alive_(keepAlive)
    , zmq_(zmq)
    , config_(config)
    , remote_contract_(nullptr)
    , remote_endpoint_(GetRemoteEndpoint(server, remote_contract_))
    , request_socket_(zsock_new_req(nullptr))
    , lock_(new std::mutex)
    , thread_(nullptr)
    , last_activity_(0)
    , status_(false)
{
    shutdown_.store(false);

    if (false == zsys_has_curve()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": libzmq has no libsodium support." << std::endl;

        OT_FAIL;
    }

    OT_ASSERT(lock_);

    ResetTimer();
    Init();
    thread_.reset(new std::thread(&ServerConnection::Thread, this));
}

bool ServerConnection::ChangeAddressType(const proto::AddressType type)
{
    Lock lock(*lock_);

    std::uint32_t port{0};
    std::string hostname{};

    OT_ASSERT(remote_contract_);

    if (false == remote_contract_->ConnectInfo(hostname, port, type)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Unable to extract connection info." << std::endl;

        return false;
    }

    auto& endpoint = const_cast<std::string&>(remote_endpoint_);
    endpoint = "tcp://" + hostname + ":" + std::to_string(port);
    otErr << OT_METHOD << __FUNCTION__
          << ": Changing endpoint to: " << remote_endpoint_ << std::endl;
    ResetSocket();

    return true;
}

void ServerConnection::Init()
{
    status_.store(false);
    SetProxy();
    SetTimeouts();
    SetRemoteKey();

    if (0 != zsock_connect(request_socket_, "%s", remote_endpoint_.c_str())) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to connect to "
              << remote_endpoint_ << std::endl;
    }
}

void ServerConnection::ResetSocket()
{
    zsock_destroy(&request_socket_);
    request_socket_ = zsock_new_req(nullptr);

    if (nullptr == request_socket_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed trying to reset socket."
              << std::endl;

        OT_FAIL;
    }

    Init();
}

std::string ServerConnection::GetRemoteEndpoint(
    const std::string& server,
    std::shared_ptr<const ServerContract>& contract) const
{
    bool changed = false;
    std::int64_t preferred = 0;
    config_.CheckSet_long(
        "Connection",
        "preferred_address_type",
        static_cast<std::int64_t>(proto::ADDRESSTYPE_IPV4),
        preferred,
        changed);

    if (changed) {
        config_.Save();
    }

    contract = OT::App().Contract().Server(Identifier(server));

    std::uint32_t port = 0;
    std::string hostname;

    if (!contract->ConnectInfo(
            hostname, port, static_cast<proto::AddressType>(preferred))) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed retrieving connection info from server contract."
              << std::endl;

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

    if (nullptr == message) {
        otErr << OT_METHOD << __FUNCTION__ << ": No server reply." << std::endl;

        return false;
    }

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

    Lock lock(*lock_);

    NetworkReplyRaw output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(new std::string);

    OT_ASSERT(reply);

    const bool sent = (0 == zstr_send(request_socket_, message.c_str()));

    if (!sent) {
        ResetSocket();

        return output;
    }

    ResetTimer();
    const bool received = Receive(*reply);

    if (received) {
        status_.store(true);
        status = SendResult::VALID_REPLY;
    } else {
        otErr << OT_METHOD << __FUNCTION__
              << ": Timeout waiting for server reply." << std::endl;
        status_.store(false);
        status = SendResult::TIMEOUT;
    }

    return output;
}

NetworkReplyString ServerConnection::Send(const String& message)
{
    OTASCIIArmor envelope(message);
    NetworkReplyString output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(new String);

    OT_ASSERT(reply);

    if (!envelope.Exists()) {
        return output;
    }

    auto rawOutput = Send(std::string(envelope.Get()));
    status = rawOutput.first;

    if (SendResult::VALID_REPLY == status) {
        OTASCIIArmor armored;
        armored.Set(rawOutput.second->c_str());

        if (false == armored.GetString(*reply)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Received server reply, "
                  << "but unable to decode it into a String." << std::endl;
            reply.reset();
            status = SendResult::INVALID_REPLY;
        }
    }

    return output;
}

NetworkReplyMessage ServerConnection::Send(const Message& message)
{
    NetworkReplyMessage output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(new Message);

    OT_ASSERT(reply);

    String input;
    message.SaveContractRaw(input);
    auto rawOutput = Send(input);
    status = rawOutput.first;

    if (SendResult::VALID_REPLY == status) {
        if (false == reply->LoadContractFromString(*rawOutput.second)) {
            otErr << OT_METHOD << __FUNCTION__ << ": Received server reply, "
                  << "but unable to instantiate it as a Message." << std::endl;
            reply.reset();
            status = SendResult::INVALID_REPLY;
        }
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

    if (zmq_.SocksProxy(proxy)) {
        OT_ASSERT(nullptr != request_socket_);

        zsock_set_socks_proxy(request_socket_, proxy.c_str());
    }
}

void ServerConnection::SetTimeouts()
{
    zsock_set_linger(request_socket_, zmq_.Linger().count());
    zsock_set_sndtimeo(request_socket_, zmq_.SendTimeout().count());
    zsock_set_rcvtimeo(request_socket_, zmq_.ReceiveTimeout().count());
    zcert_apply(zcert_new(), request_socket_);
}

bool ServerConnection::Status() const { return status_.load(); }

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

ServerConnection::~ServerConnection()
{
    if (thread_) {
        thread_->join();
    }

    zsock_destroy(&request_socket_);
}
}  // namespace opentxs
