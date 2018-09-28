// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define TEST_NYM_1 "testNym1"
#define TEST_NYM_2 "testNym2"
#define TEST_NYM_3 "testNym3"

#define COMMAND_VERSION 1
#define APIARG_VERSION 1
#define CREATENYM_VERSION 1
#define ADDCONTACT_VERSION 1
#define CREATEINSTRUMENTDEFINITION_VERSION 1
#define SENDPAYMENT_VERSION 1
#define MOVEFUNDS_VERSION 1

using namespace opentxs;

namespace
{

class Test_Rpc : public ::testing::Test
{
public:
    Test_Rpc()
        : ot_{opentxs::OT::App()}
    {
    }

protected:
    const opentxs::api::Native& ot_;

    static std::string issuer_account_id_;
    static proto::ServerContract server_contract_;
    static std::string server_id_;
    static std::string nym2_account_id_;
    static std::string nym3_account1_id_;
    static std::string nym3_account2_id_;
    static std::map<std::string, int> widget_update_counters_;
    static std::mutex widget_update_lock_;

    static void accept_cheque_1(
        const api::client::Manager& client,
        const Identifier& serverID,
        const Identifier& nymID,
        const Identifier& accountID);
    static void accept_transfer_1(
        const api::client::Manager& client,
        const Identifier& serverID,
        const Identifier& nymID,
        const Identifier& accountID);
    static std::size_t get_index(const std::int32_t instance);
    static const api::Core& get_session(const std::int32_t instance);
    static int get_widget_count(const std::string& id);
    static void increment_widget(const std::string& id);
    static void process_receipt_1(
        const api::client::Manager& client,
        const Identifier& serverID,
        const Identifier& nymID,
        const Identifier& accountID);

    proto::RPCCommand init(proto::RPCCommandType commandtype)
    {
        auto cookie = opentxs::Identifier::Random()->str();

        proto::RPCCommand command;
        command.set_version(COMMAND_VERSION);
        command.set_cookie(cookie);
        command.set_type(commandtype);

        return command;
    }

    bool add_session(proto::RPCCommandType commandtype, ArgList& args)
    {
        auto command = init(commandtype);
        command.set_session(-1);
        for (auto& arg : args) {
            auto apiarg = command.add_arg();
            apiarg->set_version(APIARG_VERSION);
            apiarg->set_key(arg.first);
            apiarg->add_value(*arg.second.begin());
        }
        auto response = ot_.RPC(command);

        EXPECT_TRUE(proto::Validate(response, VERBOSE));
        
        EXPECT_EQ(1, response.status_size());
        return proto::RPCRESPONSE_SUCCESS == response.status(0).code();
    }

