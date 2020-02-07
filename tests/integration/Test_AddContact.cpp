// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"
#include "integration/Helpers.hpp"

namespace
{
struct Test_AddContact : public ::testing::Test {
    static Callbacks cb_alex_;
    static Callbacks cb_bob_;
    static Callbacks cb_chris_;
    static const StateMap state_;

    const ot::api::client::Manager& api_alex_;
    const ot::api::client::Manager& api_bob_;
    const ot::api::client::Manager& api_chris_;
    const ot::api::server::Manager& api_server_1_;
    ot::OTZMQSubscribeSocket alex_ui_update_listener_;
    ot::OTZMQSubscribeSocket bob_ui_update_listener_;
    ot::OTZMQSubscribeSocket chris_ui_update_listener_;

    Test_AddContact()
        : api_alex_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
        , api_bob_(ot::Context().StartClient(OTTestEnvironment::test_args_, 1))
        , api_chris_(
              ot::Context().StartClient(OTTestEnvironment::test_args_, 2))
        , api_server_1_(
              ot::Context().StartServer(OTTestEnvironment::test_args_, 0, true))
        , alex_ui_update_listener_(
              api_alex_.ZeroMQ().SubscribeSocket(cb_alex_.callback_))
        , bob_ui_update_listener_(
              api_bob_.ZeroMQ().SubscribeSocket(cb_bob_.callback_))
        , chris_ui_update_listener_(
              api_chris_.ZeroMQ().SubscribeSocket(cb_chris_.callback_))
    {
        subscribe_sockets();

        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(alex_).init(api_alex_, server_1_);
        const_cast<User&>(bob_).init(api_bob_, server_1_);
        const_cast<User&>(chris_).init(api_chris_, server_1_);
    }

