// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace zmq = opentxs::network::zeromq;

namespace
{
class Test_RouterSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

    //    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_RouterSocket::context_{zmq::Context::Factory()};

}  // namespace

TEST(RouterSocket, RouterSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_RouterSocket::context_.get());

    auto dealerSocket = zmq::RouterSocket::Factory(
        Test_RouterSocket::context_,
        zmq::Socket::Direction::Connect,
        zmq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Router, dealerSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
