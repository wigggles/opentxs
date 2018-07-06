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

#include "stdafx.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <mutex>
#include <thread>

#include "ServerConnection.hpp"

template class opentxs::Pimpl<opentxs::network::ServerConnection>;

#define OT_METHOD "opentxs::ServerConnection::"

namespace opentxs::network
{
OTServerConnection ServerConnection::Factory(
    const api::network::ZMQ& zmq,
    const std::string& serverID,
    const zeromq::PublishSocket& updates)
{
    return OTServerConnection(
        new implementation::ServerConnection(zmq, serverID, updates));
}
}  // namespace opentxs::network

namespace opentxs::network::implementation
{
ServerConnection::ServerConnection(
    const opentxs::api::network::ZMQ& zmq,
    const std::string& serverID,
    const zeromq::PublishSocket& updates)
    : zmq_(zmq)
    , updates_(updates)
    , server_id_(serverID)
    , address_type_(zmq.DefaultAddressType())
    , remote_contract_(OT::App().Wallet().Server(Identifier::Factory(serverID)))
    , thread_(nullptr)
    , socket_(zmq.Context().RequestSocket())
    , last_activity_(std::time(nullptr))
    , socket_ready_(Flag::Factory(false))
    , status_(Flag::Factory(false))
    , use_proxy_(Flag::Factory(false))
{
    thread_.reset(new std::thread(&ServerConnection::activity_timer, this));

    OT_ASSERT(remote_contract_)
    OT_ASSERT(thread_)
}

bool ServerConnection::ChangeAddressType(const proto::AddressType type)
{
    Lock lock(lock_);
    address_type_ = type;
    reset_socket(lock);

    return true;
}

bool ServerConnection::ClearProxy()
{
    Lock lock(lock_);
    use_proxy_->Off();
    reset_socket(lock);

    return true;
}

bool ServerConnection::EnableProxy()
{
    Lock lock(lock_);
    use_proxy_->On();
    reset_socket(lock);

    return true;
}

std::string ServerConnection::endpoint() const
{
    std::uint32_t port{0};
    std::string hostname{""};
    const auto have =
        remote_contract_->ConnectInfo(hostname, port, address_type_);

    if (false == have) {
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

zeromq::RequestSocket& ServerConnection::get_socket(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock))

    if (false == socket_ready_.get()) {
        socket_ = socket(lock);
        socket_ready_->On();
    }

    return socket_;
}

void ServerConnection::publish() const
{
    const bool state(status_.get());
    auto message = opentxs::network::zeromq::Message::Factory();
    message->AddFrame(server_id_);
    message->AddFrame(Data::Factory(&state, sizeof(state)));
    updates_.Publish(message);
}

void ServerConnection::reset_socket(const Lock& lock)
{
    OT_ASSERT(verify_lock(lock))

    socket_ready_->Off();
}

void ServerConnection::reset_timer()
{
    last_activity_.store(std::time(nullptr));
}

NetworkReplyRaw ServerConnection::Send(const std::string& input)
{
    Lock lock(lock_);
    NetworkReplyRaw output{SendResult::ERROR, nullptr};
    auto& status = output.first;
    auto& reply = output.second;
    reply.reset(new std::string);

    OT_ASSERT(reply);

    auto result =
        get_socket(lock).SendRequest(network::zeromq::Message::Factory(input));
    status = result.first;
    network::zeromq::Message& message = result.second;

    switch (status) {
        case SendResult::ERROR: {
            if (status_->Off()) { publish(); }

            reset_socket(lock);
        } break;
        case SendResult::TIMEOUT: {
            if (status_->Off()) { publish(); }

            reset_socket(lock);
        } break;
        case SendResult::VALID_REPLY: {
            if (status_->On()) { publish(); }

            reset_timer();

            if (0 < input.size()) {
                OT_ASSERT(1 == message.Body().size());

                reply.reset(new std::string(*message.Body().begin()));
            }

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

    if (!envelope.Exists()) { return output; }

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

void ServerConnection::set_curve(
    const Lock& lock,
    zeromq::RequestSocket& socket) const
{
    OT_ASSERT(verify_lock(lock));

    const auto set = socket.SetCurve(*remote_contract_);

    OT_ASSERT(set);
}

void ServerConnection::set_proxy(
    const Lock& lock,
    zeromq::RequestSocket& socket) const
{
    OT_ASSERT(verify_lock(lock));

    if (false == use_proxy_.get()) { return; }

    auto proxy = zmq_.SocksProxy();

    if (false == proxy.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Setting proxy to " << proxy
              << std::endl;
        const auto set = socket.SetSocksProxy(proxy);

        OT_ASSERT(set);
    }
}

void ServerConnection::set_timeouts(
    const Lock& lock,
    zeromq::RequestSocket& socket) const
{
    OT_ASSERT(verify_lock(lock));

    const auto set = socket.SetTimeouts(
        zmq_.Linger(), zmq_.SendTimeout(), zmq_.ReceiveTimeout());

    OT_ASSERT(set);
}

OTZMQRequestSocket ServerConnection::socket(const Lock& lock) const
{
    auto output = zmq_.Context().RequestSocket();
    set_proxy(lock, output);
    set_timeouts(lock, output);
    set_curve(lock, output);
    output->Start(endpoint());

    return output;
}

bool ServerConnection::Status() const { return status_.get(); }

void ServerConnection::activity_timer()
{
    while (zmq_.Running()) {
        const auto limit = zmq_.KeepAlive();
        const auto now = std::chrono::seconds(std::time(nullptr));
        const auto last = std::chrono::seconds(last_activity_.load());
        const auto duration = now - last;

        if (duration > limit) {
            if (limit > std::chrono::seconds(0)) {
                Send(std::string(""));
            } else {
                if (status_->Off()) { publish(); };
            }
        }

        Log::Sleep(std::chrono::seconds(1));
    }
}

ServerConnection::~ServerConnection()
{
    if (thread_) { thread_->join(); }
}
}  // namespace opentxs::network::implementation
