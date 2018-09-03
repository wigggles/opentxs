// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define TEST_NYM_4 "testNym4"
#define TEST_NYM_5 "testNym5"
#define TEST_NYM_6 "testNym6"

using namespace opentxs;

namespace
{

class Test_Rpc_Queued : public ::testing::Test
{
public:
    Test_Rpc_Queued()
        : ot_{opentxs::OT::App()}
    {
        if (false == bool(notification_callback_)) {
            notification_callback_.reset(new OTZMQListenCallback(
                network::zeromq::ListenCallback::Factory(
                    [](const network::zeromq::Message& incoming) -> void {
                        process_notification(incoming);
                    })));
        }
        if (false == bool(notification_socket_)) {
            notification_socket_.reset(new OTZMQSubscribeSocket(
                ot_.ZMQ().SubscribeSocket(*notification_callback_)));
        }
    }

protected:
    const opentxs::api::Native& ot_;

    static int client_a_;
    static int client_b_;
    static bool create_account_complete_;
    static std::string create_account_task_;
    static int intro_server_;
    static bool issue_unitdef_complete_;
    static std::string issue_unitdef_task_;
    static std::unique_ptr<OTZMQListenCallback> notification_callback_;
    static std::unique_ptr<OTZMQSubscribeSocket> notification_socket_;
    static std::string receiver_nym_id_;
    static std::set<std::string> register_nym_tasks_;
    static std::int32_t register_nyms_count_;
    static bool send_payment_cheque_complete_;
    static std::mutex task_lock_;
    static std::string send_payment_cheque_task_;
    static std::string sender_nym_id_;
    static int server_;
    static std::string unit_definition_id_;

    static void accept_cheque_1(
        const api::client::Manager& client,
        const Identifier& serverID,
        const Identifier& nymID,
        const Identifier& accountID);
    static std::size_t get_index(const std::int32_t instance);
    static const api::Core& get_session(const std::int32_t instance);
    static const std::set<OTIdentifier> get_accounts(
        const proto::RPCCommand& command);
    static void process_notification(const network::zeromq::Message& incoming);
    static void process_receipt_1(
        const api::client::Manager& client,
        const Identifier& serverID,
        const Identifier& nymID,
        const Identifier& accountID);
    static void setup();
    static void verify_nym_and_unitdef();
    static void verify_account();

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = opentxs::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(1);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }
};

int Test_Rpc_Queued::client_a_{0};
int Test_Rpc_Queued::client_b_{0};
bool Test_Rpc_Queued::create_account_complete_{false};
std::string Test_Rpc_Queued::create_account_task_;
int Test_Rpc_Queued::intro_server_{0};
bool Test_Rpc_Queued::issue_unitdef_complete_{false};
std::string Test_Rpc_Queued::issue_unitdef_task_;
std::unique_ptr<OTZMQListenCallback> Test_Rpc_Queued::notification_callback_{
    nullptr};
std::unique_ptr<OTZMQSubscribeSocket> Test_Rpc_Queued::notification_socket_{
    nullptr};
std::string Test_Rpc_Queued::receiver_nym_id_;
std::int32_t Test_Rpc_Queued::register_nyms_count_{0};
std::set<std::string> Test_Rpc_Queued::register_nym_tasks_;
bool Test_Rpc_Queued::send_payment_cheque_complete_{false};
std::mutex Test_Rpc_Queued::task_lock_{};
std::string Test_Rpc_Queued::send_payment_cheque_task_;
std::string Test_Rpc_Queued::sender_nym_id_;
int Test_Rpc_Queued::server_{0};
std::string Test_Rpc_Queued::unit_definition_id_;

