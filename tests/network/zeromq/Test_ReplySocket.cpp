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
class Test_ReplySocket : public ::testing::Test
{
public:
    static OTZMQContext context_;
};

OTZMQContext Test_ReplySocket::context_{zmq::Context::Factory()};

}  // namespace

TEST(ReplySocket, ReplySocket_Factory)
{
    ASSERT_NE(nullptr, &Test_ReplySocket::context_.get());

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            return zmq::Message::Factory();
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_ReplySocket::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());
}

// TODO: Add tests for other public member functions: SetPrivateKey
