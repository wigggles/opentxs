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
    EXPECT_NE(
        serverContext.It().LocalNymboxHash(), clientContext->LocalNymboxHash());
    EXPECT_EQ(
        serverContext.It().RemoteNymboxHash(),
        clientContext->LocalNymboxHash());
}

TEST_F(Test_Basic, getNymbox)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    EXPECT_EQ(serverContext.It().Request(), 3);
    ASSERT_TRUE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;

    EXPECT_EQ(3, requestNumber);
    EXPECT_EQ(0, transactionNumber);
    EXPECT_EQ(SendResult::VALID_REPLY, result);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(serverContext.It().Request(), clientContext->Request());
    EXPECT_EQ(serverContext.It().Request(), 4);
    EXPECT_EQ(
        serverContext.It().LocalNymboxHash(), clientContext->LocalNymboxHash());
    EXPECT_EQ(
        serverContext.It().RemoteNymboxHash(),
        clientContext->LocalNymboxHash());

    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_TRUE(1 == transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(OTTransaction::blank, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_transaction_numbers)
{
    auto serverContext = client_.Wallet().mutable_ServerContext(
        Identifier::Factory(AliceNymID_), server_.ID());
    auto clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    EXPECT_EQ(serverContext.It().Request(), 4);
    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_TRUE(1 == transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getBoxReceipt(serverContext.It(), nymID, 0, number);
    const auto& [result, message] = reply;

    EXPECT_EQ(4, requestNumber);
    EXPECT_EQ(0, transactionNumber);
    EXPECT_EQ(SendResult::VALID_REPLY, result);

    clientContext = server_.Wallet().ClientContext(
        server_.NymID(), Identifier::Factory(AliceNymID_));

    ASSERT_TRUE(clientContext);
    EXPECT_EQ(serverContext.It().Request(), clientContext->Request());
    EXPECT_EQ(serverContext.It().Request(), 5);
    EXPECT_EQ(
        serverContext.It().LocalNymboxHash(), clientContext->LocalNymboxHash());
    EXPECT_EQ(
        serverContext.It().RemoteNymboxHash(),
        clientContext->LocalNymboxHash());

    nymbox.reset(client_.OTAPI().LoadNymbox(serverID, nymID));

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_TRUE(1 == transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}
}  // namespace
