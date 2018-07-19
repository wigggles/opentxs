// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_RequestSocket : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
};

OTZMQContext Test_RequestSocket::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST(RequestSocket, RequestSocket_Factory)
{
    ASSERT_NE(nullptr, &Test_RequestSocket::context_.get());

    auto requestSocket =
        network::zeromq::RequestSocket::Factory(Test_RequestSocket::context_);

    ASSERT_NE(nullptr, &requestSocket.get());
    ASSERT_EQ(SocketType::Request, requestSocket->Type());
}

// TODO: Add tests for other public member functions: SetCurve, SetSocksProxy
