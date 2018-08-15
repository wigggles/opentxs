// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

#define NYMBOX_TYPE 0
#define NYMBOX_UPDATED true
#define NYMBOX_SAME false
#define NO_TRANSACTION false
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME "dollars"
#define UNIT_DEFINITION_SYMBOL "$"
#define UNIT_DEFINITION_TLA "USA"
#define UNIT_DEFINITION_POWER 2
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME "cents"

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
    const OTIdentifier alice_nym_id_;
    const Identifier& server_id_;
    std::shared_ptr<const ServerContract> server_contract_;
    std::shared_ptr<const UnitDefinition> asset_contract_1_;

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
        , alice_nym_id_(Identifier::Factory(
              std::string("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")))
        , server_id_(server_.ID())
        , server_contract_(server_.Wallet().Server(server_id_))
        , asset_contract_1_(nullptr)
    {
        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_id_.empty());

        const auto clientVersion =
            client_.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client_.Sync().SetIntroductionServer(*clientVersion);
    }

    void create_unit_definition()
    {
        asset_contract_1_ = client_.Wallet().UnitDefinition(
            alice_nym_id_->str(),
            UNIT_DEFINITION_CONTRACT_NAME,
            UNIT_DEFINITION_TERMS,
            UNIT_DEFINITION_PRIMARY_UNIT_NAME,
            UNIT_DEFINITION_SYMBOL,
            UNIT_DEFINITION_TLA,
            UNIT_DEFINITION_POWER,
            UNIT_DEFINITION_FRACTIONAL_UNIT_NAME);

        ASSERT_TRUE(asset_contract_1_);
        EXPECT_EQ(proto::UNITTYPE_CURRENCY, asset_contract_1_->Type());
    }

    OTIdentifier find_issuer_account()
    {
        const auto accounts = client_.Storage().AccountsByOwner(alice_nym_id_);

        OT_ASSERT(1 == accounts.size());

        return *accounts.begin();
    }

    void verify_account(
        const Nym& clientNym,
        const Nym& serverNym,
        const Account& client,
        const Account& server)
    {
        EXPECT_EQ(client.GetBalance(), server.GetBalance());
        EXPECT_EQ(
            client.GetInstrumentDefinitionID(),
            server.GetInstrumentDefinitionID());

        std::unique_ptr<Ledger> clientInbox(client.LoadInbox(clientNym));
        std::unique_ptr<Ledger> serverInbox(server.LoadInbox(serverNym));
        std::unique_ptr<Ledger> clientOutbox(client.LoadOutbox(clientNym));
        std::unique_ptr<Ledger> serverOutbox(server.LoadOutbox(serverNym));

        ASSERT_TRUE(clientInbox);
        ASSERT_TRUE(serverInbox);
        ASSERT_TRUE(clientOutbox);
        ASSERT_TRUE(serverOutbox);

        auto clientInboxHash = Identifier::Factory();
        auto serverInboxHash = Identifier::Factory();
        auto clientOutboxHash = Identifier::Factory();
        auto serverOutboxHash = Identifier::Factory();

        EXPECT_TRUE(clientInbox->CalculateInboxHash(clientInboxHash));
        EXPECT_TRUE(serverInbox->CalculateInboxHash(serverInboxHash));
        EXPECT_TRUE(clientOutbox->CalculateOutboxHash(clientOutboxHash));
        EXPECT_TRUE(serverOutbox->CalculateOutboxHash(serverOutboxHash));
    }

    void verify_state_pre(
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber)
    {
        EXPECT_EQ(serverContext.Request(), initialRequestNumber);
        EXPECT_EQ(clientContext.Request(), initialRequestNumber);
    }

    void verify_state_post(
        const ClientContext& clientContext,
        const ServerContext& serverContext,
        const RequestNumber initialRequestNumber,
        const RequestNumber messageRequestNumber,
        const TransactionNumber messageTransactionNumber,
        const SendResult messageResult,
        const std::shared_ptr<Message>& message,
        const bool nymboxHashUpdated,
        const bool usesTransaction,
        const std::size_t expectedNymboxItems)
    {
        const auto nextRequestNumber = initialRequestNumber + 1;

        EXPECT_EQ(messageRequestNumber, initialRequestNumber);
        EXPECT_EQ(serverContext.Request(), nextRequestNumber);
        EXPECT_EQ(clientContext.Request(), nextRequestNumber);
        EXPECT_EQ(
            serverContext.RemoteNymboxHash(), clientContext.LocalNymboxHash());

        if (nymboxHashUpdated) {
            EXPECT_NE(
                serverContext.LocalNymboxHash(),
                serverContext.RemoteNymboxHash());
        } else {
            EXPECT_EQ(
                serverContext.LocalNymboxHash(),
                serverContext.RemoteNymboxHash());
        }

        if (usesTransaction) {
            // TODO
        } else {
            EXPECT_EQ(0, messageTransactionNumber);
        }

        EXPECT_EQ(SendResult::VALID_REPLY, messageResult);
        ASSERT_TRUE(message);

        std::unique_ptr<Ledger> nymbox{
            client_.OTAPI().LoadNymbox(server_id_, alice_nym_id_)};

        ASSERT_TRUE(nymbox);
        EXPECT_TRUE(nymbox->VerifyAccount(*serverContext.Nym()));

        const auto& transactionMap = nymbox->GetTransactionMap();

        EXPECT_EQ(expectedNymboxItems, transactionMap.size());
    }
};

