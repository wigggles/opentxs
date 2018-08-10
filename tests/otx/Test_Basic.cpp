// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{
class Test_Basic : public ::testing::Test
{
public:
    const opentxs::ArgList args_;
    const opentxs::api::client::Manager& client_;
    const opentxs::api::server::Manager& server_;
    const std::string SeedA_;
    const std::string Alice_;
    const std::string AliceNymID_;
    std::shared_ptr<const ServerContract> server_contract_;

    Test_Basic()
        : args_({{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}})
        , client_(dynamic_cast<const opentxs::api::client::Manager&>(
              opentxs::OT::App().Client()))
        , server_(dynamic_cast<const opentxs::api::server::Manager&>(
              opentxs::OT::App().StartServer(args_, 0, true)))
        , SeedA_(opentxs::OT::App().Client().Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , Alice_(opentxs::OT::App().Client().Exec().CreateNymHD(
              proto::CITEMTYPE_INDIVIDUAL,
              "Alice",
              SeedA_,
              0))
        , AliceNymID_("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")
        , server_contract_(nullptr)
    {
    }
};

TEST_F(Test_Basic, ServerContract)
{
    const auto& serverID = server_.ID();

    ASSERT_FALSE(serverID.empty());

    server_contract_ = server_.Wallet().Server(serverID);

    ASSERT_TRUE(server_contract_);

    const auto clientVersion =
        client_.Wallet().Server(server_contract_->PublicContract());

    ASSERT_TRUE(clientVersion);
}

TEST_F(Test_Basic, getRequestNumber_not_registered)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_EQ(serverContext.It().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.It().UpdateRequestNumber();

    EXPECT_EQ(0, number);
    EXPECT_EQ(serverContext.It().Request(), 0);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_FALSE(clientContext);
}

TEST_F(Test_Basic, registerNym_first_time)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_EQ(serverContext.It().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;

    EXPECT_EQ(0, requestNumber);
    EXPECT_EQ(0, transactionNumber);
    EXPECT_EQ(SendResult::VALID_REPLY, result);
    EXPECT_TRUE(message);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(serverContext.It().Request(), clientContext->Request());
    EXPECT_EQ(serverContext.It().Request(), 1);
}

TEST_F(Test_Basic, getTransactionNumbers_first_time)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_EQ(serverContext.It().Request(), 1);
    ASSERT_TRUE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getTransactionNumbers(serverContext.It());
    const auto& [result, message] = reply;

    EXPECT_EQ(1, requestNumber);
    EXPECT_EQ(0, transactionNumber);
    EXPECT_EQ(SendResult::VALID_REPLY, result);
    ASSERT_TRUE(message);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(serverContext.It().Request(), clientContext->Request());
    EXPECT_EQ(serverContext.It().Request(), 2);
    EXPECT_NE(
        serverContext.It().LocalNymboxHash(), clientContext->LocalNymboxHash());
    EXPECT_EQ(
        serverContext.It().RemoteNymboxHash(),
        clientContext->LocalNymboxHash());
}

TEST_F(Test_Basic, Reregister)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_EQ(serverContext.It().Request(), 2);
    ASSERT_TRUE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;

    EXPECT_EQ(2, requestNumber);
    EXPECT_EQ(0, transactionNumber);
    EXPECT_EQ(SendResult::VALID_REPLY, result);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(serverContext.It().Request(), clientContext->Request());
    EXPECT_EQ(serverContext.It().Request(), 3);
}
}  // namespace
