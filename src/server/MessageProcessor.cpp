// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "MessageProcessor.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
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
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RouterSocket.hpp"

#include "Server.hpp"
#include "UserCommandProcessor.hpp"

#include <stddef.h>
#include <sys/types.h>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::MessageProcessor::"

template class opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>;
template class opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>;

namespace opentxs::server
{

MessageProcessor::MessageProcessor(
    Server& server,
    const network::zeromq::Context& context,
    const Flag& running)
    : server_(server)
    , running_(running)
    , context_(context)
    , frontend_callback_(network::zeromq::ListenCallback::Factory(
          [=](const network::zeromq::Message& incoming) -> void {
              this->process_frontend(incoming);
          }))
    , frontend_socket_(context.RouterSocket(frontend_callback_.get(), false))
    , backend_callback_(network::zeromq::ReplyCallback::Factory(
          [=](const network::zeromq::Message& incoming) -> OTZMQMessage {
              return this->process_backend(incoming);
          }))
    , backend_socket_(context.ReplySocket(backend_callback_.get(), false))
    , internal_callback_(network::zeromq::ListenCallback::Factory(
          [=](const network::zeromq::Message& incoming) -> void {
              this->process_internal(incoming);
          }))
    , internal_socket_(context.DealerSocket(internal_callback_, true))
    , thread_(nullptr)
    , internal_endpoint_(
          std::string("inproc://opentxs/notary/") + Identifier::Random()->str())
    , counter_lock_()
    , drop_incoming_(0)
    , drop_outgoing_(0)
{
    auto bound = backend_socket_->Start(internal_endpoint_);
    bound &= internal_socket_->Start(internal_endpoint_);

    OT_ASSERT(bound);
}

void MessageProcessor::cleanup()
{
    if (thread_) {
        thread_->join();
        thread_.reset();
    }
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

void MessageProcessor::init(
    const bool inproc,
    const int port,
    const OTPassword& privkey)
{
    if (port == 0) { OT_FAIL; }

    const auto set = frontend_socket_->SetPrivateKey(privkey);

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

OTZMQMessage MessageProcessor::process_backend(
    const network::zeromq::Message& incoming)
{
    // ProcessCron and process_backend must not run simultaneously
    Lock lock(lock_);
    std::string reply{};

    std::string messageString{};
    if (0 < incoming.Body().size()) {
        messageString = *incoming.Body().begin();
    }

    bool error = processMessage(messageString, reply);

    if (error) { reply = ""; }

    auto output = network::zeromq::Message::ReplyFactory(incoming);
    output->AddFrame(reply);

    return output;
}

void MessageProcessor::process_frontend(
    const network::zeromq::Message& incoming)
{
    Lock lock(counter_lock_);

    if (0 < drop_incoming_) {
        --drop_incoming_;
    } else {
        OTZMQMessage request{incoming};
        internal_socket_->Send(request);
    }
}

void MessageProcessor::process_internal(
    const network::zeromq::Message& incoming)
{
    Lock lock(counter_lock_);

    if (0 < drop_outgoing_) {
        --drop_outgoing_;
    } else {
        OTZMQMessage reply{incoming};
        frontend_socket_->Send(reply);
    }
}

bool MessageProcessor::processMessage(
    const std::string& messageString,
    std::string& reply)
{
    if (messageString.size() < 1) { return true; }

    Armored armored;
    armored.MemSet(messageString.data(), messageString.size());
    String serialized;
    armored.GetString(serialized);
    auto request{server_.API().Factory().Message(server_.API())};

    if (false == serialized.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty serialized request."
              << std::endl;

        return true;
    }

    if (false == request->LoadContractFromString(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to deserialized request." << std::endl;

        return true;
    }

    auto replymsg{server_.API().Factory().Message(server_.API())};

    OT_ASSERT(false != bool(replymsg));

    const bool processed =
        server_.CommandProcessor().ProcessUserCommand(*request, *replymsg);

    if (false == processed) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Failed to process user command " << request->m_strCommand
               << std::endl;
        otInfo << String(*request) << std::endl;
    } else {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Successfully processed user command "
               << request->m_strCommand << std::endl;
    }

    String serializedReply(*replymsg);

    if (false == serializedReply.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to serialize reply."
              << std::endl;

        return true;
    }

    Armored armoredReply(serializedReply);

    if (false == armoredReply.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to armor reply."
              << std::endl;

        return true;
    }

    reply.assign(armoredReply.Get(), armoredReply.GetLength());

    return false;
}

void MessageProcessor::Start()
{
    if (false == bool(thread_)) {
        thread_.reset(new std::thread(&MessageProcessor::run, this));
    }
}

MessageProcessor::~MessageProcessor() {}
}  // namespace opentxs::server