    void list(proto::RPCCommandType commandtype, std::int32_t session = -1)
    {
        auto command = init(commandtype);
        command.set_session(session);

        auto response = ot_.RPC(command);

        ASSERT_TRUE(proto::Validate(response, VERBOSE));

        ASSERT_EQ(1, response.version());
        ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
        ASSERT_EQ(command.type(), response.type());

        ASSERT_EQ(1, response.status_size());
        ASSERT_EQ(proto::RPCRESPONSE_NONE, response.status(0).code());
    }
};

std::string Test_Rpc::issuer_account_id_{};
proto::ServerContract Test_Rpc::server_contract_;
std::string Test_Rpc::server_id_{};
std::string Test_Rpc::nym2_account_id_{};
std::string Test_Rpc::nym3_account1_id_{};
std::string Test_Rpc::nym3_account2_id_{};
std::map<std::string, int> Test_Rpc::widget_update_counters_{};
std::mutex Test_Rpc::widget_update_lock_{};

void Test_Rpc::accept_cheque_1(
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

void Test_Rpc::accept_transfer_1(
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

    nymbox = client.ServerAction().DownloadNymbox(nymID, serverID);

    ASSERT_TRUE(nymbox);

    const auto numbers =
        client.ServerAction().GetTransactionNumbers(nymID, serverID, 1);

    ASSERT_TRUE(numbers);

    process_receipt_1(client, serverID, nymID, accountID);
}

std::size_t Test_Rpc::get_index(const std::int32_t instance)
{
    return (instance - (instance % 2)) / 2;
};

const api::Core& Test_Rpc::get_session(const std::int32_t instance)
{
    auto is_server = instance % 2;

    if (is_server) {
        return opentxs::OT::App().Server(get_index(instance));
    } else {
        return opentxs::OT::App().Client(get_index(instance));
    }
};

int Test_Rpc::get_widget_count(const std::string& id)
{
    Lock lock(widget_update_lock_);

    try {
        return widget_update_counters_.at(id);
    } catch (...) {
        return -1;
    }
}

void Test_Rpc::process_receipt_1(
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

void Test_Rpc::increment_widget(const std::string& id)
{
    Lock lock(widget_update_lock_);

    try {
        LogOutput("Widget ")(id)(" update counter incremented to ")(
            ++widget_update_counters_.at(id))
            .Flush();
    } catch (...) {
    }
}

TEST_F(Test_Rpc, List_Client_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTCLIENTSESSIONS);
}

TEST_F(Test_Rpc, List_Server_Sessions_None)
{
    list(proto::RPCCOMMAND_LISTSERVERSESSIONS);
}

// The client created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Client_Session)
{
    auto command = init(proto::RPCCOMMAND_ADDCLIENTSESSION);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(0, response.session());
}

TEST_F(Test_Rpc, List_Server_Contracts_None)
{
    list(proto::RPCCOMMAND_LISTSERVERCONTRACTS, 0);
}

// The server created in this test gets used in subsequent tests.
TEST_F(Test_Rpc, Add_Server_Session)
{
    auto command = init(proto::RPCCOMMAND_ADDSERVERSESSION);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.session());

    auto& manager = get_session(response.session());

    // Register the server on the client.
    auto& servermanager = dynamic_cast<const api::server::Manager&>(manager);
    server_id_ = servermanager.ID().str();
    auto servercontract = servermanager.Wallet().Server(servermanager.ID());

    auto& client = get_session(0);
    auto& clientmanager = dynamic_cast<const api::client::Manager&>(client);
    auto clientservercontract =
        clientmanager.Wallet().Server(servercontract->PublicContract());

    // Make the server the introduction server.
    clientmanager.Sync().SetIntroductionServer(*clientservercontract);
}

TEST_F(Test_Rpc, List_Client_Sessions)
{
    ArgList args;
    auto added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    ASSERT_TRUE(added);

    added = add_session(proto::RPCCOMMAND_ADDCLIENTSESSION, args);
    ASSERT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTCLIENTSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(3, response.sessions_size());

    for (auto& session : response.sessions()) {
        ASSERT_EQ(1, session.version());
        ASSERT_TRUE(
            0 == session.instance() || 2 == session.instance() ||
            4 == session.instance());
    }
}

TEST_F(Test_Rpc, List_Server_Sessions)
{
    ArgList args{{OPENTXS_ARG_COMMANDPORT, {"7086"}},
                 {OPENTXS_ARG_LISTENCOMMAND, {"7086"}}};

    auto added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    ASSERT_TRUE(added);

    auto& commandport = args[OPENTXS_ARG_COMMANDPORT];
    commandport.clear();
    commandport.emplace("7087");

    auto& listencommand = args[OPENTXS_ARG_LISTENCOMMAND];
    listencommand.clear();
    listencommand.emplace("7087");

    added = add_session(proto::RPCCOMMAND_ADDSERVERSESSION, args);
    ASSERT_TRUE(added);

    auto command = init(proto::RPCCOMMAND_LISTSERVERSESSIONS);
    command.set_session(-1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(3, response.sessions_size());

    for (auto& session : response.sessions()) {
        ASSERT_EQ(1, session.version());
        ASSERT_TRUE(
            1 == session.instance() || 3 == session.instance() ||
            5 == session.instance());
    }
}

TEST_F(Test_Rpc, List_Server_Contracts)
{
    auto command = init(proto::RPCCOMMAND_LISTSERVERCONTRACTS);
    command.set_session(1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, Get_Notary_Contract)
{
    auto command = init(proto::RPCCOMMAND_GETSERVERCONTRACT);
    command.set_session(0);
    command.add_identifier(server_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.notary_size());

    server_contract_ = response.notary(0);
}

TEST_F(Test_Rpc, Import_Server_Contract)
{
    auto command = init(proto::RPCCOMMAND_IMPORTSERVERCONTRACT);
    command.set_session(2);
    auto& server = *command.add_server();
    server = server_contract_;

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, Import_Server_Contract_Partial)
{
    auto command = init(proto::RPCCOMMAND_IMPORTSERVERCONTRACT);
    command.set_session(3);
    auto& server = *command.add_server();
    server = server_contract_;

    auto& invalid_server = *command.add_server();
    invalid_server = server_contract_;
    invalid_server.set_nymid("invalid nym identifier");

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(2, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(proto::RPCRESPONSE_NONE, response.status(1).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, List_Contacts_None)
{
    list(proto::RPCCOMMAND_LISTCONTACTS, 0);
}

// The nym created in this test is used in subsequent tests.
TEST_F(Test_Rpc, Create_Nym)
{
    // Add tests for specifying the seedid and index (not -1).
    // Add tests for adding claims.

    auto command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    auto createnym = command.mutable_createnym();

    ASSERT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(proto::CITEMTYPE_INDIVIDUAL);
    createnym->set_name(TEST_NYM_1);
    createnym->set_index(-1);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 != response.identifier_size());

    // Now create more nyms for later tests.
    command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    createnym = command.mutable_createnym();

    ASSERT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(proto::CITEMTYPE_INDIVIDUAL);
    createnym->set_name(TEST_NYM_2);
    createnym->set_index(-1);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_TRUE(0 != response.identifier_size());

    command = init(proto::RPCCOMMAND_CREATENYM);
    command.set_session(0);

    createnym = command.mutable_createnym();

    ASSERT_NE(nullptr, createnym);

    createnym->set_version(CREATENYM_VERSION);
    createnym->set_type(proto::CITEMTYPE_INDIVIDUAL);
    createnym->set_name(TEST_NYM_3);
    createnym->set_index(-1);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_TRUE(0 != response.identifier_size());
}

TEST_F(Test_Rpc, List_Contacts)
{
    auto command = init(proto::RPCCOMMAND_LISTCONTACTS);
    command.set_session(0);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_TRUE(3 == response.identifier_size());
}

TEST_F(Test_Rpc, Add_Contact)
{
    // Add a contact using a label.
    auto command = init(proto::RPCCOMMAND_ADDCONTACT);
    command.set_session(0);

    auto& addcontact = *command.add_addcontact();
    addcontact.set_version(ADDCONTACT_VERSION);
    addcontact.set_label("TestContact1");

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_EQ(1, response.identifier_size());

    // Add a contact using a nym id.
    auto& client = ot_.Client(0);
    ASSERT_EQ(4, client.Contacts().ContactList().size());

    auto& client2 = ot_.Client(2);

    command = init(proto::RPCCOMMAND_ADDCONTACT);

    command.set_session(2);

    auto& addcontact2 = *command.add_addcontact();
    addcontact2.set_version(ADDCONTACT_VERSION);
    addcontact2.set_nymid(
        client.Wallet().NymByIDPartialMatch(TEST_NYM_2)->ID().str());

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_EQ(1, response.identifier_size());

    // Add a contact using a payment code.
    command = init(proto::RPCCOMMAND_ADDCONTACT);

    command.set_session(2);

    auto& addcontact3 = *command.add_addcontact();
    addcontact3.set_version(ADDCONTACT_VERSION);
    addcontact3.set_paymentcode(
        client.Wallet().NymByIDPartialMatch(TEST_NYM_3)->PaymentCode());

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());

    ASSERT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, List_Unit_Definitions_None)
{
    list(proto::RPCCOMMAND_LISTUNITDEFINITIONS, 0);
}

TEST_F(Test_Rpc, Create_Unit_Definition)
{
    auto command = init(proto::RPCCOMMAND_CREATEUNITDEFINITION);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto def = command.mutable_createunit();

    ASSERT_NE(nullptr, def);

    def->set_version(CREATEINSTRUMENTDEFINITION_VERSION);
    def->set_name("GoogleTestDollar");
    def->set_symbol("G");
    def->set_primaryunitname("gdollar");
    def->set_fractionalunitname("gcent");
    def->set_tla("GTD");
    def->set_power(2);
    def->set_terms("Google Test Dollars");
    def->set_unitofaccount(proto::CITEMTYPE_USD);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 != response.identifier_size());
}

TEST_F(Test_Rpc, List_Unit_Definitions)
{
    auto command = init(proto::RPCCOMMAND_LISTUNITDEFINITIONS);
    command.set_session(0);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.identifier_size());
}

TEST_F(Test_Rpc, RegisterNym)
{
    auto command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    // Register the other nyms.
    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);

    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_2);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());
    command.set_notary(server.ID().str());

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    command = init(proto::RPCCOMMAND_REGISTERNYM);
    command.set_session(0);

    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_3);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());
    command.set_notary(server.ID().str());

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
}

