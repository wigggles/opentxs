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
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

using namespace opentxs;

namespace
{
class Test_SubscribeSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_SubscribeSocket::context_{
    network::zeromq::Context::Factory()};

}  // namespace

TEST(SubscribeSocket, ListenCallback_Factory)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> void {

        });

    ASSERT_NE(&listenCallback.get(), nullptr);
}

TEST_F(Test_SubscribeSocket, ListenCallback_Process)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> void {

            const std::string& inputString = input;
            EXPECT_EQ(inputString, testMessage_);
        });

    ASSERT_NE(&listenCallback.get(), nullptr);

    auto testMessage = network::zeromq::Message::Factory(testMessage_);

    ASSERT_NE(&testMessage.get(), nullptr);

    listenCallback->Process(testMessage);
}

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            const std::string& inputString = input;
            EXPECT_EQ(inputString, testMessage_);

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(&listenCallback.get(), nullptr);

    ASSERT_NE(&Test_SubscribeSocket::context_.get(), nullptr);

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_SubscribeSocket::context_, listenCallback);

    ASSERT_NE(&subscribeSocket.get(), nullptr);
    ASSERT_EQ(subscribeSocket->Type(), SocketType::Subscribe);
}

// TODO: Add tests for other public member functions: SetCurve, SetSocksProxy
