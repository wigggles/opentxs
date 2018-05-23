/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include <gtest/gtest.h>
#include <string>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"

#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/MultipartMessage.hpp"

#include <zmq.h>

using namespace opentxs;

namespace
{
class Test_Message : public ::testing::Test
{
public:
    static OTZMQContext context_;
};

OTZMQContext Test_Message::context_{network::zeromq::Context::Factory()};

}  // namespace

TEST(Message, Factory)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    OTZMQMessage message = network::zeromq::Message::Factory();

    ASSERT_NE(nullptr, &message.get());
}

TEST(Message, Factory2)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    auto data = Data::Factory("0", 1);

    OTZMQMessage message = network::zeromq::Message::Factory(data);

    ASSERT_NE(nullptr, &message.get());
    ASSERT_EQ(message->size(), data->GetSize());
}

TEST(Message, Factory3)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    OTZMQMessage message = network::zeromq::Message::Factory("testString");

    ASSERT_NE(nullptr, &message.get());
    std::string messageString = message.get();
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Message, operator_string)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    auto message =
        network::zeromq::Message::Factory(Data::Factory("testString", 10));

    ASSERT_NE(nullptr, &message.get());
    std::string messageString = message.get();
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Message, data)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    auto message = network::zeromq::Message::Factory();

    const void* data = message->data();
    ASSERT_NE(nullptr, data);

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_EQ(data, zmq_msg_data(zmq_msg));
}

TEST(Message, size)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    auto message = network::zeromq::Message::Factory();

    std::size_t size = message->size();
    ASSERT_EQ(0, size);

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_EQ(0, zmq_msg_size(zmq_msg));

    message =
        network::zeromq::Message::Factory(Data::Factory("testString", 10));
    size = message->size();
    ASSERT_EQ(10, size);

    zmq_msg = message.get();
    ASSERT_EQ(10, zmq_msg_size(zmq_msg));
}

TEST(Message, zmq_msg_t)
{
    ASSERT_NE(nullptr, &Test_Message::context_.get());

    auto message = network::zeromq::Message::Factory();

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_NE(nullptr, zmq_msg);
}