TEST_F(Test_Rpc, List_Accounts_None)
{
    list(proto::RPCCOMMAND_LISTACCOUNTS, 0);
}

TEST_F(Test_Rpc, Create_Issuer_Account)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    const auto unitdefinitionlist = manager.Wallet().UnitDefinitionList();
    ASSERT_TRUE(!unitdefinitionlist.empty());

    auto& unitid = unitdefinitionlist.front().first;
    command.set_unit(unitid);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        ASSERT_TRUE(Identifier::Validate(accountID));

        issuer_account_id_ = accountID;
    }
}

TEST_F(Test_Rpc, Create_Issuer_Account_Unnecessary)
{
    auto command = init(proto::RPCCOMMAND_ISSUEUNITDEFINITION);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());

    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());

    const auto unitdefinitionlist = manager.Wallet().UnitDefinitionList();
    ASSERT_TRUE(!unitdefinitionlist.empty());

    auto& unitid = unitdefinitionlist.front().first;
    command.set_unit(unitid);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_UNNECESSARY, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(0, response.identifier_size());
}

TEST_F(Test_Rpc, Create_Account)
{
    auto command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    auto& manager = ot_.Client(0);
    auto nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_2);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());
    auto& server = ot_.Server(0);
    command.set_notary(server.ID().str());
    const auto unitdefinitionlist = manager.Wallet().UnitDefinitionList();

    ASSERT_TRUE(!unitdefinitionlist.empty());

    auto& unitid = unitdefinitionlist.front().first;
    command.set_unit(unitid);
    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());
    ASSERT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        ASSERT_TRUE(Identifier::Validate(accountID));

        nym2_account_id_ = accountID;
    }

    // Create two accounts for nym 3.
    command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_3);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());
    command.set_notary(server.ID().str());
    command.set_unit(unitid);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        ASSERT_TRUE(Identifier::Validate(accountID));

        nym3_account1_id_ = response.identifier(0);
    }

    command = init(proto::RPCCOMMAND_CREATEACCOUNT);
    command.set_session(0);
    nym = manager.Wallet().NymByIDPartialMatch(TEST_NYM_3);

    ASSERT_TRUE(bool(nym));

    command.set_owner(nym->ID().str());
    command.set_notary(server.ID().str());
    command.set_unit(unitid);

    response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_EQ(1, response.identifier_size());

    {
        const auto& accountID = response.identifier(0);

        ASSERT_TRUE(Identifier::Validate(accountID));

        nym3_account2_id_ = response.identifier(0);
    }
}