    void subscribe_sockets()
    {
        ASSERT_TRUE(alex_ui_update_listener_->Start(
            api_alex_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(bob_ui_update_listener_->Start(
            api_bob_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(chris_ui_update_listener_->Start(
            api_chris_.Endpoints().WidgetUpdate()));
    }
};

Callbacks Test_AddContact::cb_alex_{alex_.name_};
Callbacks Test_AddContact::cb_bob_{bob_.name_};
Callbacks Test_AddContact::cb_chris_{chris_.name_};
const StateMap Test_AddContact::state_{
    {alex_.name_,
     {
         {Widget::ContactList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       alex_.api_->UI().ContactList(alex_.nym_id_);
                   const auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
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

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
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

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("C", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(
                       alex_.SetContact(chris_.name_, row->ContactID()));
                   EXPECT_FALSE(alex_.Contact(chris_.name_).empty());
#else
                   EXPECT_TRUE(row->Last());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

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

                   EXPECT_FALSE(row->ContactID().empty());
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

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("C", row->Section().c_str());
                   EXPECT_TRUE(row->Last());
#else
                   EXPECT_TRUE(row->Last());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

                   return true;
               }},
          }},
     }},
    {bob_.name_,
     {
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

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(bob_.SetContact(alex_.name_, row->ContactID()));
                   EXPECT_FALSE(bob_.Contact(alex_.name_).empty());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().ContactList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("C", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(bob_.SetContact(chris_.name_, row->ContactID()));
                   EXPECT_FALSE(bob_.Contact(chris_.name_).empty());

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

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       bob_.api_->UI().MessagableList(bob_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("C", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
     }},
    {chris_.name_,
     {
         {Widget::ContactList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().ContactList(chris_.nym_id_);
                   const auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_TRUE(
                       row->DisplayName() == chris_.name_ ||
                       row->DisplayName() == "Owner");
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(
                       chris_.SetContact(chris_.name_, row->ContactID()));
                   EXPECT_FALSE(chris_.Contact(chris_.name_).empty());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().ContactList(chris_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(
                       chris_.SetContact(alex_.name_, row->ContactID()));
                   EXPECT_FALSE(chris_.Contact(alex_.name_).empty());
#else
                   EXPECT_TRUE(row->Last());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().ContactList(chris_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == chris_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("ME", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   EXPECT_TRUE(chris_.SetContact(bob_.name_, row->ContactID()));
                   EXPECT_FALSE(chris_.Contact(bob_.name_).empty());

                   return true;
               }},
          }},
         {Widget::MessagableList,
          {
              {0,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().MessagableList(chris_.nym_id_);
                   auto row = widget.First();

                   EXPECT_FALSE(row->Valid());

                   return true;
               }},
              {1,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().MessagableList(chris_.nym_id_);
                   auto row = widget.First();

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_EQ(row->DisplayName(), alex_.name_);
                   EXPECT_TRUE(row->Valid());
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_TRUE(row->Last());
#else
                   EXPECT_FALSE(row->Valid());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

                   return true;
               }},
              {2,
               []() -> bool {
                   const auto& widget =
                       chris_.api_->UI().MessagableList(chris_.nym_id_);
                   auto row = widget.First();

                   EXPECT_TRUE(row->Valid());

                   if (false == row->Valid()) { return false; }

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == alex_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("A", row->Section().c_str());
                   EXPECT_FALSE(row->Last());

                   if (row->Last()) { return false; }

                   row = widget.Next();
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

                   EXPECT_FALSE(row->ContactID().empty());
                   EXPECT_TRUE(row->DisplayName() == bob_.name_);
                   EXPECT_STREQ("", row->ImageURI().c_str());
                   EXPECT_STREQ("B", row->Section().c_str());
                   EXPECT_TRUE(row->Last());

                   return true;
               }},
          }},
     }},
};

TEST_F(Test_AddContact, init_ot) {}

TEST_F(Test_AddContact, init_ui)
{
    auto alexContactList = std::future<bool>{};
    auto bobContactList = std::future<bool>{};
    auto chrisContactList = std::future<bool>{};

    {
        ot::Lock lock{cb_alex_.callback_lock_};
        alexContactList = cb_alex_.RegisterWidget(
            lock,
            Widget::ContactList,
            api_alex_.UI().ContactList(alex_.nym_id_).WidgetID(),
            1,
            state_.at(alex_.name_).at(Widget::ContactList).at(0));
        cb_alex_.RegisterWidget(
            lock,
            Widget::MessagableList,
            api_alex_.UI().MessagableList(alex_.nym_id_).WidgetID());
    }
    {
        ot::Lock lock{cb_bob_.callback_lock_};
        bobContactList = cb_bob_.RegisterWidget(
            lock,
            Widget::ContactList,
            api_bob_.UI().ContactList(bob_.nym_id_).WidgetID(),
            1,
            state_.at(bob_.name_).at(Widget::ContactList).at(0));
        cb_bob_.RegisterWidget(
            lock,
            Widget::MessagableList,
            api_bob_.UI().MessagableList(bob_.nym_id_).WidgetID());
    }
    {
        ot::Lock lock{cb_chris_.callback_lock_};
        chrisContactList = cb_chris_.RegisterWidget(
            lock,
            Widget::ContactList,
            api_chris_.UI().ContactList(chris_.nym_id_).WidgetID(),
            1,
            state_.at(chris_.name_).at(Widget::ContactList).at(0));
        cb_chris_.RegisterWidget(
            lock,
            Widget::MessagableList,
            api_chris_.UI().MessagableList(chris_.nym_id_).WidgetID());
    }

    EXPECT_TRUE(test_future(alexContactList));
    EXPECT_TRUE(test_future(bobContactList));
    EXPECT_TRUE(test_future(chrisContactList));
    EXPECT_TRUE(state_.at(alex_.name_).at(Widget::MessagableList).at(0)());
    EXPECT_TRUE(state_.at(bob_.name_).at(Widget::MessagableList).at(0)());
    EXPECT_TRUE(state_.at(chris_.name_).at(Widget::MessagableList).at(0)());
}

TEST_F(Test_AddContact, introduction_server)
{
    api_alex_.OTX().StartIntroductionServer(alex_.nym_id_);
    api_bob_.OTX().StartIntroductionServer(bob_.nym_id_);
    api_chris_.OTX().StartIntroductionServer(chris_.nym_id_);
    auto alexTask =
        api_alex_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true);
    auto bobTask =
        api_bob_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);
    auto chrisTask =
        api_chris_.OTX().RegisterNymPublic(chris_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, alexTask.first);
    ASSERT_NE(0, bobTask.first);
    ASSERT_NE(0, chrisTask.first);
    EXPECT_EQ(
        ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, alexTask.second.get().first);
    EXPECT_EQ(
        ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS, bobTask.second.get().first);
    EXPECT_EQ(
        ot::proto::LASTREPLYSTATUS_MESSAGESUCCESS,
        chrisTask.second.get().first);

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();
}

TEST_F(Test_AddContact, nymid)
{
    ASSERT_FALSE(bob_.nym_id_->empty());

    auto contactList = cb_alex_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(alex_.name_).at(Widget::ContactList).at(1));
    auto messagableList = cb_alex_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(alex_.name_).at(Widget::MessagableList).at(1));

