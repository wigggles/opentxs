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
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/MultipartMessage.hpp"

#include <zmq.h>

using namespace opentxs;

TEST(MultipartMessage, Factory)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    ASSERT_NE(nullptr, &multipartMessage.get());
}

TEST(MultipartMessage, AddFrame)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::Message& message = multipartMessage->AddFrame();
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(0, message.size());
}

TEST(MultipartMessage, AddFrame_Data)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::Message& message =
        multipartMessage->AddFrame(Data::Factory("testString", 10));
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(10, message.size());

    std::string messageString = message;
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(MultipartMessage, AddFrame_string)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::Message& message =
        multipartMessage->AddFrame("testString");
    ASSERT_EQ(1, multipartMessage->size());
    ASSERT_NE(nullptr, message.data());
    ASSERT_EQ(10, message.size());

    std::string messageString = message;
    ASSERT_STREQ("testString", messageString.c_str());
}

TEST(MultipartMessage, at)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    network::zeromq::Message& message = multipartMessage->at(0);
    std::string messageString = message;
    ASSERT_STREQ("msg1", messageString.c_str());

    network::zeromq::Message& message2 = multipartMessage->at(1);
    messageString = message2;
    ASSERT_STREQ("msg2", messageString.c_str());

    network::zeromq::Message& message3 = multipartMessage->at(2);
    messageString = message3;
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(MultipartMessage, at_const)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    const network::zeromq::Message& message = multipartMessage->at(0);
    std::string messageString = message;
    ASSERT_STREQ("msg1", messageString.c_str());

    const network::zeromq::Message& message2 = multipartMessage->at(1);
    messageString = message2;
    ASSERT_STREQ("msg2", messageString.c_str());

    const network::zeromq::Message& message3 = multipartMessage->at(2);
    messageString = message3;
    ASSERT_STREQ("msg3", messageString.c_str());
}

TEST(MultipartMessage, begin)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::FrameIterator it = multipartMessage->begin();
    ASSERT_EQ(multipartMessage->end(), it);
    ASSERT_EQ(0, std::distance(it, multipartMessage->end()));

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    ASSERT_NE(multipartMessage->end(), it);
    ASSERT_EQ(3, std::distance(it, multipartMessage->end()));

    std::advance(it, 3);
    ASSERT_EQ(multipartMessage->end(), it);
    ASSERT_EQ(0, std::distance(it, multipartMessage->end()));
}

TEST(MultipartMessage, Body)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    const network::zeromq::FrameSection bodySection = multipartMessage->Body();
    ASSERT_EQ(2, bodySection.size());

    const auto& message = bodySection.at(1);
    std::string msgString = message;
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(MultipartMessage, Body_at)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    const network::zeromq::Message& message = multipartMessage->Body_at(1);
    std::string msgString = message;
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(MultipartMessage, Body_begin)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    network::zeromq::FrameIterator bodyBegin = multipartMessage->Body_begin();
    auto body = multipartMessage->Body();
    ASSERT_EQ(body.begin(), bodyBegin);
}

TEST(MultipartMessage, Body_end)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    network::zeromq::FrameIterator bodyEnd = multipartMessage->Body_end();
    auto body = multipartMessage->Body();
    ASSERT_EQ(body.end(), bodyEnd);
}

TEST(MultipartMessage, end)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    network::zeromq::FrameIterator it = multipartMessage->end();
    ASSERT_EQ(multipartMessage->begin(), it);
    ASSERT_EQ(0, std::distance(multipartMessage->begin(), it));

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    network::zeromq::FrameIterator it2 = multipartMessage->end();
    ASSERT_NE(multipartMessage->begin(), it2);
    ASSERT_EQ(3, std::distance(multipartMessage->begin(), it2));
}

TEST(MultipartMessage, Header)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    network::zeromq::FrameSection headerSection = multipartMessage->Header();
    ASSERT_EQ(2, headerSection.size());

    const auto& message = headerSection.at(1);
    std::string msgString = message;
    ASSERT_STREQ("msg2", msgString.c_str());
}

TEST(MultipartMessage, Header_at)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    const network::zeromq::Message& message = multipartMessage->Header_at(1);
    std::string msgString = message;
    ASSERT_STREQ("msg2", msgString.c_str());
}

TEST(MultipartMessage, Header_begin)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    network::zeromq::FrameIterator headerBegin =
        multipartMessage->Header_begin();
    auto header = multipartMessage->Header();
    ASSERT_EQ(header.begin(), headerBegin);
}

TEST(MultipartMessage, Header_end)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame();
    multipartMessage->AddFrame("msg3");
    multipartMessage->AddFrame("msg4");

    network::zeromq::FrameIterator headerEnd = multipartMessage->Header_end();
    auto header = multipartMessage->Header();
    ASSERT_EQ(header.end(), headerEnd);
}

TEST(MultipartMessage, size)
{
    auto multipartMessage = network::zeromq::MultipartMessage::Factory();

    std::size_t size = multipartMessage->size();
    ASSERT_EQ(0, size);

    multipartMessage->AddFrame("msg1");
    multipartMessage->AddFrame("msg2");
    multipartMessage->AddFrame("msg3");

    size = multipartMessage->size();
    ASSERT_EQ(3, size);
}
