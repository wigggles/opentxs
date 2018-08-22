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

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_SubscribeSocket::context_.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_SubscribeSocket::context_,
        network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
