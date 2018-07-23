// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

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
        [this](network::zeromq::Message& input) -> void {

        });

    ASSERT_NE(nullptr, &listenCallback.get());
}

TEST_F(Test_SubscribeSocket, ListenCallback_Process)
{
    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](network::zeromq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();
            EXPECT_EQ(testMessage_, inputString);
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto testMessage = network::zeromq::Message::Factory(testMessage_);

    ASSERT_NE(nullptr, &testMessage.get());

    listenCallback->Process(testMessage);
}

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_SubscribeSocket::context_.get());

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](network::zeromq::Message& input) -> OTZMQMessage {
            return network::zeromq::Message::Factory();
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_SubscribeSocket::context_, listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
