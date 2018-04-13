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

#include <gtest/gtest.h>
#include <string>
#include <thread>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"

#include "opentxs/Forward.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"

using namespace opentxs;

namespace
{

class Test_RequestReply : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};
    const std::string endpoint_{"inproc://opentxs/text/request_reply_test"};

    void requestSocketThread(const std::string& msg);
};

OTZMQContext Test_RequestReply::context_{network::zeromq::Context::Factory()};

void Test_RequestReply::requestSocketThread(const std::string& msg)
{
    ASSERT_NE(&Test_RequestReply::context_.get(), nullptr);

    auto requestSocket =
        network::zeromq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(&requestSocket.get(), nullptr);
    ASSERT_EQ(requestSocket->Type(), SocketType::Request);

    requestSocket->SetTimeouts(0, -1, 10000);
    requestSocket->Start(endpoint_);

    auto[result, message] = requestSocket->SendRequest(msg);

    ASSERT_EQ(result, SendResult::VALID_REPLY);
    const std::string& messageString = message.get();
    ASSERT_EQ(messageString, msg);
}

}  // namespace

TEST_F(Test_RequestReply, Request_Reply)
{
    ASSERT_NE(&Test_RequestReply::context_.get(), nullptr);

    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            const std::string& inputString = input;
            EXPECT_EQ(inputString, testMessage_);

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&replyCallback.get(), nullptr);

    auto replySocket = network::zeromq::ReplySocket::Factory(
        Test_RequestReply::context_, replyCallback);

    ASSERT_NE(&replySocket.get(), nullptr);
    ASSERT_EQ(replySocket->Type(), SocketType::Reply);

    replySocket->SetTimeouts(0, 10000, -1);
    replySocket->Start(endpoint_);

    auto requestSocket =
        network::zeromq::RequestSocket::Factory(Test_RequestReply::context_);

    ASSERT_NE(&requestSocket.get(), nullptr);
    ASSERT_EQ(requestSocket->Type(), SocketType::Request);

    requestSocket->SetTimeouts(0, -1, 10000);
    requestSocket->Start(endpoint_);

    auto[result, message] = requestSocket->SendRequest(testMessage_);

    ASSERT_EQ(result, SendResult::VALID_REPLY);
    const std::string& messageString = message.get();
    ASSERT_EQ(messageString, testMessage_);
}

TEST_F(Test_RequestReply, Request_Reply_2_Threads)
{
    ASSERT_NE(&Test_RequestReply::context_.get(), nullptr);

    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            const std::string& inputString = input;
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&replyCallback.get(), nullptr);

    auto replySocket = network::zeromq::ReplySocket::Factory(
        Test_RequestReply::context_, replyCallback);

    ASSERT_NE(&replySocket.get(), nullptr);
    ASSERT_EQ(replySocket->Type(), SocketType::Reply);

    replySocket->SetTimeouts(0, 10000, -1);
    replySocket->Start(endpoint_);

    std::thread requestSocketThread1(
        &Test_RequestReply::requestSocketThread, this, testMessage2_);
    std::thread requestSocketThread2(
        &Test_RequestReply::requestSocketThread, this, testMessage3_);

    requestSocketThread1.join();
    requestSocketThread2.join();
}
