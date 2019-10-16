// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

using namespace opentxs;

namespace zmq = opentxs::network::zeromq;

namespace
{
class Test_RouterSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_RouterSocket()
        : context_(Context().ZMQ())
    {
    }
};
}  // namespace

TEST_F(Test_RouterSocket, RouterSocket_Factory)
{
    auto dealerSocket = context_.RouterSocket(
        zmq::ListenCallback::Factory(),
        zmq::socket::Socket::Direction::Connect);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Router, dealerSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
