// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

using namespace opentxs;

namespace zmq = opentxs::network::zeromq;

namespace
{
class Test_SubscribeSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    const std::string testMessage_{"zeromq test message"};

    Test_SubscribeSocket()
        : context_(Context().ZMQ())
    {
    }
};
}  // namespace

TEST_F(Test_SubscribeSocket, SubscribeSocket_Factory)
{
    auto subscribeSocket =
        context_.SubscribeSocket(network::zeromq::ListenCallback::Factory());

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());
}

// TODO: Add tests for other public member functions: SetPublicKey,
// SetSocksProxy
