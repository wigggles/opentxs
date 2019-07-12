// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

TEST(FrameSection, constructors)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    network::zeromq::FrameSection frameSection(multipartMessage->Header());
    ASSERT_EQ(0, frameSection.size());

    network::zeromq::FrameSection frameSection2(&multipartMessage.get(), 0, 0);
    ASSERT_EQ(0, frameSection2.size());

    multipartMessage->AddFrame(std::string("msg1"));
    multipartMessage->AddFrame(std::string{"msg2"});

    network::zeromq::FrameSection frameSection3(multipartMessage->Body());
    ASSERT_EQ(2, frameSection3.size());

    network::zeromq::FrameSection frameSection4(&multipartMessage.get(), 0, 2);
    ASSERT_EQ(2, frameSection4.size());

    multipartMessage->AddFrame();
    multipartMessage->AddFrame(std::string{"msg3"});

    network::zeromq::FrameSection frameSection5(multipartMessage->Header());
    ASSERT_EQ(2, frameSection5.size());

    network::zeromq::FrameSection frameSection6(multipartMessage->Body());
    ASSERT_EQ(1, frameSection6.size());

    network::zeromq::FrameSection frameSection7(&multipartMessage.get(), 3, 1);
    ASSERT_EQ(1, frameSection7.size());
}

TEST(FrameSection, at)
{
    auto multipartMessage = network::zeromq::Message::Factory();
    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});
    multipartMessage->AddFrame();

    auto header = multipartMessage->Header();

    const network::zeromq::Frame& header1 = header.at(0);
    std::string msgString = header1;
    ASSERT_STREQ("msg1", msgString.c_str());

    const network::zeromq::Frame& header2 = header.at(1);
    msgString = header2;
    ASSERT_STREQ("msg2", msgString.c_str());

    multipartMessage->AddFrame(std::string{"msg3"});
    multipartMessage->AddFrame(std::string{"msg4"});

    auto body = multipartMessage->Body();

    const network::zeromq::Frame& body1 = body.at(0);
    msgString = body1;
    ASSERT_STREQ("msg3", msgString.c_str());

    const network::zeromq::Frame& body2 = body.at(1);
    msgString = body2;
    ASSERT_STREQ("msg4", msgString.c_str());
}

TEST(FrameSection, begin)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    auto headerSection = multipartMessage->Header();
    network::zeromq::FrameIterator headerIterator = headerSection.begin();
    ASSERT_EQ(headerSection.end(), headerIterator);
    ASSERT_EQ(0, std::distance(headerIterator, headerSection.end()));

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});
    multipartMessage->AddFrame();

    auto headerSection2 = multipartMessage->Header();
    network::zeromq::FrameIterator headerIterator2 = headerSection.begin();
    ASSERT_NE(headerSection2.end(), headerIterator2);
    ASSERT_EQ(2, std::distance(headerIterator2, headerSection2.end()));

    std::advance(headerIterator2, 2);
    ASSERT_EQ(headerSection2.end(), headerIterator2);
    ASSERT_EQ(0, std::distance(headerIterator2, headerSection2.end()));

    auto bodySection = multipartMessage->Body();
    network::zeromq::FrameIterator bodyIterator = bodySection.begin();
    ASSERT_EQ(bodySection.end(), bodyIterator);
    ASSERT_EQ(0, std::distance(bodyIterator, bodySection.end()));

    multipartMessage->AddFrame(std::string{"msg3"});
    multipartMessage->AddFrame(std::string{"msg4"});

    auto bodySection2 = multipartMessage->Header();
    network::zeromq::FrameIterator bodyIterator2 = headerSection.begin();
    ASSERT_NE(bodySection2.end(), bodyIterator2);
    ASSERT_EQ(2, std::distance(bodyIterator2, bodySection2.end()));

    std::advance(bodyIterator2, 2);
    ASSERT_EQ(bodySection2.end(), bodyIterator2);
    ASSERT_EQ(0, std::distance(bodyIterator2, bodySection2.end()));
}

TEST(FrameSection, end)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    auto headerSection = multipartMessage->Header();
    network::zeromq::FrameIterator headerIterator = headerSection.end();
    ASSERT_EQ(headerSection.begin(), headerIterator);
    ASSERT_EQ(0, std::distance(headerSection.begin(), headerIterator));

    auto bodySection = multipartMessage->Body();
    network::zeromq::FrameIterator bodyIterator = bodySection.end();
    ASSERT_EQ(bodySection.begin(), bodyIterator);
    ASSERT_EQ(0, std::distance(bodySection.begin(), bodyIterator));

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});
    multipartMessage->AddFrame();

    auto headerSection2 = multipartMessage->Header();
    network::zeromq::FrameIterator headerIterator2 = headerSection2.end();
    ASSERT_NE(headerSection2.begin(), headerIterator2);
    ASSERT_EQ(2, std::distance(headerSection2.begin(), headerIterator2));

    auto bodySection2 = multipartMessage->Body();
    network::zeromq::FrameIterator bodyIterator2 = bodySection2.end();
    ASSERT_EQ(bodySection2.begin(), bodyIterator2);
    ASSERT_EQ(0, std::distance(bodySection2.begin(), bodyIterator2));

    multipartMessage->AddFrame(std::string{"msg3"});
    multipartMessage->AddFrame(std::string{"msg4"});

    auto bodySection3 = multipartMessage->Body();
    network::zeromq::FrameIterator bodyIterator3 = bodySection3.end();
    ASSERT_NE(bodySection3.begin(), bodyIterator3);
    ASSERT_EQ(2, std::distance(bodySection3.begin(), bodyIterator3));
}

TEST(FrameSection, size)
{
    auto multipartMessage = network::zeromq::Message::Factory();

    auto headerSection = multipartMessage->Header();
    std::size_t size = headerSection.size();
    ASSERT_EQ(0, size);

    auto bodySection = multipartMessage->Body();
    size = bodySection.size();
    ASSERT_EQ(0, size);

    multipartMessage->AddFrame(std::string{"msg1"});
    multipartMessage->AddFrame(std::string{"msg2"});
    multipartMessage->AddFrame();

    auto headerSection2 = multipartMessage->Header();
    size = headerSection2.size();
    ASSERT_EQ(2, size);

    auto bodySection2 = multipartMessage->Body();
    size = bodySection2.size();
    ASSERT_EQ(0, size);

    multipartMessage->AddFrame(std::string{"msg3"});
    multipartMessage->AddFrame(std::string{"msg4"});

    auto bodySection3 = multipartMessage->Body();
    size = bodySection3.size();
    ASSERT_EQ(2, size);
}
