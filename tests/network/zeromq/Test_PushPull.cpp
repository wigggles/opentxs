// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_PushPull : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/push_pull_test"};
};

OTZMQContext Test_PushPull::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST_F(Test_PushPull, Push_Pull)
{
    ASSERT_NE(nullptr, &Test_PushPull::context_.get());

    bool callbackFinished{false};
    
    auto pullCallback = network::zeromq::ListenCallback::Factory(
        [this, &callbackFinished](network::zeromq::Message& input) -> void {
            EXPECT_EQ(1, input.size());
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);
            
            callbackFinished = true;
        });

    ASSERT_NE(nullptr, &pullCallback.get());

    auto pullSocket = network::zeromq::PullSocket::Factory(
        Test_PushPull::context_, false, pullCallback);

    ASSERT_NE(nullptr, &pullSocket.get());
    ASSERT_EQ(SocketType::Pull, pullSocket->Type());

    pullSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    pullSocket->Start(endpoint_);

    auto pushSocket = network::zeromq::PushSocket::Factory(
        Test_PushPull::context_, true);

    ASSERT_NE(nullptr, &pushSocket.get());
    ASSERT_EQ(SocketType::Push, pushSocket->Type());

    pushSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    pushSocket->Start(endpoint_);

    auto sent = pushSocket->Push(testMessage_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 15;
    while (!callbackFinished && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }
    
    ASSERT_TRUE(callbackFinished);
}
