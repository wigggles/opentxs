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

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"

#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"

using namespace opentxs;

namespace
{
class Test_ReplySocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_ReplySocket::context_{network::zeromq::Context::Factory()};

} // namespace

TEST(ReplySocket, ReplyCallback_Factory)
{
    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&replyCallback.get(), nullptr);
}

TEST_F(Test_ReplySocket, ReplyCallback_Process)
{
    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            const std::string& inputString = input;
            EXPECT_EQ(inputString, testMessage_);

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&replyCallback.get(), nullptr);

    auto testMessage = network::zeromq::Message::Factory(testMessage_);

    ASSERT_NE(&testMessage.get(), nullptr);

    auto message = replyCallback->Process(testMessage);

    const std::string& messageString = message.get();
    ASSERT_EQ(testMessage_, messageString);
}

TEST(ReplySocket, ReplySocket_Factory)
{
	ASSERT_NE(&Test_ReplySocket::context_.get(), nullptr);
	
    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&replyCallback.get(), nullptr);

    auto replySocket = network::zeromq::ReplySocket::Factory(
    		Test_ReplySocket::context_, replyCallback);

    ASSERT_NE(&replySocket.get(), nullptr);
    ASSERT_EQ(replySocket->Type(), SocketType::Reply);
}

// TODO: Add tests for other public member functions: SetCurve

