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
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

using namespace opentxs;

TEST(Frame, Factory)
{
    OTZMQFrame message = network::zeromq::Frame::Factory();

    ASSERT_NE(nullptr, &message.get());
}

TEST(Frame, Factory2)
{
    auto data = Data::Factory("0", 1);

    OTZMQFrame message = network::zeromq::Frame::Factory(data);

    ASSERT_NE(nullptr, &message.get());
    ASSERT_EQ(message->size(), data->GetSize());
}

TEST(Frame, Factory3)
{
    OTZMQFrame message = network::zeromq::Frame::Factory("testString");

    ASSERT_NE(nullptr, &message.get());
    std::string messageString = message.get();
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Frame, operator_string)
{
    auto message =
        network::zeromq::Frame::Factory(Data::Factory("testString", 10));

    ASSERT_NE(nullptr, &message.get());
    std::string messageString = message.get();
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(Frame, data)
{
    auto message = network::zeromq::Frame::Factory();

    const void* data = message->data();
    ASSERT_NE(nullptr, data);

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_EQ(data, zmq_msg_data(zmq_msg));
}

TEST(Frame, size)
{
    auto message = network::zeromq::Frame::Factory();

    std::size_t size = message->size();
    ASSERT_EQ(0, size);

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_EQ(0, zmq_msg_size(zmq_msg));

    message =
        network::zeromq::Frame::Factory(Data::Factory("testString", 10));
    size = message->size();
    ASSERT_EQ(10, size);

    zmq_msg = message.get();
    ASSERT_EQ(10, zmq_msg_size(zmq_msg));
}

TEST(Frame, zmq_msg_t)
{
    auto message = network::zeromq::Frame::Factory();

    zmq_msg_t* zmq_msg = message.get();
    ASSERT_NE(nullptr, zmq_msg);
}