void Test_Rpc_Queued::accept_cheque_1(
    const api::client::Manager& client,
    const Identifier& serverID,
    const Identifier& nymID,
    const Identifier& accountID)
{
    auto nymbox = client.ServerAction().DownloadNymbox(nymID, serverID);

    ASSERT_TRUE(nymbox);

    auto account =
        client.ServerAction().DownloadAccount(nymID, serverID, accountID, true);

    ASSERT_TRUE(account);

    const auto workflows = client.Storage().PaymentWorkflowList(nymID.str());

    ASSERT_EQ(1, workflows.size());

    const auto workflowID = Identifier::Factory(workflows.begin()->first);
    const auto workflow = client.Workflow().LoadWorkflow(nymID, workflowID);

    ASSERT_TRUE(workflow);
    ASSERT_TRUE(api::client::Workflow::ContainsCheque(*workflow));

    auto [state, cheque] =
        api::client::Workflow::InstantiateCheque(client, *workflow);

    ASSERT_EQ(state, proto::PAYMENTWORKFLOWSTATE_CONVEYED);
    ASSERT_TRUE(cheque);

    nymbox = client.ServerAction().DownloadNymbox(nymID, serverID);

    ASSERT_TRUE(nymbox);

    const auto numbers =
        client.ServerAction().GetTransactionNumbers(nymID, serverID, 1);

    ASSERT_TRUE(numbers);

    auto deposited =
        client.ServerAction().DepositCheque(nymID, serverID, accountID, cheque);

    deposited->Run();

    ASSERT_EQ(SendResult::VALID_REPLY, deposited->LastSendResult());
    ASSERT_TRUE(deposited->Reply());
    ASSERT_TRUE(deposited->Reply()->m_bSuccess);

    account =
        client.ServerAction().DownloadAccount(nymID, serverID, accountID, true);

    ASSERT_TRUE(account);

    nymbox = client.ServerAction().DownloadNymbox(nymID, serverID);

    ASSERT_TRUE(nymbox);
}

std::size_t Test_Rpc_Queued::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
};

const api::Core& Test_Rpc_Queued::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return opentxs::OT::App().Server(get_index(instance));
    } else {
        return opentxs::OT::App().Client(get_index(instance));
    }
};

void Test_Rpc_Queued::process_notification(
    const network::zeromq::Message& incoming)
{
    ASSERT_EQ(2, incoming.Body().size());

    const std::string taskid = incoming.Body().at(0);
    const auto& frame = incoming.Body().at(1);
    const auto data = Data::Factory(frame.data(), frame.size());
    const auto completed = static_cast<bool>(data->at(0));
    Lock lock(task_lock_);

    if (0 != register_nym_tasks_.count(taskid)) {
        ++register_nyms_count_;
    } else if (taskid == issue_unitdef_task_) {
        issue_unitdef_complete_ = true;
    } else if (taskid == send_payment_cheque_task_) {
        send_payment_cheque_complete_ = true;
    } else if (taskid == create_account_task_) {
        create_account_complete_ = true;
    }
};

void Test_Rpc_Queued::process_receipt_1(
    const api::client::Manager& client,
    const Identifier& serverID,
    const Identifier& nymID,
    const Identifier& accountID)
{
    auto nymbox = client.ServerAction().DownloadNymbox(nymID, serverID);

    ASSERT_TRUE(nymbox);

    auto account =
        client.ServerAction().DownloadAccount(nymID, serverID, accountID, true);

    ASSERT_TRUE(account);

    const auto accepted =
        client.Sync().AcceptIncoming(nymID, accountID, serverID);

    ASSERT_TRUE(accepted);
}

