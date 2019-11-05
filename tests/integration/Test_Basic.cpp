// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"
#include "integration/Helpers.hpp"

#define UNIT_DEFINITION_CONTRACT_VERSION 2
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME "dollars"
#define UNIT_DEFINITION_SYMBOL "$"
#define UNIT_DEFINITION_TLA "USD"
#define UNIT_DEFINITION_POWER 2
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME "cents"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::proto::CITEMTYPE_USD
#define CHEQUE_AMOUNT_1 100
#define CHEQUE_AMOUNT_2 75
#define CHEQUE_MEMO "memo"

namespace
{
class Integration : public ::testing::Test
{
public:
    static Callbacks cb_alex_;
    static Callbacks cb_bob_;
    static Issuer issuer_data_;
    static const StateMap state_;
    static int msg_count_;
    static std::map<int, std::string> message_;
    static ot::OTUnitID unit_id_;

    const ot::api::client::Manager& api_alex_;
    const ot::api::client::Manager& api_bob_;
    const ot::api::client::Manager& api_issuer_;
    const ot::api::server::Manager& api_server_1_;
    ot::OTZMQSubscribeSocket alex_ui_update_listener_;
    ot::OTZMQSubscribeSocket bob_ui_update_listener_;

    Integration()
        : api_alex_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
        , api_bob_(ot::Context().StartClient(OTTestEnvironment::test_args_, 1))
        , api_issuer_(
              ot::Context().StartClient(OTTestEnvironment::test_args_, 2))
        , api_server_1_(
              ot::Context().StartServer(OTTestEnvironment::test_args_, 0, true))
        , alex_ui_update_listener_(
              api_alex_.ZeroMQ().SubscribeSocket(cb_alex_.callback_))
        , bob_ui_update_listener_(
              api_bob_.ZeroMQ().SubscribeSocket(cb_bob_.callback_))
    {
        subscribe_sockets();

        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(alex_).init(api_alex_, server_1_);
        const_cast<User&>(bob_).init(api_bob_, server_1_);
        const_cast<User&>(issuer_).init(api_issuer_, server_1_);
    }

