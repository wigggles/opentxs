// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_ReplySocket : public ::testing::Test
{
public:
    static OTZMQContext context_;
};

OTZMQContext Test_ReplySocket::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST(ReplySocket, ReplySocket_Factory)
{
    ASSERT_NE(nullptr, &Test_ReplySocket::context_.get());

    auto replyCallback = network::zeromq::ReplyCallback::Factory(
        [this](const network::zeromq::Message& input) -> OTZMQMessage {
            return network::zeromq::Message::Factory();
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = network::zeromq::ReplySocket::Factory(
        Test_ReplySocket::context_, false, replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());
}

// TODO: Add tests for other public member functions: SetPrivateKey
