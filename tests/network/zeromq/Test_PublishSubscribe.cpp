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
#include <set>
#include <thread>
#include <atomic>
#include <chrono>

#include "gtest/gtest-message.h"
#include "gtest/gtest-test-part.h"

#include "opentxs/Forward.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

using namespace opentxs;

namespace
{

class Test_PublishSubscribe : public ::testing::Test
{
public:
    static OTZMQContext context_;

    const std::string testMessage_{"zeromq test message"};
    const std::string testMessage2_{"zeromq test message 2"};

    const std::string endpoint_{"inproc://opentxs/test/publish_subscribe_test"};
    const std::string endpoint2_{
        "inproc://opentxs/test/publish_subscribe_test2"};

    std::atomic_int callbackFinishedCount_{0};
    std::atomic_int subscribeThreadStartedCount_{0};
    std::atomic_int publishThreadStartedCount_{0};

    int subscribeThreadCount_{0};
    int callbackCount_{0};

    void subscribeSocketThread(
        const std::set<std::string>& endpoints,
        const std::set<std::string>& msgs);
    void publishSocketThread(
        const std::string& endpoint,
        const std::string& msg);
};

OTZMQContext Test_PublishSubscribe::context_{
    network::zeromq::Context::Factory()};

void Test_PublishSubscribe::subscribeSocketThread(
    const std::set<std::string>& endpoints,
    const std::set<std::string>& msgs)
{
    ASSERT_NE(nullptr, &Test_PublishSubscribe::context_.get());

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this, msgs](const network::zeromq::Message& input) -> void {

            const std::string& inputString = *input.Body().begin();
            bool found = msgs.count(inputString);
            EXPECT_TRUE(found);
            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_PublishSubscribe::context_, listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    for (auto endpoint : endpoints) {
        subscribeSocket->Start(endpoint);
    }

    ++subscribeThreadStartedCount_;

    auto end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);
}

void Test_PublishSubscribe::publishSocketThread(
    const std::string& endpoint,
    const std::string& msg)
{
    ASSERT_NE(nullptr, &Test_PublishSubscribe::context_.get());

    auto publishSocket = network::zeromq::PublishSocket::Factory(
        Test_PublishSubscribe::context_);

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    publishSocket->Start(endpoint);

    ++publishThreadStartedCount_;

    auto end = std::time(nullptr) + 15;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    bool sent = publishSocket->Publish(msg);

    ASSERT_TRUE(sent);

    end = std::time(nullptr) + 15;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);
}

}  // namespace

TEST_F(Test_PublishSubscribe, Publish_Subscribe)
{
    ASSERT_NE(&Test_PublishSubscribe::context_.get(), nullptr);

    auto publishSocket = network::zeromq::PublishSocket::Factory(
        Test_PublishSubscribe::context_);

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    publishSocket->Start(endpoint_);

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> void {

            const std::string& inputString = *input.Body().begin();
            EXPECT_EQ(testMessage_, inputString);
            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_PublishSubscribe::context_, listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    subscribeSocket->Start(endpoint_);

    bool sent = publishSocket->Publish(testMessage_);

    ASSERT_TRUE(sent);

    auto end = std::time(nullptr) + 30;
    while (!callbackFinishedCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(1, callbackFinishedCount_);
}

TEST_F(Test_PublishSubscribe, Publish_1_Subscribe_2)
{
    subscribeThreadCount_ = 2;
    callbackCount_ = 2;

    ASSERT_NE(nullptr, &Test_PublishSubscribe::context_.get());

    auto publishSocket = network::zeromq::PublishSocket::Factory(
        Test_PublishSubscribe::context_);

    ASSERT_NE(nullptr, &publishSocket.get());
    ASSERT_EQ(SocketType::Publish, publishSocket->Type());

    publishSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(30000),
        std::chrono::milliseconds(-1));
    publishSocket->Start(endpoint_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        std::set<std::string>({endpoint_}),
        std::set<std::string>({testMessage_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        std::set<std::string>({endpoint_}),
        std::set<std::string>({testMessage_}));

    auto end = std::time(nullptr) + 30;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(subscribeThreadCount_, subscribeThreadStartedCount_);

    bool sent = publishSocket->Publish(testMessage_);

    ASSERT_TRUE(sent);

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_1)
{
    subscribeThreadCount_ = 1;
    callbackCount_ = 2;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        testMessage_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        testMessage2_);

    auto end = std::time(nullptr) + 15;
    while (publishThreadStartedCount_ < 2 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(2, publishThreadStartedCount_);

    auto listenCallback = network::zeromq::ListenCallback::Factory(
        [this](const network::zeromq::Message& input) -> void {

            const std::string& inputString = *input.Body().begin();
            bool match =
                inputString == testMessage_ || inputString == testMessage2_;
            EXPECT_TRUE(match);
            ++callbackFinishedCount_;
        });

    ASSERT_NE(nullptr, &listenCallback.get());

    auto subscribeSocket = network::zeromq::SubscribeSocket::Factory(
        Test_PublishSubscribe::context_, listenCallback);

    ASSERT_NE(nullptr, &subscribeSocket.get());
    ASSERT_EQ(SocketType::Subscribe, subscribeSocket->Type());

    subscribeSocket->SetTimeouts(
        std::chrono::milliseconds(0),
        std::chrono::milliseconds(-1),
        std::chrono::milliseconds(30000));
    subscribeSocket->Start(endpoint_);
    subscribeSocket->Start(endpoint2_);

    ++subscribeThreadStartedCount_;

    end = std::time(nullptr) + 30;
    while (callbackFinishedCount_ < callbackCount_ && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(callbackCount_, callbackFinishedCount_);

    publishSocketThread1.join();
    publishSocketThread2.join();
}

TEST_F(Test_PublishSubscribe, Publish_2_Subscribe_2)
{
    subscribeThreadCount_ = 2;
    callbackCount_ = 4;

    std::thread publishSocketThread1(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint_,
        testMessage_);
    std::thread publishSocketThread2(
        &Test_PublishSubscribe::publishSocketThread,
        this,
        endpoint2_,
        testMessage2_);

    auto end = std::time(nullptr) + 15;
    while (publishThreadStartedCount_ < 2 && std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(2, publishThreadStartedCount_);

    std::thread subscribeSocketThread1(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        std::set<std::string>({endpoint_, endpoint2_}),
        std::set<std::string>({testMessage_, testMessage2_}));
    std::thread subscribeSocketThread2(
        &Test_PublishSubscribe::subscribeSocketThread,
        this,
        std::set<std::string>({endpoint_, endpoint2_}),
        std::set<std::string>({testMessage_, testMessage2_}));

    end = std::time(nullptr) + 30;
    while (subscribeThreadStartedCount_ < subscribeThreadCount_ &&
           std::time(nullptr) < end)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    ASSERT_EQ(subscribeThreadCount_, subscribeThreadStartedCount_);

    publishSocketThread1.join();
    publishSocketThread2.join();

    subscribeSocketThread1.join();
    subscribeSocketThread2.join();
}
