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

    ASSERT_NE(nullptr, &listenCallback.get());
}

TEST_F(Test_SubscribeSocket, ListenCallback_Process)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> void {

            const std::string& inputString = input;
            EXPECT_EQ(testMessage_, inputString);
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto testMessage = network::zeromq::Message::Factory(testMessage_);

    ASSERT_NE(nullptr, &testMessage.get());

    listenCallback->Process(testMessage);
}

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {

            const std::string& inputString = input;
            EXPECT_EQ(testMessage_, inputString);

            return network::zeromq::Message::Factory(input);
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    ASSERT_NE(nullptr, &Test_SubscribeSocket::context_.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_SubscribeSocket::context_, listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());
}

// TODO: Add tests for other public member functions: SetCurve, SetSocksProxy