    void subscribe_sockets()
    {
        ASSERT_TRUE(alex_ui_update_listener_->Start(
            api_alex_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(bob_ui_update_listener_->Start(
            api_bob_.Endpoints().WidgetUpdate()));
    }
};

int Integration::msg_count_ = 0;
std::map<int, std::string> Integration::message_{};
ot::OTUnitID Integration::unit_id_{ot::identifier::UnitDefinition::Factory()};
Callbacks Integration::cb_alex_{alex_.name_};
Callbacks Integration::cb_bob_{bob_.name_};
Issuer Integration::issuer_data_{};
const StateMap Integration::state_{
    {alex_.name_,
     {
         {Widget::Profile,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().Profile(alex_.nym_id_);

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_EQ(
                       widget.PaymentCode(), alex_.PaymentCode()->asBase58());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_EQ(widget.DisplayName(), alex_.name_);

                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::ContactList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ContactList(alex_.nym_id_);
                   const auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_TRUE(
                       row->DisplayName() == alex_.name_ ||
                       row->DisplayName() == "Owner");
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(alex_.SetContact(alex_.name_, row->ContactID()));
                   EXPECT_FALSE(alex_.Contact(alex_.name_).empty());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ContactList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(alex_.SetContact(bob_.name_, row->ContactID()));
                   EXPECT_FALSE(alex_.Contact(bob_.name_).empty());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ContactList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), issuer_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("I", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(
                       alex_.SetContact(issuer_.name_, row->ContactID()));
                   EXPECT_FALSE(alex_.Contact(issuer_.name_).empty());

                   return true;
               }},
          }},
         {Widget::MessagableList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().MessagableList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().MessagableList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().MessagableList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), issuer_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("I", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::PayableListBTC,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().PayableList(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().PayableList(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::PayableListBCH,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().PayableList(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().PayableList(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().PayableList(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), issuer_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::ActivitySummary,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ActivitySummary(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_];
                   const auto& widget =
                       alex_.api_->UI().ActivitySummary(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget =
                       alex_.api_->UI().ActivitySummary(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {3,
               []() -> bool {
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget =
                       alex_.api_->UI().ActivitySummary(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), issuer_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("Received cheque", row->Text().c_str());
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(ot::StorageBox::INCOMINGCHEQUE, row->Type());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {4,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ActivitySummary(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), "Sent cheque for dollars 0.75");
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::OUTGOINGCHEQUE);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), issuer_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("Received cheque", row->Text().c_str());
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(ot::StorageBox::INCOMINGCHEQUE, row->Type());

                   return true;
               }},
          }},
         {Widget::ActivityThreadBob,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().ActivityThread(
                       alex_.nym_id_, alex_.Contact(bob_.name_));
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_];
                   const auto& widget = alex_.api_->UI().ActivityThread(
                       alex_.nym_id_, alex_.Contact(bob_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_ - 1];
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget = alex_.api_->UI().ActivityThread(
                       alex_.nym_id_, alex_.Contact(bob_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {3,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_ - 1];
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget = alex_.api_->UI().ActivityThread(
                       alex_.nym_id_, alex_.Contact(bob_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->Amount(), CHEQUE_AMOUNT_2);
                   EXPECT_EQ(row->DisplayAmount(), "dollars 0.75");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), CHEQUE_MEMO);
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), "Sent cheque for dollars 0.75");
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::OUTGOINGCHEQUE);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::ActivityThreadIssuer,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().ActivityThread(
                       alex_.nym_id_, alex_.Contact(issuer_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   bool loading{true};

                   // This allows the test to work correctly in valgrind when
                   // loading is unusually slow
                   while (loading) {
                       row = widget.First();
                       loading = row->Loading();
                   }

                   EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
                   EXPECT_FALSE(row->Loading());
                   EXPECT_STREQ(CHEQUE_MEMO, row->Memo().c_str());
                   EXPECT_FALSE(row->Pending());
                   EXPECT_STREQ(
                       "Received cheque for dollars 1.00", row->Text().c_str());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(ot::StorageBox::INCOMINGCHEQUE, row->Type());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::AccountSummaryBTC,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().AccountSummary(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountSummaryBCH,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().AccountSummary(
                       alex_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountSummaryUSD,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().AccountSummary(
                       alex_.nym_id_, ot::proto::CITEMTYPE_USD);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountActivityUSD,
          {
              {0,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().AccountActivity(
                       alex_.nym_id_, alex_.Account(UNIT_DEFINITION_TLA));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
                   EXPECT_EQ(1, row->Contacts().size());

                   if (0 < row->Contacts().size()) {
                       EXPECT_EQ(
                           alex_.Contact(issuer_.name_).str(),
                           *row->Contacts().begin());
                   }

                   EXPECT_EQ("dollars 1.00", row->DisplayAmount());
                   EXPECT_EQ(CHEQUE_MEMO, row->Memo());
                   EXPECT_FALSE(row->Workflow().empty());
                   EXPECT_EQ("Received cheque #510 from Issuer", row->Text());
                   EXPECT_EQ(ot::StorageBox::INCOMINGCHEQUE, row->Type());
                   EXPECT_FALSE(row->UUID().empty());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget = alex_.api_->UI().AccountActivity(
                       alex_.nym_id_, alex_.Account(UNIT_DEFINITION_TLA));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(-1 * CHEQUE_AMOUNT_2, row->Amount());
                   EXPECT_EQ(1, row->Contacts().size());

                   if (0 < row->Contacts().size()) {
                       EXPECT_EQ(
                           alex_.Contact(bob_.name_).str(),
                           *row->Contacts().begin());
                   }

                   EXPECT_EQ("-dollars 0.75", row->DisplayAmount());
                   EXPECT_EQ(CHEQUE_MEMO, row->Memo());
                   EXPECT_FALSE(row->Workflow().empty());
                   EXPECT_EQ("Wrote cheque #721 for Bob", row->Text());
                   EXPECT_EQ(ot::StorageBox::OUTGOINGCHEQUE, row->Type());
                   EXPECT_FALSE(row->UUID().empty());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
                   EXPECT_EQ(1, row->Contacts().size());

                   if (0 < row->Contacts().size()) {
                       EXPECT_EQ(
                           alex_.Contact(issuer_.name_).str(),
                           *row->Contacts().begin());
                   }

                   EXPECT_EQ("dollars 1.00", row->DisplayAmount());
                   EXPECT_EQ(CHEQUE_MEMO, row->Memo());
                   EXPECT_FALSE(row->Workflow().empty());
                   EXPECT_EQ("Received cheque #510 from Issuer", row->Text());
                   EXPECT_EQ(ot::StorageBox::INCOMINGCHEQUE, row->Type());
                   EXPECT_FALSE(row->UUID().empty());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::AccountList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().AccountList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   auto reason = alex_.Reason();
                   const auto& widget =
                       alex_.api_->UI().AccountList(alex_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   alex_.SetAccount(UNIT_DEFINITION_TLA, row->AccountID());

                   EXPECT_FALSE(alex_.Account(UNIT_DEFINITION_TLA).empty());
                   EXPECT_EQ(unit_id_->str(), row->ContractID());
                   EXPECT_STREQ("dollars 1.00", row->DisplayBalance().c_str());
                   EXPECT_STREQ("", row->Name().c_str());
                   EXPECT_EQ(server_1_.id_->str(), row->NotaryID());
                   EXPECT_EQ(
                       server_1_.Contract()->EffectiveName(reason),
                       row->NotaryName());
                   EXPECT_EQ(ot::AccountType::Custodial, row->Type());
                   EXPECT_EQ(ot::proto::CITEMTYPE_USD, row->Unit());

                   return true;
               }},
          }},
         {Widget::ContactIssuer,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().Contact(alex_.Contact(issuer_.name_));

                   EXPECT_EQ(
                       alex_.Contact(issuer_.name_).str(), widget.ContactID());
                   EXPECT_EQ(std::string(issuer_.name_), widget.DisplayName());
                   EXPECT_EQ(issuer_.payment_code_, widget.PaymentCode());

                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   return true;
               }},
          }},
     }},
    {bob_.name_,
     {
         {Widget::Profile,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().Profile(bob_.nym_id_);

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_EQ(
                       widget.PaymentCode(), bob_.PaymentCode()->asBase58());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_EQ(widget.DisplayName(), bob_.name_);

                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::ContactList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().ContactList(bob_.nym_id_);
                   const auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_TRUE(
                       row->DisplayName() == bob_.name_ ||
                       row->DisplayName() == "Owner");
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(bob_.SetContact(bob_.name_, row->ContactID()));
                   EXPECT_FALSE(bob_.Contact(bob_.name_).empty());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().ContactList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(bob_.SetContact(alex_.name_, row->ContactID()));
                   EXPECT_FALSE(bob_.Contact(alex_.name_).empty());

                   return true;
               }},
          }},
         {Widget::MessagableList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().MessagableList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().MessagableList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::PayableListBTC,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().PayableList(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().PayableList(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_EQ(row->DisplayName(), bob_.name_);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::PayableListBCH,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().PayableList(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().PayableList(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Last());

                   // TODO why isn't Bob in this list?

                   return true;
               }},
          }},
         {Widget::ActivitySummary,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().ActivitySummary(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_];
                   const auto& widget =
                       bob_.api_->UI().ActivitySummary(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget =
                       bob_.api_->UI().ActivitySummary(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {3,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().ActivitySummary(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_EQ(row->ImageURI(), "");
                   EXPECT_EQ(row->Text(), "Received cheque");
                   EXPECT_FALSE(row->ThreadID().empty());
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::INCOMINGCHEQUE);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::ActivityThreadAlice,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().ActivityThread(
                       bob_.nym_id_, bob_.Contact(alex_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());
                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), message_.at(msg_count_));
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_ - 1];
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget = bob_.api_->UI().ActivityThread(
                       bob_.nym_id_, bob_.Contact(alex_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   bool loading{true};

                   // This allows the test to work correctly in valgrind when
                   // loading is unusually slow
                   while (loading) {
                       row = widget.First();
                       row = widget.Next();
                       loading = row->Loading();
                   }

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& firstMessage = message_[msg_count_ - 1];
                   const auto& secondMessage = message_[msg_count_];
                   const auto& widget = bob_.api_->UI().ActivityThread(
                       bob_.nym_id_, bob_.Contact(alex_.name_));
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), firstMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILINBOX);
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   bool loading{true};

                   EXPECT_EQ(row->Amount(), 0);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), "");
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), secondMessage);
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::MAILOUTBOX);
                   EXPECT_FALSE(row->Last());

                   // This allows the test to work correctly in valgrind when
                   // loading is unusually slow
                   while (loading) {
                       row = widget.First();
                       row = widget.Next();
                       row = widget.Next();
                       loading = row->Loading();
                   }

                   EXPECT_EQ(row->Amount(), CHEQUE_AMOUNT_2);
                   EXPECT_EQ(row->DisplayAmount(), "");
                   EXPECT_FALSE(row->Loading());
                   EXPECT_EQ(row->Memo(), CHEQUE_MEMO);
                   EXPECT_FALSE(row->Pending());
                   EXPECT_EQ(row->Text(), "Received cheque");
                   EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
                   EXPECT_EQ(row->Type(), ot::StorageBox::INCOMINGCHEQUE);
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
         {Widget::AccountSummaryBTC,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().AccountSummary(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BTC);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountSummaryBCH,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().AccountSummary(
                       bob_.nym_id_, ot::proto::CITEMTYPE_BCH);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountSummaryUSD,
          {
              {0,
               []() -> bool {
                   const auto& widget = bob_.api_->UI().AccountSummary(
                       bob_.nym_id_, ot::proto::CITEMTYPE_USD);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
         {Widget::AccountList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().AccountList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
          }},
     }},
};