TEST_F(Test_Rpc, Send_Payment_Transfer)
{
    auto command = init(proto::RPCCOMMAND_SENDPAYMENT);
    command.set_session(0);

    auto& server = ot_.Server(0);
    auto& client = ot_.Client(0);
    auto nym1 = client.Wallet().NymByIDPartialMatch(TEST_NYM_1);

    ASSERT_TRUE(bool(nym1));

    auto nymbox = client.ServerAction().DownloadNymbox(nym1->ID(), server.ID());

    ASSERT_TRUE(nymbox);

    const auto numbers =
        client.ServerAction().GetTransactionNumbers(nym1->ID(), server.ID(), 1);

    ASSERT_TRUE(numbers);

    auto sendpayment = command.mutable_sendpayment();

    ASSERT_NE(nullptr, sendpayment);

    sendpayment->set_version(SENDPAYMENT_VERSION);
    sendpayment->set_type(proto::RPCPAYMENTTYPE_TRANSFER);

    auto nym3 = client.Wallet().NymByIDPartialMatch(TEST_NYM_3);

    ASSERT_TRUE(bool(nym3));

    auto& contacts = client.Contacts();
    const auto contactid = contacts.ContactID(nym3->ID());
    sendpayment->set_contact(contactid->str());
    sendpayment->set_sourceaccount(issuer_account_id_);
    sendpayment->set_destinationaccount(nym3_account1_id_);
    sendpayment->set_memo("Send_Payment_Transfer test");
    sendpayment->set_amount(75);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    accept_transfer_1(
        client,
        Identifier::Factory(server_id_),
        nym3->ID(),
        Identifier::Factory(nym3_account1_id_));
    process_receipt_1(
        client,
        Identifier::Factory(server_id_),
        nym1->ID(),
        Identifier::Factory(issuer_account_id_));

    {
        const auto account =
            client.Wallet().Account(Identifier::Factory(nym3_account1_id_));

        ASSERT_TRUE(account);

        ASSERT_EQ(75, account.get().GetBalance());
    }

    {
        const auto account =
            client.Wallet().Account(Identifier::Factory(issuer_account_id_));

        ASSERT_TRUE(account);

        ASSERT_EQ(-75, account.get().GetBalance());
    }
}

