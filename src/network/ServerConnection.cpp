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

#include "opentxs/network/ServerConnection.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include <chrono>
#include <cstdint>

#define OT_METHOD "opentxs::ServerConnection::"

namespace opentxs
{
ServerConnection::ServerConnection(
    const std::string& server,
    const std::string& proxy,
    std::atomic<bool>& shutdown,
    std::atomic<std::chrono::seconds>& keepAlive,
    api::network::ZMQ& zmq,
    api::Settings& config)
    : shutdown_(shutdown)
    , keep_alive_(keepAlive)
    , zmq_(zmq)
    , config_(config)
    , context_(zmq.Context())
    , remote_contract_(nullptr)
    , remote_endpoint_(GetRemoteEndpoint(server, remote_contract_))
    , request_socket_(context_.NewRequestSocket())
    , lock_(new std::mutex)
    , thread_(nullptr)
    , last_activity_(0)
    , status_(false)
    , use_proxy_(true)
{
    OT_ASSERT(request_socket_);
    OT_ASSERT(lock_);

    ResetTimer();
    Init(proxy);
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

bool ServerConnection::ClearProxy()
{
    Lock lock(*lock_);

    use_proxy_.store(false);
    ResetSocket();

    return true;
}

bool ServerConnection::EnableProxy()
{
    Lock lock(*lock_);

    use_proxy_.store(true);
    ResetSocket();

    return true;
}

void ServerConnection::Init(const std::string& proxy)
{
    status_.store(false);

    if (use_proxy_.load()) {
        SetProxy(proxy);
    }

    SetTimeouts();
    SetCurve();
    request_socket_->Start(remote_endpoint_);
}

void ServerConnection::ResetSocket()
{
    request_socket_->Close();
    request_socket_ = context_.NewRequestSocket();

    if (false == bool(request_socket_)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed trying to reset socket."
              << std::endl;

        OT_FAIL;
    }

    std::string proxy{};

    if (use_proxy_.load()) {
        zmq_.SocksProxy(proxy);
    }

    Init(proxy);
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

    contract = OT::App().Wallet().Server(Identifier(server));

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

void ServerConnection::ResetTimer()
{
    last_activity_.store(std::time(nullptr));
}

NetworkReplyRaw ServerConnection::Send(const std::string& input)
{
    OT_ASSERT(lock_);

    Lock lock(*lock_);

    NetworkReplyRaw output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(new std::string);

    OT_ASSERT(reply);

    auto message = context_.NewMessage(input);

    OT_ASSERT(message);

    auto result = request_socket_->SendRequest(*message);
    status = result.first;

    OT_ASSERT(result.second);

    switch (status) {
        case SendResult::ERROR: {
            ResetSocket();
        } break;
        case SendResult::TIMEOUT: {
            status_.store(false);
            ResetTimer();
        } break;
        case SendResult::VALID_REPLY: {
            status_.store(true);
            ResetTimer();
            reply.reset(new std::string(*result.second));

            OT_ASSERT(reply);
        } break;
        default: {
            OT_FAIL;
        }
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

void ServerConnection::SetCurve()
{
    OT_ASSERT(remote_contract_);

    const auto set = request_socket_->SetCurve(*remote_contract_);

    OT_ASSERT(set);
}

void ServerConnection::SetProxy(const std::string& proxy)
{
    OT_ASSERT(nullptr != request_socket_);

    if (false == proxy.empty()) {
        const auto set = request_socket_->SetSocksProxy(proxy);

        OT_ASSERT(set);
    }
}

void ServerConnection::SetTimeouts()
{
    OT_ASSERT(nullptr != request_socket_);

    const auto set = request_socket_->SetTimeouts(
        zmq_.Linger(), zmq_.SendTimeout(), zmq_.ReceiveTimeout());

    OT_ASSERT(set);
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
}
}  // namespace opentxs