    const auto& widget = alex_.api_->UI().ContactList(alex_.nym_id_);
    const auto id = widget.AddContact(bob_.name_, bob_.nym_id_->str(), "");

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList, 10));
    EXPECT_TRUE(test_future(messagableList, 10));
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_AddContact, paymentcode)
{
    ASSERT_FALSE(chris_.payment_code_.empty());

    auto contactList = cb_alex_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(alex_.name_).at(Widget::ContactList).at(2));
    auto messagableList = cb_alex_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(alex_.name_).at(Widget::MessagableList).at(2));

    const auto& widget = alex_.api_->UI().ContactList(alex_.nym_id_);
    const auto id = widget.AddContact(chris_.name_, "", chris_.payment_code_);

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList, 10));
    EXPECT_TRUE(test_future(messagableList, 10));
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_AddContact, both)
{
    ASSERT_FALSE(alex_.nym_id_->empty());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    ASSERT_FALSE(alex_.payment_code_.empty());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

    auto contactList = cb_bob_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(bob_.name_).at(Widget::ContactList).at(1));
    auto messagableList = cb_bob_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(bob_.name_).at(Widget::MessagableList).at(1));

    const auto& widget = bob_.api_->UI().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        alex_.name_, alex_.nym_id_->str(), alex_.payment_code_);

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList));
    EXPECT_TRUE(test_future(messagableList, 10));
}

TEST_F(Test_AddContact, backwards)
{
    ASSERT_FALSE(chris_.nym_id_->empty());
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    ASSERT_FALSE(chris_.payment_code_.empty());
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

    auto contactList = cb_bob_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(bob_.name_).at(Widget::ContactList).at(2));
    auto messagableList = cb_bob_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(bob_.name_).at(Widget::MessagableList).at(2));

    const auto& widget = bob_.api_->UI().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        chris_.name_, chris_.payment_code_, chris_.nym_id_->str());

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList));
    EXPECT_TRUE(test_future(messagableList, 10));
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_AddContact, paymentcode_as_nymid)
{
    ASSERT_FALSE(alex_.payment_code_.empty());

    auto contactList = cb_chris_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(chris_.name_).at(Widget::ContactList).at(1));
    auto messagableList = cb_chris_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(chris_.name_).at(Widget::MessagableList).at(1));

    const auto& widget = chris_.api_->UI().ContactList(chris_.nym_id_);
    const auto id = widget.AddContact(alex_.name_, alex_.payment_code_, "");

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList));
    EXPECT_TRUE(test_future(messagableList, 10));
}
#endif  // OT_CRYPTO_SUPPORTED_SOURCE_BIP47

TEST_F(Test_AddContact, nymid_as_paymentcode)
{
    ASSERT_FALSE(bob_.nym_id_->empty());

    auto contactList = cb_chris_.SetCallback(
        Widget::ContactList,
        1,
        state_.at(chris_.name_).at(Widget::ContactList).at(2));
    auto messagableList = cb_chris_.SetCallback(
        Widget::MessagableList,
        1,
        state_.at(chris_.name_).at(Widget::MessagableList).at(2));

    const auto& widget = chris_.api_->UI().ContactList(chris_.nym_id_);
    const auto id = widget.AddContact(bob_.name_, "", bob_.nym_id_->str());

    EXPECT_FALSE(id.empty());
    EXPECT_TRUE(test_future(contactList));
    EXPECT_TRUE(test_future(messagableList, 10));
}

TEST_F(Test_AddContact, shutdown)
{
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();
}
}  // namespace
