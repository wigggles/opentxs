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
#include "opentxs/network/zeromq/FrameIterator.hpp"

#include <zmq.h>

using namespace opentxs;

namespace
{
class Test_MultipartMessage : public ::testing::Test
{
public:
    static OTZMQContext context_;
};

OTZMQContext Test_MultipartMessage::context_{
    network::zeromq::Context::Factory()};

}  // namespace

TEST(MultipartMessage, Factory)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    ASSERT_NE(nullptr, &multipartMessage.get());
}

TEST(MultipartMessage, at)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    auto& message = multipartMessage->at(0);
    std::string messageString = message;
    ASSERT_STREQ("msg1", messageString.c_str());

    auto& message2 = multipartMessage->at(1);
    messageString = message2;
    ASSERT_STREQ("msg2", messageString.c_str());

    auto& message3 = multipartMessage->at(2);
    messageString = message3;
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(MultipartMessage, begin)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::FrameIterator it = multipartMessage->begin();
    ASSERT_EQ(multipartMessage->end(), it);
    ASSERT_EQ(0, std::distance(it, multipartMessage->end()));

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    ASSERT_NE(multipartMessage->end(), it);
    ASSERT_EQ(3, std::distance(it, multipartMessage->end()));

    std::string messageString = *it;
    ASSERT_STREQ("msg1", messageString.c_str());

    std::advance(it, 3);
    ASSERT_EQ(multipartMessage->end(), it);
    ASSERT_EQ(0, std::distance(it, multipartMessage->end()));
}

TEST(MultipartMessage, end)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::FrameIterator it = multipartMessage->end();
    ASSERT_EQ(multipartMessage->begin(), it);
    ASSERT_EQ(0, std::distance(multipartMessage->begin(), it));

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    auto it2 = multipartMessage->end();
    ASSERT_NE(multipartMessage->begin(), it2);
    ASSERT_EQ(3, std::distance(multipartMessage->begin(), it2));
}

TEST(MultipartMessage, size)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    ASSERT_EQ(0, multipartMessage->size());

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    ASSERT_EQ(3, multipartMessage->size());
}

TEST(MultipartMessage, AddFrame)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    auto& message = multipartMessage->AddFrame();
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(0, message.size());
}

TEST(MultipartMessage, AddFrame_Data)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    auto& message = multipartMessage->AddFrame(Data::Factory("testString", 10));
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(10, message.size());
    
    std::string messageString = message;
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(MultipartMessage, AddFrame_string)
{
    ASSERT_NE(nullptr, &Test_MultipartMessage::context_.get());

    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    auto& message = multipartMessage->AddFrame("testString");
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(10, message.size());
    
    std::string messageString = message;
    ASSERT_STREQ("testString", messageString.c_str());
}