// TODO: tests for RPCPAYMENTTYPE_VOUCHER, RPCPAYMENTTYPE_INVOICE,
// RPCPAYMENTTYPE_BLIND

TEST_F(Test_Rpc, Move_Funds)
{
    auto command = init(proto::RPCCOMMAND_MOVEFUNDS);
    command.set_session(0);

    auto& manager = ot_.Client(0);
    auto nym3 = manager.Wallet().NymByIDPartialMatch(TEST_NYM_3);

    ASSERT_TRUE(bool(nym3));

    auto movefunds = command.mutable_movefunds();

    ASSERT_NE(nullptr, movefunds);

    movefunds->set_version(MOVEFUNDS_VERSION);
    movefunds->set_type(proto::RPCPAYMENTTYPE_TRANSFER);
    movefunds->set_sourceaccount(nym3_account1_id_);
    movefunds->set_destinationaccount(nym3_account2_id_);
    movefunds->set_memo("Move_Funds test");
    movefunds->set_amount(25);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    process_receipt_1(
        manager,
        Identifier::Factory(server_id_),
        nym3->ID(),
        Identifier::Factory(nym3_account2_id_));
    process_receipt_1(
        manager,
        Identifier::Factory(server_id_),
        nym3->ID(),
        Identifier::Factory(nym3_account1_id_));

    {
        const auto account =
            manager.Wallet().Account(Identifier::Factory(nym3_account1_id_));

        ASSERT_TRUE(account);

        ASSERT_EQ(50, account.get().GetBalance());
    }

    {
        const auto account =
            manager.Wallet().Account(Identifier::Factory(nym3_account2_id_));

        ASSERT_TRUE(account);

        ASSERT_EQ(25, account.get().GetBalance());
    }
}

TEST_F(Test_Rpc, Get_Account_Balance)
{
    auto command = init(proto::RPCCOMMAND_GETACCOUNTBALANCE);
    command.set_session(0);

    auto& manager = ot_.Client(0);

    command.add_identifier(nym3_account2_id_);

    auto response = ot_.RPC(command);

    ASSERT_TRUE(proto::Validate(response, VERBOSE));

    ASSERT_EQ(1, response.status_size());
    ASSERT_EQ(proto::RPCRESPONSE_SUCCESS, response.status(0).code());
    ASSERT_EQ(1, response.version());
    ASSERT_STREQ(command.cookie().c_str(), response.cookie().c_str());
    ASSERT_EQ(command.type(), response.type());

    ASSERT_TRUE(0 != response.balance_size());

    auto& accountdata = *response.balance().cbegin();
    ASSERT_EQ(1, accountdata.version());
    ASSERT_EQ(nym3_account2_id_, accountdata.id());

    ASSERT_TRUE(accountdata.label().empty());

    const auto account =
        manager.Wallet().Account(Identifier::Factory(nym3_account2_id_));
    ASSERT_TRUE(bool(account));

    ASSERT_EQ(
        account.get().GetInstrumentDefinitionID().str(), accountdata.unit());

    ASSERT_TRUE(account.get().VerifyOwnerByID(
        Identifier::Factory(accountdata.owner())));

    auto issuerid =
        manager.Storage().AccountIssuer(Identifier::Factory(nym3_account2_id_));
    ASSERT_EQ(issuerid->str(), accountdata.issuer());

    ASSERT_EQ(account.get().GetBalance(), accountdata.balance());
    ASSERT_EQ(account.get().GetBalance(), accountdata.pendingbalance());

    ASSERT_EQ(25, accountdata.balance());
}

}  // namespace
