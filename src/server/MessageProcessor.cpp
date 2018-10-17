// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "MessageProcessor.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/DealerSocket.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RouterSocket.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/Request.hpp"

#include "Server.hpp"
#include "UserCommandProcessor.hpp"

#include <stddef.h>
#include <sys/types.h>
#include <ostream>
#include <string>

#define OTX_ZAP_DOMAIN "opentxs-otx"

#define OT_METHOD "opentxs::MessageProcessor::"

namespace zmq = opentxs::network::zeromq;

template class opentxs::Pimpl<zmq::ReplySocket>;
template class opentxs::Pimpl<zmq::ReplyCallback>;

namespace opentxs::server
{
MessageProcessor::MessageProcessor(
    Server& server,
    const zmq::Context& context,
    const Flag& running)
    : server_(server)
    , running_(running)
    , context_(context)
    , frontend_callback_(zmq::ListenCallback::Factory(
          [=](const zmq::Message& incoming) -> void {
              this->process_frontend(incoming);
          }))
    , frontend_socket_(context.RouterSocket(
          frontend_callback_,
          zmq::Socket::Direction::Bind))
    , backend_callback_(zmq::ReplyCallback::Factory(
          [=](const zmq::Message& incoming) -> OTZMQMessage {
              return this->process_backend(incoming);
          }))
    , backend_socket_(
          context.ReplySocket(backend_callback_, zmq::Socket::Direction::Bind))
    , internal_callback_(zmq::ListenCallback::Factory(
          [=](const zmq::Message& incoming) -> void {
              this->process_internal(incoming);
          }))
    , internal_socket_(context.DealerSocket(
          internal_callback_,
          zmq::Socket::Direction::Connect))
    , notification_callback_(zmq::ListenCallback::Factory(
          [=](const zmq::Message& incoming) -> void {
              this->process_notification(incoming);
          }))
    , notification_socket_(context.PullSocket(
          notification_callback_,
          zmq::Socket::Direction::Bind))
    , thread_()
    , internal_endpoint_(
          std::string("inproc://opentxs/notary/") + Identifier::Random()->str())
    , counter_lock_()
    , drop_incoming_(0)
    , drop_outgoing_(0)
{
    auto bound = backend_socket_->Start(internal_endpoint_);
    bound &= internal_socket_->Start(internal_endpoint_);
    bound &= notification_socket_->Start(
        server_.API().Endpoints().InternalPushNotification());

    OT_ASSERT(bound);
}

void MessageProcessor::associate_connection(
    const Identifier& nymID,
    const Data& connection)
{
    if (nymID.empty()) { return; }
    if (connection.empty()) { return; }

    eLock lock(connection_map_lock_);
    const auto result = active_connections_.emplace(nymID, connection);

    if (std::get<1>(result)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym " << nymID.str()
              << " is available via connection " << connection.asHex()
              << std::endl;
    }
}

void MessageProcessor::cleanup()
{
    if (thread_.joinable()) { thread_.join(); }
}

void MessageProcessor::DropIncoming(const int count) const
{
    Lock lock(counter_lock_);
    drop_incoming_ = count;
}

void MessageProcessor::DropOutgoing(const int count) const
{
    Lock lock(counter_lock_);
    drop_outgoing_ = count;
}

proto::ServerRequest MessageProcessor::extract_proto(
    const zmq::Frame& incoming) const
{
    return proto::RawToProto<proto::ServerRequest>(
        incoming.data(), incoming.size());
}

void MessageProcessor::init(
    const bool inproc,
    const int port,
    const OTPassword& privkey)
{
    if (port == 0) { OT_FAIL; }

    auto set = frontend_socket_->SetPrivateKey(privkey);

    OT_ASSERT(set);

    set = frontend_socket_->SetDomain(OTX_ZAP_DOMAIN);

    OT_ASSERT(set);

    std::stringstream endpoint{};

    if (inproc) {
        endpoint << "inproc://opentxs/notary/";
        endpoint << std::to_string(server_.API().Instance());
        endpoint << ":";
    } else {
        endpoint << std::string("tcp://*:");
    }

    endpoint << std::to_string(port);
    const auto bound = frontend_socket_->Start(endpoint.str());

    OT_ASSERT(bound);

    otErr << std::endl
          << OT_METHOD << __FUNCTION__ << ": Bound to endpoint "
          << endpoint.str() << std::endl;
}

void MessageProcessor::run()
{
    while (running_) {
        // timeout is the time left until the next cron should execute.
        const auto timeout = server_.ComputeTimeout();

        if (timeout <= 0) {
            // ProcessCron and process_backend must not run simultaneously
            Lock lock(lock_);
            server_.ProcessCron();
        }

        Log::Sleep(std::chrono::milliseconds(50));
    }
}

OTZMQMessage MessageProcessor::process_backend(const zmq::Message& incoming)
{
    // ProcessCron and process_backend must not run simultaneously
    Lock lock(lock_);
    std::string reply{};

    std::string messageString{};
    if (0 < incoming.Body().size()) {
        messageString = *incoming.Body().begin();
    }

    bool error = process_message(messageString, reply);

    if (error) { reply = ""; }

    auto output = zmq::Message::ReplyFactory(incoming);
    output->AddFrame(reply);

    return output;
}

bool MessageProcessor::process_command(
    const proto::ServerRequest& serialized,
    Identifier& nymID)
{
    const auto allegedNymID = Identifier::Factory(serialized.nym());
    const auto nym = server_.API().Wallet().Nym(allegedNymID);

    if (false == bool(nym)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Nym is not yet registered"
              << std::endl;

        return true;
    }

    auto request = otx::Request::Factory(server_.API(), serialized);

    if (request->Validate()) {
        nymID.Assign(request->Initiator());
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid request" << std::endl;

        return false;
    }

    // TODO look at the request type and do some stuff

    return true;
}

void MessageProcessor::process_frontend(const zmq::Message& incoming)
{
    Lock lock(counter_lock_);

    if (0 < drop_incoming_) {
        --drop_incoming_;
    } else {
        proto::ServerRequest command{};
        bool isProto{false};

        if (1 < incoming.Body().size()) {
            command = extract_proto(incoming.Body().at(0));
            isProto = proto::Validate(command, SILENT);
        }

        if (isProto) {
            auto nymID = Identifier::Factory();
            const auto valid = process_command(command, nymID);

            if (valid) {
                const auto headerFrames = incoming.Header().size();

                if (0 < headerFrames) {
                    const auto& frame = incoming.Header().at(0);
                    const auto id = Data::Factory(frame.data(), frame.size());
                    associate_connection(nymID, id);
                }

                return;
            }
        }

        OTZMQMessage request{incoming};
        internal_socket_->Send(request);
    }
}

void MessageProcessor::process_internal(const zmq::Message& incoming)
{
    Lock lock(counter_lock_);

    if (0 < drop_outgoing_) {
        --drop_outgoing_;
    } else {
        OTZMQMessage reply{incoming};
        frontend_socket_->Send(reply);
    }
}

bool MessageProcessor::process_message(
    const std::string& messageString,
    std::string& reply)
{
    if (messageString.size() < 1) { return true; }

    auto armored = Armored::Factory();
    armored->MemSet(messageString.data(), messageString.size());
    auto serialized = String::Factory();
    armored->GetString(serialized);
    auto request{server_.API().Factory().Message()};

    if (false == serialized->Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty serialized request."
              << std::endl;

        return true;
    }

    if (false == request->LoadContractFromString(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to deserialized request." << std::endl;

        return true;
    }

    auto replymsg{server_.API().Factory().Message()};

    OT_ASSERT(false != bool(replymsg));

    const bool processed =
        server_.CommandProcessor().ProcessUserCommand(*request, *replymsg);

    if (false == processed) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Failed to process user command ")(
            request->m_strCommand)
            .Flush();
        LogVerbose(OT_METHOD)(__FUNCTION__)(String::Factory(*request)).Flush();
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(
            ": Successfully processed user command ")(request->m_strCommand)
            .Flush();
    }

