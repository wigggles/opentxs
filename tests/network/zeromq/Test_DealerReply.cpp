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

class Test_DealerReply : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};
    const std::string testMessage3_{"zeromq test message 3"};

    const std::string endpoint_{"inproc://opentxs/test/dealer_reply_test"};

    void dealerSocketThread(const std::string& msg);
};

OTZMQContext Test_DealerReply::context_{zmq::Context::Factory()};

void Test_DealerReply::dealerSocketThread(const std::string& msg)
{
    ASSERT_NE(nullptr, &Test_DealerReply::context_.get());

    bool replyProcessed{false};

    auto listenCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Connect,
        listenCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto message = zmq::Message::Factory(msg);
    message->PrependEmptyFrame();
    auto sent = dealerSocket->Send(message);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

}  // namespace

TEST_F(Test_DealerReply, Dealer_Reply)
{
    ASSERT_NE(nullptr, &Test_DealerReply::context_.get());

    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this, &replyReturned](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            auto reply = zmq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            const std::string& inputString = *input.Body().begin();

            EXPECT_EQ(testMessage_, inputString);

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Connect,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto message = zmq::Message::Factory(testMessage_);
    message->PrependEmptyFrame();
    auto sent = dealerSocket->Send(message);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}

TEST_F(Test_DealerReply, Dealer_2_Reply_1)
{
    ASSERT_NE(nullptr, &Test_DealerReply::context_.get());

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this](const zmq::Message& input) -> OTZMQMessage {
            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage2_ || inputString == testMessage3_;
            EXPECT_TRUE(match);

            auto reply = zmq::Message::ReplyFactory(input);
            reply->AddFrame(inputString);
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    std::thread dealerSocketThread1(
        &Test_DealerReply::dealerSocketThread, this, testMessage2_);
    std::thread dealerSocketThread2(
        &Test_DealerReply::dealerSocketThread, this, testMessage3_);

    dealerSocketThread1.join();
    dealerSocketThread2.join();
}

TEST_F(Test_DealerReply, Dealer_Reply_Multipart)
{
    ASSERT_NE(nullptr, &Test_DealerReply::context_.get());

    bool replyReturned{false};

    auto replyCallback = zmq::ReplyCallback::Factory(
        [this, &replyReturned](const zmq::Message& input) -> OTZMQMessage {
            // ReplySocket removes the delimiter frame.
            EXPECT_EQ(4, input.size());
            EXPECT_EQ(1, input.Header().size());
            EXPECT_EQ(2, input.Body().size());

            for (const std::string& frame : input.Header()) {
                EXPECT_EQ(testMessage_, frame);
            }

            for (const std::string& frame : input.Body()) {
                bool match = frame == testMessage2_ || frame == testMessage3_;

                EXPECT_TRUE(match);
            }

            auto reply = zmq::Message::ReplyFactory(input);
            for (const auto& frame : input.Body()) { reply->AddFrame(frame); }
            replyReturned = true;
            return reply;
        });

    ASSERT_NE(nullptr, &replyCallback.get());

    auto replySocket = zmq::ReplySocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Bind,
        replyCallback);

    ASSERT_NE(nullptr, &replySocket.get());
    ASSERT_EQ(SocketType::Reply, replySocket->Type());

    replySocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    replySocket->Start(endpoint_);

    bool replyProcessed{false};

    auto dealerCallback = zmq::ListenCallback::Factory(
        [this, &replyProcessed](zmq::Message& input) -> void {
            // ReplySocket puts the delimiter frame back when it sends the
            // reply.
            ASSERT_EQ(5, input.size());
            ASSERT_EQ(0, input.Header().size());
            ASSERT_EQ(4, input.Body().size());

            const std::string& header = input.at(1);

            ASSERT_EQ(testMessage_, header);

            for (auto i = 3; i < input.size(); ++i) {
                const std::string& frame = input.at(i);
                bool match = frame == testMessage2_ || frame == testMessage3_;

                EXPECT_TRUE(match);
            }

            replyProcessed = true;
        });

    ASSERT_NE(nullptr, &dealerCallback.get());

    auto dealerSocket = zmq::DealerSocket::Factory(
        Test_DealerReply::context_,
        zmq::Socket::Direction::Connect,
        dealerCallback);

    ASSERT_NE(nullptr, &dealerSocket.get());
    ASSERT_EQ(SocketType::Dealer, dealerSocket->Type());

    dealerSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    dealerSocket->Start(endpoint_);

    auto multipartMessage = zmq::Message::Factory(testMessage_);
    multipartMessage->AddFrame();
    multipartMessage->AddFrame(testMessage2_);
    multipartMessage->AddFrame(testMessage3_);
    // Prepend a delimiter frame for the ReplySocket.
    multipartMessage->PrependEmptyFrame();

    auto sent = dealerSocket->Send(multipartMessage);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 5;
    while (!replyReturned && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyReturned);

    end = std::time(nullptr) + 5;
    while (!replyProcessed && std::time(nullptr) < end) {
        Log::Sleep(std::chrono::milliseconds(100));
    }

    EXPECT_TRUE(replyProcessed);
}