void Test_Rpc_Queued::setup()
{
    const api::Native& ot = opentxs::OT::App();

    ArgList args{{OPENTXS_ARG_COMMANDPORT, {"7088"}},
                 {OPENTXS_ARG_LISTENCOMMAND, {"7088"}}};

    auto& intro_server = ot.StartServer(args, ot.Servers());

    auto& commandport = args[OPENTXS_ARG_COMMANDPORT];
    commandport.clear();
    commandport.emplace("7089");

    auto& listencommand = args[OPENTXS_ARG_LISTENCOMMAND];
    listencommand.clear();
    listencommand.emplace("7089");

    auto& server = ot.StartServer(args, ot.Servers());

    auto server_contract = server.Wallet().Server(server.ID());
    intro_server.Wallet().Server(server_contract->PublicContract());

    auto intro_server_contract =
        intro_server.Wallet().Server(intro_server.ID());

    ArgList empty_args;
    auto& client_a = ot.StartClient(empty_args, ot.Clients());
    auto& client_b = ot.StartClient(empty_args, ot.Clients());

    auto client_a_server_contract =
        client_a.Wallet().Server(intro_server_contract->PublicContract());
    client_a.Sync().SetIntroductionServer(*client_a_server_contract);

    auto client_b_server_contract =
        client_b.Wallet().Server(intro_server_contract->PublicContract());
    client_b.Sync().SetIntroductionServer(*client_b_server_contract);

    auto started =
        notification_socket_->get().Start(client_a.Endpoints().TaskComplete());

    ASSERT_TRUE(started);

    started =
        notification_socket_->get().Start(client_b.Endpoints().TaskComplete());

    ASSERT_TRUE(started);

    sender_nym_id_ =
        client_a.Wallet().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_4);

    receiver_nym_id_ =
        client_b.Wallet().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_5);

    auto unit_definition = client_a.Wallet().UnitDefinition(
        sender_nym_id_,
        "gdollar",
        "GoogleTestDollar",
        "G",
        "Google Test Dollars",
        "GTD",
        2,
        "gcent");

    ASSERT_TRUE(bool(unit_definition));

    unit_definition_id_ = unit_definition->ID()->str();

    intro_server_ = intro_server.Instance();
    server_ = server.Instance();
    client_a_ = client_a.Instance();
    client_b_ = client_b.Instance();
}

