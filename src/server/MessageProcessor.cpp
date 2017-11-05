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

#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/ZMQ.hpp"
#include "opentxs/server/Server.hpp"
#include "opentxs/server/UserCommandProcessor.hpp"

#include <stddef.h>
#include <sys/types.h>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::MessageProcessor::"

namespace opentxs::server
{

MessageProcessor::MessageProcessor(Server& server, std::atomic<bool>& shutdown)
    : server_(server)
    , shutdown_(shutdown)
    , zmqSocket_(zsock_new_rep(NULL))
    , zmqAuth_(zactor_new(zauth, NULL))
    , zmqPoller_(zpoller_new(zmqSocket_, NULL))
    , thread_(nullptr)
{
}

void MessageProcessor::Cleanup()
{
    if (thread_) {
        thread_->join();
        thread_.reset();
    }

    zpoller_remove(zmqPoller_, zmqSocket_);
    zpoller_destroy(&zmqPoller_);
    zactor_destroy(&zmqAuth_);
    zsock_destroy(&zmqSocket_);
}

void MessageProcessor::Init(int port, zcert_t* transportKey)
{
    if (port == 0) {
        OT_FAIL;
    }
    if (!zsys_has_curve()) {
        Log::vError("Error: libzmq has no libsodium support");
        OT_FAIL;
    }
    zstr_sendx(zmqAuth_, "CURVE", CURVE_ALLOW_ANY, NULL);
    zsock_wait(zmqAuth_);
    zsock_set_zap_domain(zmqSocket_, "global");
    zsock_set_curve_server(zmqSocket_, 1);
    zcert_apply(transportKey, zmqSocket_);
    zcert_destroy(&transportKey);
    zsock_bind(zmqSocket_, "tcp://*:%d", port);
}

void MessageProcessor::run()
{
    while (false == shutdown_.load()) {
        // timeout is the time left until the next cron should execute.
        const auto timeout = server_.computeTimeout();

        if (timeout <= 0) {
            server_.ProcessCron();

            continue;
        }

        // wait for incoming message or up to timeout,
        // i.e. stop polling in time for the next cron execution.
        if (zpoller_wait(zmqPoller_, timeout)) {
            processSocket();

            continue;
        }

        if (zpoller_terminated(zmqPoller_)) {
            otErr << __FUNCTION__
                  << ": zpoller_terminated - process interrupted or"
                  << " parent context destroyed\n";
            shutdown_.store(true);

            continue;
        }

        if (false == zpoller_expired(zmqPoller_)) {
            otErr << __FUNCTION__ << ": zpoller_wait error\n";
        }

        Log::Sleep(std::chrono::milliseconds(100));
    }
}

void MessageProcessor::processSocket()
{
    char* msg = zstr_recv(zmqSocket_);
    if (msg == nullptr) {
        Log::Error("zeromq recv() failed\n");
        return;
    }
    std::string requestString(msg);
    zstr_free(&msg);

    std::string responseString;

    bool error = processMessage(requestString, responseString);

    if (error) {
        responseString = "";
    }

    int rc = zstr_send(zmqSocket_, responseString.c_str());

    if (rc != 0) {
        Log::vError(
            "MessageProcessor: failed to send response\n"
            "request:\n%s\n\n"
            "response:\n%s\n\n",
            requestString.c_str(),
            responseString.c_str());
    }
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
