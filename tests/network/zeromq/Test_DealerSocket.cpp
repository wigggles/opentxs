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
class Test_DealerSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

    //    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_DealerSocket::context_{zmq::Context::Factory()};

}  // namespace

TEST(DealerSocket, DealerSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_DealerSocket::context_.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_DealerSocket::context_,
        zmq::Socket::Direction::Connect,
        zmq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
