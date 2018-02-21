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

#include "opentxs/server/MessageProcessor.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/server/Server.hpp"
#include "opentxs/server/UserCommandProcessor.hpp"

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
    , reply_socket_(context.ReplySocket())
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
    if (port == 0) {
        OT_FAIL;
    }

    const auto set = reply_socket_->SetCurve(privkey);

    OT_ASSERT(set);

    const auto endpoint = std::string("tcp://*:") + std::to_string(port);
    const auto bound = reply_socket_->Start(endpoint);

    OT_ASSERT(bound);
}

void MessageProcessor::run()
{
    network::zeromq::Socket::RequestCallback callback =
        [this](const network::zeromq::Message& incoming) -> OTZMQMessage {
        return this->processSocket(incoming);
    };
    reply_socket_->RegisterCallback(callback);

    while (running_) {
        // timeout is the time left until the next cron should execute.
        const auto timeout = server_.computeTimeout();

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
    bool error = processMessage(std::string(incoming), reply);

    if (error) {
        reply = "";
    }

    return network::zeromq::Message::Factory(reply);
}

bool MessageProcessor::processMessage(
    const std::string& messageString,
    std::string& reply)
{
    if (messageString.size() < 1) {

        return true;
    }

    OTASCIIArmor armored;
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
        server_.userCommandProcessor_.ProcessUserCommand(request, repy);

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

    OTASCIIArmor armoredReply(serializedReply);

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
