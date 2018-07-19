// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_PublishSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;
};

OTZMQContext Test_PublishSocket::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST(PublishSocket, PublishSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_PublishSocket::context_.get());

    auto publishSocket =
        network::zeromq::PublishSocket::Factory(Test_PublishSocket::context_);

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());
}

// TODO: Add tests for other public member functions: SetCurve