TEST_F(Integration, instantiate_ui_objects)
{
    auto future1 = std::future<bool>{};
    auto future2 = std::future<bool>{};
    auto future3 = std::future<bool>{};
    auto future4 = std::future<bool>{};
    auto future5 = std::future<bool>{};
    auto future6 = std::future<bool>{};

    {
        ot::Lock lock{cb_alex_.callback_lock_};
        future1 = cb_alex_.RegisterWidget(
            lock,
            Widget::Profile,
            api_alex_.UI().Profile(alex_.nym_id_).WidgetID(),
            2,
            state_.at(alex_.name_).at(Widget::Profile).at(0));
        cb_alex_.RegisterWidget(
            lock,
            Widget::ActivitySummary,
            api_alex_.UI().ActivitySummary(alex_.nym_id_).WidgetID());
        future3 = cb_alex_.RegisterWidget(
            lock,
            Widget::ContactList,
            api_alex_.UI().ContactList(alex_.nym_id_).WidgetID(),
            1,
            state_.at(alex_.name_).at(Widget::ContactList).at(0));
        cb_alex_.RegisterWidget(
            lock,
            Widget::PayableListBCH,
            api_alex_.UI()
                .PayableList(alex_.nym_id_, ot::proto::CITEMTYPE_BCH)
                .WidgetID());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        future5 =
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            cb_alex_.RegisterWidget(
                lock,
                Widget::PayableListBTC,
                api_alex_.UI()
                    .PayableList(alex_.nym_id_, ot::proto::CITEMTYPE_BTC)
                    .WidgetID()
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                    ,
                1,
                state_.at(alex_.name_).at(Widget::PayableListBTC).at(0)
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            );
        cb_alex_.RegisterWidget(
            lock,
            Widget::AccountSummaryBTC,
            api_alex_.UI()
                .AccountSummary(alex_.nym_id_, ot::proto::CITEMTYPE_BTC)
                .WidgetID());
        cb_alex_.RegisterWidget(
            lock,
            Widget::AccountSummaryBCH,
            api_alex_.UI()
                .AccountSummary(alex_.nym_id_, ot::proto::CITEMTYPE_BCH)
                .WidgetID());
        cb_alex_.RegisterWidget(
            lock,
            Widget::AccountSummaryUSD,
            api_alex_.UI()
                .AccountSummary(alex_.nym_id_, ot::proto::CITEMTYPE_USD)
                .WidgetID());
        cb_alex_.RegisterWidget(
            lock,
            Widget::MessagableList,
            api_alex_.UI().MessagableList(alex_.nym_id_).WidgetID());
        cb_alex_.RegisterWidget(
            lock,
            Widget::AccountList,
            api_alex_.UI().AccountList(alex_.nym_id_).WidgetID());
    }
    {
        ot::Lock lock{cb_alex_.callback_lock_};
        future2 = cb_bob_.RegisterWidget(
            lock,
            Widget::Profile,
            api_bob_.UI().Profile(bob_.nym_id_).WidgetID(),
            2,
            state_.at(bob_.name_).at(Widget::Profile).at(0));
        cb_bob_.RegisterWidget(
            lock,
            Widget::ActivitySummary,
            api_bob_.UI().ActivitySummary(bob_.nym_id_).WidgetID());
        future4 = cb_bob_.RegisterWidget(
            lock,
            Widget::ContactList,
            api_bob_.UI().ContactList(bob_.nym_id_).WidgetID(),
            1,
            state_.at(bob_.name_).at(Widget::ContactList).at(0));
        cb_bob_.RegisterWidget(
            lock,
            Widget::PayableListBCH,
            api_bob_.UI()
                .PayableList(bob_.nym_id_, ot::proto::CITEMTYPE_BCH)
                .WidgetID());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        future6 =
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            cb_bob_.RegisterWidget(
                lock,
                Widget::PayableListBTC,
                api_bob_.UI()
                    .PayableList(bob_.nym_id_, ot::proto::CITEMTYPE_BTC)
                    .WidgetID()
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                    ,
                1,
                state_.at(bob_.name_).at(Widget::PayableListBTC).at(0)
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
            );
        cb_bob_.RegisterWidget(
            lock,
            Widget::AccountSummaryBTC,
            api_bob_.UI()
                .AccountSummary(bob_.nym_id_, ot::proto::CITEMTYPE_BTC)
                .WidgetID());
        cb_bob_.RegisterWidget(
            lock,
            Widget::AccountSummaryBCH,
            api_bob_.UI()
                .AccountSummary(bob_.nym_id_, ot::proto::CITEMTYPE_BCH)
                .WidgetID());
        cb_bob_.RegisterWidget(
            lock,
            Widget::AccountSummaryUSD,
            api_bob_.UI()
                .AccountSummary(bob_.nym_id_, ot::proto::CITEMTYPE_USD)
                .WidgetID());
        cb_bob_.RegisterWidget(
            lock,
            Widget::MessagableList,
            api_bob_.UI().MessagableList(bob_.nym_id_).WidgetID());
        cb_bob_.RegisterWidget(
            lock,
            Widget::AccountList,
            api_bob_.UI().AccountList(bob_.nym_id_).WidgetID());
    }

    EXPECT_EQ(10, cb_alex_.Count());
    EXPECT_EQ(10, cb_bob_.Count());

    EXPECT_TRUE(future1.get());
    EXPECT_TRUE(future2.get());
    EXPECT_TRUE(future3.get());
    EXPECT_TRUE(future4.get());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(future5.get());
    EXPECT_TRUE(future6.get());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
}

