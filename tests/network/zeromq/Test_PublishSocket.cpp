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
class Test_PublishSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    Test_PublishSocket()
        : context_(Context().ZMQ())
    {
    }
};
}  // namespace

TEST_F(Test_PublishSocket, PublishSocket_Factory)
{
    auto publishSocket = context_.PublishSocket();

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());
}

// TODO: Add tests for other public member functions: SetPrivateKey
