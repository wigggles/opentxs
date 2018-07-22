// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "MessageProcessor.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"

#include "Server.hpp"
#include "UserCommandProcessor.hpp"

#include <stddef.h>
#include <sys/types.h>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::MessageProcessor::"

namespace opentxs::server
{

MessageProcessor::MessageProcessor(
    Server& server,
    const network::zeromq::Context& context,
    const Flag& running)
    : server_(server)
    , running_(running)
    , context_(context)
    , reply_socket_callback_(network::zeromq::ReplyCallback::Factory(
          [this](const network::zeromq::Message& incoming) -> OTZMQMessage {
              return this->processSocket(incoming);
          }))
    , reply_socket_(context.ReplySocket(reply_socket_callback_.get(), false))
    , thread_(nullptr)
{
}

void MessageProcessor::cleanup()
{
    if (thread_) {
        thread_->join();
        thread_.reset();
    }
}

void MessageProcessor::init(const int port, const OTPassword& privkey)
{
    if (port == 0) { OT_FAIL; }

    const auto set = reply_socket_->SetCurve(privkey);

    OT_ASSERT(set);

    const auto endpoint = std::string("tcp://*:") + std::to_string(port);
    const auto bound = reply_socket_->Start(endpoint);

    OT_ASSERT(bound);
}

void MessageProcessor::run()
{
    while (running_) {
        // timeout is the time left until the next cron should execute.
        const auto timeout = server_.ComputeTimeout();

        if (timeout <= 0) {
            // ProcessCron and processSocket must not run simultaneously
            Lock lock(lock_);
            server_.ProcessCron();
        }

        Log::Sleep(std::chrono::milliseconds(50));
    }
}

OTZMQMessage MessageProcessor::processSocket(
    const network::zeromq::Message& incoming)
{
    // ProcessCron and processSocket must not run simultaneously
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

bool MessageProcessor::processMessage(
    const std::string& messageString,
    std::string& reply)
{
    if (messageString.size() < 1) { return true; }

    Armored armored;
    armored.MemSet(messageString.data(), messageString.size());
    String serialized;
    armored.GetString(serialized);
    Message request;

    if (false == serialized.Exists()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Empty serialized request."
              << std::endl;

        return true;
    }

    if (false == request.LoadContractFromString(serialized)) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to deserialized request." << std::endl;

        return true;
    }

    Message repy{};
    const bool processed =
        server_.CommandProcessor().ProcessUserCommand(request, repy);

    if (false == processed) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Failed to process user command " << request.m_strCommand
               << std::endl;
        otInfo << String(request) << std::endl;
    } else {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Successfully processed user command "
               << request.m_strCommand << std::endl;
    }

    String serializedReply(repy);

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