TEST_F(Integration, initial_state)
{
    EXPECT_TRUE(ot::contract::Unit::ValidUnits(1).empty());
    EXPECT_EQ(
        ot::contract::Unit::ValidUnits(2),
        ot::proto::AllowedItemTypes().at(
            {6, ot::proto::CONTACTSECTION_CONTRACT}));

    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::ActivitySummary).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::MessagableList).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::PayableListBCH).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::AccountSummaryBTC).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::AccountSummaryBCH).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::AccountSummaryUSD).at(0)());
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::AccountList).at(0)());

    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::ActivitySummary).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::MessagableList).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::PayableListBCH).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::AccountSummaryBTC).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::AccountSummaryBCH).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::AccountSummaryUSD).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::AccountList).at(0)());
}

TEST_F(Integration, payment_codes)
{
    auto alex = api_alex_.Wallet().mutable_Nym(alex_.nym_id_, alex_.Reason());
    auto bob = api_bob_.Wallet().mutable_Nym(bob_.nym_id_, bob_.Reason());
    auto issuer =
        api_issuer_.Wallet().mutable_Nym(issuer_.nym_id_, issuer_.Reason());

    EXPECT_EQ(ot::proto::CITEMTYPE_INDIVIDUAL, alex.Type());
    EXPECT_EQ(ot::proto::CITEMTYPE_INDIVIDUAL, bob.Type());
    EXPECT_EQ(ot::proto::CITEMTYPE_INDIVIDUAL, issuer.Type());

    auto alexScopeSet = alex.SetScope(
        ot::proto::CITEMTYPE_INDIVIDUAL, alex_.name_, true, alex_.Reason());
    auto bobScopeSet = bob.SetScope(
        ot::proto::CITEMTYPE_INDIVIDUAL, bob_.name_, true, bob_.Reason());
    auto issuerScopeSet = issuer.SetScope(
        ot::proto::CITEMTYPE_INDIVIDUAL, issuer_.name_, true, issuer_.Reason());

    EXPECT_TRUE(alexScopeSet);
    EXPECT_TRUE(bobScopeSet);
    EXPECT_TRUE(issuerScopeSet);

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_FALSE(alex_.payment_code_.empty());
    EXPECT_FALSE(bob_.payment_code_.empty());
    EXPECT_FALSE(issuer_.payment_code_.empty());

    alex.AddPaymentCode(
        alex_.payment_code_,
        ot::proto::CITEMTYPE_BTC,
        true,
        true,
        alex_.Reason());
    bob.AddPaymentCode(
        bob_.payment_code_,
        ot::proto::CITEMTYPE_BTC,
        true,
        true,
        bob_.Reason());
    issuer.AddPaymentCode(
        issuer_.payment_code_,
        ot::proto::CITEMTYPE_BTC,
        true,
        true,
        issuer_.Reason());
    alex.AddPaymentCode(
        alex_.payment_code_,
        ot::proto::CITEMTYPE_BCH,
        true,
        true,
        alex_.Reason());
    bob.AddPaymentCode(
        bob_.payment_code_,
        ot::proto::CITEMTYPE_BCH,
        true,
        true,
        bob_.Reason());
    issuer.AddPaymentCode(
        issuer_.payment_code_,
        ot::proto::CITEMTYPE_BCH,
        true,
        true,
        issuer_.Reason());

    EXPECT_FALSE(alex.PaymentCode(ot::proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(bob.PaymentCode(ot::proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(issuer.PaymentCode(ot::proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(alex.PaymentCode(ot::proto::CITEMTYPE_BCH).empty());
    EXPECT_FALSE(bob.PaymentCode(ot::proto::CITEMTYPE_BCH).empty());
    EXPECT_FALSE(issuer.PaymentCode(ot::proto::CITEMTYPE_BCH).empty());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

    alex.Release();
    bob.Release();
    issuer.Release();
}

TEST_F(Integration, introduction_server)
{
    api_alex_.OTX().StartIntroductionServer(alex_.nym_id_);
    api_bob_.OTX().StartIntroductionServer(bob_.nym_id_);
    auto task1 =
        api_alex_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true);
    auto task2 =
        api_bob_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(
        ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, task1.second.get().first);
    EXPECT_EQ(
        ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, task2.second.get().first);

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, add_contact_preconditions)
{
    // Neither alex nor bob should know about each other yet
    auto alex = api_bob_.Wallet().Nym(alex_.nym_id_, bob_.Reason());
    auto bob = api_alex_.Wallet().Nym(bob_.nym_id_, alex_.Reason());

    EXPECT_FALSE(alex);
    EXPECT_FALSE(bob);
}

TEST_F(Integration, add_contact_Bob_To_Alex)
{
    auto contactListDone = cb_alex_.SetCallback(
        Widget::ContactList,
        2,
        state_.at(alex_.name_).at(Widget::ContactList).at(1));
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto payableBTCListDone = cb_alex_.SetCallback(
        Widget::PayableListBTC,
        2,
        state_.at(alex_.name_).at(Widget::PayableListBTC).at(1));
    auto payableBCHListDone = cb_alex_.SetCallback(
        Widget::PayableListBCH,
        1,
        state_.at(alex_.name_).at(Widget::PayableListBCH).at(1));
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto messagableListDone = cb_alex_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(alex_.name_).at(Widget::MessagableList).at(1));

    // Add the contact
    alex_.api_->UI()
        .ContactList(alex_.nym_id_)
        .AddContact(bob_.name_, bob_.payment_code_, bob_.nym_id_->str());

    EXPECT_TRUE(contactListDone.get());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(payableBTCListDone.get());
    EXPECT_TRUE(payableBCHListDone.get());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(messagableListDone.get());
}

TEST_F(Integration, activity_thread_alex_bob)
{
    ot::Lock lock(cb_alex_.callback_lock_);
    auto done = cb_alex_.RegisterWidget(
        lock,
        Widget::ActivityThreadBob,
        api_alex_.UI()
            .ActivityThread(alex_.nym_id_, alex_.Contact(bob_.name_))
            .WidgetID(),
        2,
        state_.at(alex_.name_).at(Widget::ActivityThreadBob).at(0));

    EXPECT_EQ(11, cb_alex_.Count());

    lock.unlock();

    EXPECT_TRUE(done.get());
}

TEST_F(Integration, send_message_from_Alex_to_Bob_1)
{
    const auto& from_client = api_alex_;
    const auto messageID = ++msg_count_;
    std::stringstream text{};
    text << alex_.name_ << " messaged " << bob_.name_ << " with message #"
         << std::to_string(messageID);
    auto& firstMessage = message_[messageID];
    firstMessage = text.str();

    auto alexActivitySummaryDone = cb_alex_.SetCallback(
        Widget::ActivitySummary,
        2,
        state_.at(alex_.name_).at(Widget::ActivitySummary).at(1));
    auto bobActivitySummaryDone = cb_bob_.SetCallback(
        Widget::ActivitySummary,
        2,
        state_.at(bob_.name_).at(Widget::ActivitySummary).at(1));
    auto alexActivityThreadDone = cb_alex_.SetCallback(
        Widget::ActivityThreadBob,
        7,
        state_.at(alex_.name_).at(Widget::ActivityThreadBob).at(1));
    auto bobContactListDone = cb_bob_.SetCallback(
        Widget::ContactList,
        3,
        state_.at(bob_.name_).at(Widget::ContactList).at(1));
    auto bobMessagableListDone = cb_bob_.SetCallback(
        Widget::MessagableList,
        2,
        state_.at(bob_.name_).at(Widget::MessagableList).at(1));
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto bobPayableListBTCDone = cb_bob_.SetCallback(
        Widget::PayableListBTC,
        1,
        state_.at(bob_.name_).at(Widget::PayableListBTC).at(1));
    auto bobPayableListBCHDone = cb_bob_.SetCallback(
        Widget::PayableListBCH,
        1,
        state_.at(bob_.name_).at(Widget::PayableListBCH).at(1));
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

    const auto& conversation = from_client.UI().ActivityThread(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    conversation.SetDraft(firstMessage);

    EXPECT_EQ(conversation.GetDraft(), firstMessage);

    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
    EXPECT_TRUE(alexActivitySummaryDone.get());
    EXPECT_TRUE(alexActivityThreadDone.get());
    EXPECT_TRUE(bobContactListDone.get());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(bobPayableListBTCDone.get());
    EXPECT_TRUE(bobPayableListBCHDone.get());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(bobMessagableListDone.get());
    EXPECT_TRUE(bobActivitySummaryDone.get());
}

TEST_F(Integration, activity_thread_bob_alex)
{
    ot::Lock lock(cb_bob_.callback_lock_);
    auto done = cb_bob_.RegisterWidget(
        lock,
        Widget::ActivityThreadAlice,
        api_bob_.UI()
            .ActivityThread(bob_.nym_id_, bob_.Contact(alex_.name_))
            .WidgetID(),
        3,
        state_.at(bob_.name_).at(Widget::ActivityThreadAlice).at(0));

    EXPECT_EQ(11, cb_bob_.Count());

    lock.unlock();

    EXPECT_TRUE(done.get());
}

TEST_F(Integration, send_message_from_Bob_to_Alex_2)
{
    const auto& from_client = api_bob_;
    const auto messageID = ++msg_count_;
    std::stringstream text{};
    text << bob_.name_ << " messaged " << alex_.name_ << " with message #"
         << std::to_string(messageID);
    auto& secondMessage = message_[messageID];
    secondMessage = text.str();

    auto alexActivitySummaryDone = cb_alex_.SetCallback(
        Widget::ActivitySummary,
        4,
        state_.at(alex_.name_).at(Widget::ActivitySummary).at(2));
    auto alexActivityThreadDone = cb_alex_.SetCallback(
        Widget::ActivityThreadBob,
        10,
        state_.at(alex_.name_).at(Widget::ActivityThreadBob).at(2));
    auto bobActivitySummaryDone = cb_bob_.SetCallback(
        Widget::ActivitySummary,
        4,
        state_.at(bob_.name_).at(Widget::ActivitySummary).at(2));
    auto bobActivityThreadDone = cb_bob_.SetCallback(
        Widget::ActivityThreadAlice,
        8,
        state_.at(bob_.name_).at(Widget::ActivityThreadAlice).at(1));

    const auto& conversation = from_client.UI().ActivityThread(
        bob_.nym_id_, bob_.Contact(alex_.name_));
    conversation.SetDraft(secondMessage);

    EXPECT_EQ(conversation.GetDraft(), secondMessage);

    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
    EXPECT_TRUE(bobActivitySummaryDone.get());
    EXPECT_TRUE(bobActivityThreadDone.get());
    EXPECT_TRUE(alexActivitySummaryDone.get());
    EXPECT_TRUE(alexActivityThreadDone.get());
}

TEST_F(Integration, issue_dollars)
{
    const auto contract = api_issuer_.Wallet().UnitDefinition(
        issuer_.nym_id_->str(),
        UNIT_DEFINITION_CONTRACT_NAME,
        UNIT_DEFINITION_TERMS,
        UNIT_DEFINITION_PRIMARY_UNIT_NAME,
        UNIT_DEFINITION_SYMBOL,
        UNIT_DEFINITION_TLA,
        UNIT_DEFINITION_POWER,
        UNIT_DEFINITION_FRACTIONAL_UNIT_NAME,
        UNIT_DEFINITION_UNIT_OF_ACCOUNT,
        issuer_.Reason());

    EXPECT_EQ(UNIT_DEFINITION_CONTRACT_VERSION, contract->Version());
    EXPECT_EQ(ot::proto::UNITTYPE_CURRENCY, contract->Type());
    EXPECT_EQ(UNIT_DEFINITION_UNIT_OF_ACCOUNT, contract->UnitOfAccount());
    EXPECT_TRUE(unit_id_->empty());

    unit_id_->Assign(contract->ID());

    EXPECT_FALSE(unit_id_->empty());

    {
        auto issuer =
            api_issuer_.Wallet().mutable_Nym(issuer_.nym_id_, issuer_.Reason());
        issuer.AddPreferredOTServer(
            server_1_.id_->str(), true, issuer_.Reason());
    }

    auto task = api_issuer_.OTX().IssueUnitDefinition(
        issuer_.nym_id_, server_1_.id_, unit_id_, ot::proto::CITEMTYPE_USD);
    auto& [taskID, future] = task;
    const auto result = future.get();

    EXPECT_NE(0, taskID);
    EXPECT_EQ(ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, result.first);
    ASSERT_TRUE(result.second);

    EXPECT_TRUE(issuer_.SetAccount(
        UNIT_DEFINITION_TLA, result.second->m_strAcctID->Get()));
    EXPECT_FALSE(issuer_.Account(UNIT_DEFINITION_TLA).empty());

    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();

    {
        const auto pNym =
            api_issuer_.Wallet().Nym(issuer_.nym_id_, issuer_.Reason());

        ASSERT_TRUE(pNym);

        const auto& nym = *pNym;
        const auto& claims = nym.Claims();
        const auto pSection =
            claims.Section(ot::proto::CONTACTSECTION_CONTRACT);

        ASSERT_TRUE(pSection);

        const auto& section = *pSection;
        const auto pGroup = section.Group(ot::proto::CITEMTYPE_USD);

        ASSERT_TRUE(pGroup);

        const auto& group = *pGroup;
        const auto& pClaim = group.PrimaryClaim();

        EXPECT_EQ(1, group.Size());
        ASSERT_TRUE(pClaim);

        const auto& claim = *pClaim;

        EXPECT_EQ(claim.Value(), unit_id_->str());
    }
}

TEST_F(Integration, add_alex_contact_to_issuer)
{
    EXPECT_TRUE(issuer_.SetContact(
        alex_.name_,
        api_issuer_.Contacts().NymToContact(alex_.nym_id_, issuer_.Reason())));
    EXPECT_FALSE(issuer_.Contact(alex_.name_).empty());

    api_issuer_.OTX().Refresh();
    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, pay_alex)
{
    auto contactListDone = cb_alex_.SetCallback(
        Widget::ContactList,
        6,
        state_.at(alex_.name_).at(Widget::ContactList).at(2));
    auto messagableListDone = cb_alex_.SetCallback(
        Widget::MessagableList,
        3,
        state_.at(alex_.name_).at(Widget::MessagableList).at(2));
    auto activitySummaryDone = cb_alex_.SetCallback(
        Widget::ActivitySummary,
        6,
        state_.at(alex_.name_).at(Widget::ActivitySummary).at(3));
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    auto payableBCHListDone = cb_alex_.SetCallback(
        Widget::PayableListBCH,
        3,
        state_.at(alex_.name_).at(Widget::PayableListBCH).at(2));
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

    auto task = api_issuer_.OTX().SendCheque(
        issuer_.nym_id_,
        issuer_.Account(UNIT_DEFINITION_TLA),
        issuer_.Contact(alex_.name_),
        CHEQUE_AMOUNT_1,
        CHEQUE_MEMO);
    auto& [taskID, future] = task;

    ASSERT_NE(0, taskID);
    EXPECT_EQ(ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, future.get().first);

    api_alex_.OTX().Refresh();
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();

    EXPECT_TRUE(contactListDone.get());
    EXPECT_TRUE(messagableListDone.get());
    EXPECT_TRUE(activitySummaryDone.get());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPECT_TRUE(payableBCHListDone.get());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47
}

TEST_F(Integration, issuer_claims)
{
    const auto pNym = api_alex_.Wallet().Nym(issuer_.nym_id_, alex_.Reason());

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();
    const auto pSection = claims.Section(ot::proto::CONTACTSECTION_CONTRACT);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;
    const auto pGroup = section.Group(ot::proto::CITEMTYPE_USD);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto& pClaim = group.PrimaryClaim();

    EXPECT_EQ(1, group.Size());
    ASSERT_TRUE(pClaim);

    const auto& claim = *pClaim;

    EXPECT_EQ(claim.Value(), unit_id_->str());
}

TEST_F(Integration, contact_alex_issuer)
{
    ot::Lock lock{cb_alex_.callback_lock_};
    auto issuerContactDone = cb_alex_.RegisterWidget(
        lock,
        Widget::ContactIssuer,
        api_alex_.UI().Contact(alex_.Contact(issuer_.name_)).WidgetID(),
        3,
        state_.at(alex_.name_).at(Widget::ContactIssuer).at(0));

    EXPECT_EQ(12, cb_alex_.Count());

    lock.unlock();

    EXPECT_TRUE(issuerContactDone.get());
}

TEST_F(Integration, deposit_cheque_alex)
{
    ot::Lock lock{cb_alex_.callback_lock_};
    auto activityThreadDone = cb_alex_.RegisterWidget(
        lock,
        Widget::ActivityThreadIssuer,
        api_alex_.UI()
            .ActivityThread(alex_.nym_id_, alex_.Contact(issuer_.name_))
            .WidgetID(),
        3,
        state_.at(alex_.name_).at(Widget::ActivityThreadIssuer).at(0));

    EXPECT_EQ(13, cb_alex_.Count());

    auto accountListDone = cb_alex_.SetCallback(
        Widget::AccountList,
        4,
        state_.at(alex_.name_).at(Widget::AccountList).at(1));
    auto issuerContactDone = cb_alex_.SetCallback(
        Widget::ContactIssuer,
        9,
        state_.at(alex_.name_).at(Widget::ContactIssuer).at(0));

    lock.unlock();

    EXPECT_TRUE(activityThreadDone.get());

    const auto& thread = alex_.api_->UI().ActivityThread(
        alex_.nym_id_, alex_.Contact(issuer_.name_));
    auto row = thread.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_TRUE(row->Deposit());
    EXPECT_TRUE(accountListDone.get());
    EXPECT_TRUE(issuerContactDone.get());

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, account_activity_alex)
{
    ot::Lock lock{cb_alex_.callback_lock_};
    auto accountActivityDone = cb_alex_.RegisterWidget(
        lock,
        Widget::AccountActivityUSD,
        api_alex_.UI()
            .AccountActivity(alex_.nym_id_, alex_.Account(UNIT_DEFINITION_TLA))
            .WidgetID(),
        4,
        state_.at(alex_.name_).at(Widget::AccountActivityUSD).at(0));

    EXPECT_EQ(14, cb_alex_.Count());

    lock.unlock();

    EXPECT_TRUE(accountActivityDone.get());
}

TEST_F(Integration, process_inbox_issuer)
{
    auto task = api_issuer_.OTX().ProcessInbox(
        issuer_.nym_id_, server_1_.id_, issuer_.Account(UNIT_DEFINITION_TLA));
    auto& [id, future] = task;

    ASSERT_NE(0, id);

    const auto [status, message] = future.get();

    EXPECT_EQ(ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const auto account = api_issuer_.Wallet().Account(
        issuer_.Account(UNIT_DEFINITION_TLA), issuer_.Reason());

    EXPECT_EQ(-1 * CHEQUE_AMOUNT_1, account.get().GetBalance());
}

TEST_F(Integration, pay_bob)
{
    auto alexActivityThreadDone = cb_alex_.SetCallback(
        Widget::ActivityThreadBob,
        15,
        state_.at(alex_.name_).at(Widget::ActivityThreadBob).at(3));
    auto alexAccountActivityDone = cb_alex_.SetCallback(
        Widget::AccountActivityUSD,
        8,
        state_.at(alex_.name_).at(Widget::AccountActivityUSD).at(1));
    auto alexActivitySummaryDone = cb_alex_.SetCallback(
        Widget::ActivitySummary,
        8,
        state_.at(alex_.name_).at(Widget::ActivitySummary).at(4));
    auto bobActivityThreadDone = cb_bob_.SetCallback(
        Widget::ActivityThreadAlice,
        11,
        state_.at(bob_.name_).at(Widget::ActivityThreadAlice).at(2));
    auto bobActivitySummaryDone = cb_bob_.SetCallback(
        Widget::ActivitySummary,
        6,
        state_.at(bob_.name_).at(Widget::ActivitySummary).at(3));
    auto issuerContactDone = cb_alex_.SetCallback(
        Widget::ContactIssuer,
        13,
        state_.at(alex_.name_).at(Widget::ContactIssuer).at(0));

    auto& thread =
        api_alex_.UI().ActivityThread(alex_.nym_id_, alex_.Contact(bob_.name_));
    const auto sent = thread.Pay(
        CHEQUE_AMOUNT_2,
        alex_.Account(UNIT_DEFINITION_TLA),
        CHEQUE_MEMO,
        ot::PaymentType::Cheque);

    EXPECT_TRUE(sent);
    EXPECT_TRUE(alexActivityThreadDone.get());
    EXPECT_TRUE(alexAccountActivityDone.get());
    EXPECT_TRUE(alexActivitySummaryDone.get());
    EXPECT_TRUE(bobActivityThreadDone.get());
    EXPECT_TRUE(bobActivitySummaryDone.get());
    EXPECT_TRUE(issuerContactDone.get());

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, shutdown)
{
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
}
}  // namespace