void Test_Rpc_Queued::verify_nym_and_unitdef()
{
    auto end = std::time(nullptr) + 20;
    while (std::time(nullptr) < end) {
        if (2 == register_nyms_count_ && issue_unitdef_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_EQ(2, register_nyms_count_);
    ASSERT_TRUE(issue_unitdef_complete_);
};

void Test_Rpc_Queued::verify_account()
{
    auto end = std::time(nullptr) + 20;
    while (std::time(nullptr) < end) {
        if (true == create_account_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(create_account_complete_);
};

TEST_F(Test_Rpc_Queued, RegisterNym)
{
    setup();

    // Register the sender nym.
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(client_a_);
    command.set_owner(sender_nym_id_);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_TRUE(0 == response.identifier_size());

    register_nym_tasks_.emplace(response.task());

    // Register the receiver nym.
    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(client_b_);

    command.set_owner(receiver_nym_id_);

    command.set_notary(server.ID().str());

    response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 == response.identifier_size());

    register_nym_tasks_.emplace(response.task());
}

TEST_F(Test_Rpc_Queued, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(client_a_);
    command.set_owner(sender_nym_id_);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_);

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    issue_unitdef_task_ = response.task();
}

TEST_F(Test_Rpc_Queued, Verify_Responses) { verify_nym_and_unitdef(); }

TEST_F(Test_Rpc_Queued, Send_Payment_Cheque)
{
    auto& client_a = ot_.Client(get_index(client_a_));
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(client_a_);
    auto& client_b = ot_.Client(get_index(client_b_));
    auto nym5 = client_b.Wallet().Nym(Identifier::Factory(receiver_nym_id_));

    ASSERT_TRUE(bool(nym5));

    auto& contacts = client_a.Contacts();
    const auto contact = contacts.NewContact(
        std::string(TEST_NYM_5),
        nym5->ID(),
        client_a.Factory().PaymentCode(nym5->PaymentCode()));

    ASSERT_TRUE(contact);

    const auto issueraccounts = client_a.Storage().AccountsByIssuer(
        Identifier::Factory(sender_nym_id_));

    ASSERT_TRUE(false == issueraccounts.empty());

    auto issueraccountid = *issueraccounts.cbegin();
    Lock lock(task_lock_);

    while (send_payment_cheque_task_.empty()) {
        auto sendpayment = command.mutable_sendpayment();

        ASSERT_NE(nullptr, sendpayment);

        sendpayment->set_version(1);
        sendpayment->set_type(proto::RPCPAYMENTTYPE_CHEQUE);
        sendpayment->set_contact(contact->ID().str());
        sendpayment->set_sourceaccount(issueraccountid->str());
        sendpayment->set_memo("Send_Payment_Cheque test");
        sendpayment->set_amount(100);

        auto response = ot_.RPC(command);

        ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.success());
        ASSERT_EQ(1, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        send_payment_cheque_task_ = response.task();
    }
}

TEST_F(Test_Rpc_Queued, Verify_Send_Cheque)
{
    auto end = std::time(nullptr) + 20;

    while (std::time(nullptr) < end) {
        if (send_payment_cheque_complete_) { break; }

        Log::Sleep(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(send_payment_cheque_complete_);

    //    auto& client = ot_.Client(get_index(client_instance_));
    //    auto nym4 = client.Wallet().NymByIDPartialMatch(TEST_NYM_4);
    //
    //    ASSERT_TRUE(bool(nym4));
    //
    //    auto nym5 = client.Wallet().NymByIDPartialMatch(TEST_NYM_5);
    //
    //    ASSERT_TRUE(bool(nym5));
    //
    //    auto& server = ot_.Server(get_index(server_instance_));
    //
    //    const auto nym5_accounts =
    //    client.Storage().AccountsByOwner(nym5->ID());
    //
    //    ASSERT_TRUE(!nym5_accounts.empty());
    //    auto nym5_account_id = *nym5_accounts.cbegin();
    //
    //    const auto issueraccounts =
    //    client.Storage().AccountsByIssuer(nym4->ID());
    //
    //    ASSERT_TRUE(!issueraccounts.empty());
    //    auto issuer_account_id = *issueraccounts.cbegin();
    //
    //    accept_cheque_1(
    //        client,
    //        Identifier::Factory(server.ID()),
    //        nym5->ID(),
    //        Identifier::Factory(nym5_account_id));
    //    process_receipt_1(
    //        client,
    //        Identifier::Factory(server.ID()),
    //        nym4->ID(),
    //        Identifier::Factory(issuer_account_id));
    //
    //    {
    //        const auto account =
    //            client.Wallet().Account(Identifier::Factory(issuer_account_id));
    //
    //        ASSERT_TRUE(account);
    //
    //        ASSERT_EQ(-100, account.get().GetBalance());
    //    }
    //
    //    {
    //        const auto account =
    //            client.Wallet().Account(Identifier::Factory(nym5_account_id));
    //
    //        ASSERT_TRUE(account);
    //
    //        ASSERT_EQ(100, account.get().GetBalance());
    //    }
}

TEST_F(Test_Rpc_Queued, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(client_a_);

    auto& client_a = ot_.Client(get_index(client_a_));
    auto nym_id =
        client_a.Wallet().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, TEST_NYM_6);

    ASSERT_FALSE(nym_id.empty());

    command.set_owner(nym_id);
    auto& server = ot_.Server(get_index(server_));
    command.set_notary(server.ID().str());
    command.set_unit(unit_definition_id_);

    Lock lock(task_lock_);
    auto response = ot_.RPC(command);

    ASSERT_EQ(proto::RPCRESPONSE_QUEUED, response.success());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    create_account_task_ = response.task();
}

TEST_F(Test_Rpc_Queued, Verify_Account) { verify_account(); }

// TODO: tests for RPCPAYMENTTYPE_VOUCHER, RPCPAYMENTTYPE_INVOICE,
// RPCPAYMENTTYPE_BLIND

TEST_F(Test_Rpc_Queued, Cleanup)
{
    notification_callback_.reset();
    notification_socket_.reset();
}

}  // namespace