    auto serializedReply = String::Factory(*replymsg);

    if (false == serializedReply->Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to serialize reply."
              << std::endl;

        return true;
    }

    auto armoredReply = Armored::Factory(serializedReply);

    if (false == armoredReply->Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to armor reply."
              << std::endl;

        return true;
    }

    reply.assign(armoredReply->Get(), armoredReply->GetLength());

    return false;
}

void MessageProcessor::process_notification(const zmq::Message& incoming)
{
    if (2 != incoming.Body().size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message.").Flush();

        return;
    }

    const auto nymID = Identifier::Factory(incoming.Body().at(0));
    const auto connection = query_connection(nymID);

    if (connection->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": No notification channel available for ")(nymID)
            .Flush();

        return;
    }

    const auto nym = server_.API().Wallet().Nym(server_.GetServerNym().ID());

    OT_ASSERT(nym);

    const auto& payload = incoming.Body().at(1);
    auto message = otx::Reply::Factory(
        nym, nymID, server_.GetServerID(), proto::SERVERREPLY_PUSH, true);
    message->SetPush(proto::TextToProto<proto::OTXPush>(payload));

    OT_ASSERT(message->Validate());

    const auto reply = proto::ProtoAsData(message->Contract());
    auto pushNotification = zmq::Message::Factory();
    pushNotification->AddFrame(connection);
    pushNotification->AddFrame();
    pushNotification->AddFrame(reply);
    pushNotification->AddFrame();
    frontend_socket_->Send(pushNotification);
    LogOutput(OT_METHOD)(__FUNCTION__)(": Push notification for ")(nymID)(
        " delivered via ")(connection->asHex())
        .Flush();
}

OTData MessageProcessor::query_connection(const Identifier& nymID)
{
    sLock lock(connection_map_lock_);
    auto it = active_connections_.find(nymID);

    if (active_connections_.end() == it) { return Data::Factory(); }

    return it->second;
}

void MessageProcessor::Start()
{
    thread_ = std::thread(&MessageProcessor::run, this);
}

MessageProcessor::~MessageProcessor() { cleanup(); }
}  // namespace opentxs::server