TEST_F(Test_Basic, getRequestNumber_not_registered)
{
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), 0);
    EXPECT_FALSE(clientContext);

    const auto number = serverContext.It().UpdateRequestNumber();

    EXPECT_EQ(0, number);
    EXPECT_EQ(serverContext.It().Request(), 0);

    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_FALSE(clientContext);
}

TEST_F(Test_Basic, registerNym_first_time)
{
    const RequestNumber sequence{0};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    EXPECT_EQ(serverContext.It().Request(), sequence);
    EXPECT_FALSE(clientContext);

    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;
    clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getTransactionNumbers_first_time)
{
    const RequestNumber sequence{1};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getTransactionNumbers(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, Reregister)
{
    const RequestNumber sequence{2};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().registerNym(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,  // TODO Maybe OTClient should have updated local nymbox
                         // hash
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_transaction_numbers)
{
    const RequestNumber sequence{3};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(OTTransaction::blank, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_transaction_numbers)
{
    const RequestNumber sequence{4};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
    nymbox.reset(client_.OTAPI().LoadNymbox(serverID, nymID));

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, processNymbox)
{
    const RequestNumber sequence{5};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}

TEST_F(Test_Basic, getNymbox_after_processNymbox)
{
    const RequestNumber sequence{6};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(
        serverContext.It().Server(), serverContext.It().Nym()->ID())};

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const TransactionNumber number{transactionMap.begin()->first};
    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_TRUE(transaction.IsAbbreviated());
    EXPECT_EQ(OTTransaction::successNotice, transaction.GetType());
    EXPECT_FALSE(nymbox->LoadBoxReceipt(number));
}

TEST_F(Test_Basic, getBoxReceipt_success_notice)
{
    const RequestNumber sequence{7};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);
    const auto& nymID = serverContext.It().Nym()->ID();
    const auto& serverID = serverContext.It().Server();

    ASSERT_TRUE(clientContext);

    std::unique_ptr<Ledger> nymbox{client_.OTAPI().LoadNymbox(serverID, nymID)};

    ASSERT_TRUE(nymbox);

    TransactionNumber number{0};

    {
        const auto& transactionMap = nymbox->GetTransactionMap();

        ASSERT_EQ(1, transactionMap.size());

        number = {transactionMap.begin()->first};
        nymbox.reset();
    }

    ASSERT_NE(0, number);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getBoxReceipt(
            serverContext.It(), nymID, NYMBOX_TYPE, number);
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);

    nymbox.reset(client_.OTAPI().LoadNymbox(serverID, nymID));

    ASSERT_TRUE(nymbox);

    const auto& transactionMap = nymbox->GetTransactionMap();

    ASSERT_EQ(1, transactionMap.size());

    const auto& transaction = *transactionMap.begin()->second;

    EXPECT_FALSE(transaction.IsAbbreviated());
}

TEST_F(Test_Basic, issueAsset)
{
    const RequestNumber sequence{8};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    create_unit_definition();
    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().registerInstrumentDefinition(
            serverContext.It(), asset_contract_1_->PublicContract());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        1);
    const auto accountID = Identifier::Factory(message->m_strAcctID);

    ASSERT_FALSE(accountID->empty());

    const auto clientAccount = client_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getAccountData_after_issuer_account)
{
    const RequestNumber sequence{9};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    const auto accountID = find_issuer_account();

    ASSERT_FALSE(accountID->empty());

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getAccountData(serverContext.It(), accountID);
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        1);

    const auto clientAccount = client_.Wallet().Account(accountID);
    const auto serverAccount = server_.Wallet().Account(accountID);

    ASSERT_TRUE(clientAccount);
    ASSERT_TRUE(serverAccount);

    verify_account(
        *serverContext.It().Nym(),
        *clientContext->Nym(),
        clientAccount,
        serverAccount);
}

TEST_F(Test_Basic, getNymbox_after_registerInstrumentDefinition)
{
    const RequestNumber sequence{10};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().getNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_SAME,
        NO_TRANSACTION,
        1);
}

TEST_F(Test_Basic, processNymbox_after_registerInstrumentDefinition)
{
    const RequestNumber sequence{11};
    auto serverContext =
        client_.Wallet().mutable_ServerContext(alice_nym_id_, server_id_);
    auto clientContext =
        server_.Wallet().ClientContext(server_.NymID(), alice_nym_id_);

    ASSERT_TRUE(clientContext);

    verify_state_pre(*clientContext, serverContext.It(), sequence);
    const auto [requestNumber, transactionNumber, reply] =
        client_.OTAPI().processNymbox(serverContext.It());
    const auto& [result, message] = reply;
    verify_state_post(
        *clientContext,
        serverContext.It(),
        sequence,
        requestNumber,
        transactionNumber,
        result,
        message,
        NYMBOX_UPDATED,
        NO_TRANSACTION,
        0);
}
}  // namespace
