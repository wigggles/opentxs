// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#include <mutex>

using namespace opentxs;

#define ALICE "Alice"
#define BOB "Bob"
#define ACCOUNT_SUMMARY_BTC "ACCOUNT_SUMMARY_BTC"
#define ACCOUNT_SUMMARY_BCH "ACCOUNT_SUMMARY_BCH"
#define ACTIVITY_SUMMARY "ACTIVITY_SUMMARY"
#define CONTACTS_LIST "CONTACTS_LIST"
#define MESSAGABLE_LIST "MESSAGAGABLE_LIST"
#define PAYABLE_LIST_BTC "PAYABLE_LIST_BTC"
#define PAYABLE_LIST_BCH "PAYABLE_LIST_BCH"
#define PROFILE "PROFILE"
#define ACTIVITY_THREAD_ALICE_BOB "ACTIVITY_THREAD_ALICE_BOB"
#define ACTIVITY_THREAD_BOB_ALICE "ACTIVITY_THREAD_BOB_ALICE"
#define OT_METHOD "::Test_Basic::"

namespace
{
bool init_{false};

class Test_Basic : public ::testing::Test
{
public:
    using WidgetCallback = std::function<bool()>;
    // target counter value, callback
    using WidgetCallbackData =
        std::tuple<int, WidgetCallback, std::promise<bool>>;
    // name, counter
    using WidgetData = std::tuple<std::string, int, WidgetCallbackData>;
    using WidgetMap = std::map<OTIdentifier, WidgetData>;
    using WidgetNameMap = std::map<std::string, OTIdentifier>;
    // message type, counter
    using ServerReplyMap = std::map<std::string, int>;
    using UIChecker = std::function<bool()>;

    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string SeedB_;
    static const std::string Alice_;
    static const std::string Bob_;
    static const OTNymID alice_nym_id_;
    static const OTNymID bob_nym_id_;
    static OTIdentifier contact_id_alice_bob_;
    static OTIdentifier contact_id_bob_alice_;
    static const std::shared_ptr<const ServerContract> server_contract_;

    static WidgetMap alice_widget_map_;
    static WidgetNameMap alice_ui_names_;
    static WidgetMap bob_widget_map_;
    static WidgetNameMap bob_ui_names_;

    static OTZMQListenCallback alice_ui_update_callback_;
    static OTZMQListenCallback bob_ui_update_callback_;

    static const opentxs::api::client::Manager* alice_;
    static const opentxs::api::client::Manager* bob_;
    static std::mutex callback_lock_;

    const opentxs::api::client::Manager& alice_client_;
    const opentxs::api::client::Manager& bob_client_;
    const opentxs::api::server::Manager& server_1_;
    const OTServerID server_1_id_;
    OTZMQSubscribeSocket alice_ui_update_listener_;
    OTZMQSubscribeSocket bob_ui_update_listener_;

    static std::string alice_payment_code_;
    static std::string bob_payment_code_;

    static int msg_count_;
    static std::map<int, std::string> message_;

    Test_Basic()
        : alice_client_(OT::App().StartClient(args_, 0))
        , bob_client_(OT::App().StartClient(args_, 1))
        , server_1_(OT::App().StartServer(args_, 0, true))
        , server_1_id_(identifier::Server::Factory(server_1_.ID().str()))
        , alice_ui_update_listener_(
              alice_client_.ZeroMQ().SubscribeSocket(alice_ui_update_callback_))
        , bob_ui_update_listener_(
              bob_client_.ZeroMQ().SubscribeSocket(bob_ui_update_callback_))
    {
#if OT_CASH
        server_1_.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
#endif
        subscribe_sockets();

        if (false == init_) { init(); }
    }

    std::future<bool> add_ui_widget(
        const std::string& name,
        const Identifier& id,
        WidgetMap& map,
        WidgetNameMap& nameMap,
        int counter = 0,
        WidgetCallback callback = {})
    {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Name: ")(name)(" ID: ")(id)
            .Flush();
        WidgetData data{};
        std::get<0>(data) = name;
        auto& [limit, cb, promise] = std::get<2>(data);
        limit = counter, cb = callback, promise = {};
        auto output = promise.get_future();
        map.emplace(id, std::move(data));
        nameMap.emplace(name, id);

        return output;
    }

    void import_server_contract(
        const ServerContract& contract,
        const opentxs::api::client::Manager& client)
    {
        auto clientVersion =
            client.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client.OTX().SetIntroductionServer(*clientVersion);
    }

    void init()
    {
        const_cast<std::string&>(SeedA_) =
            alice_client_.Exec().Wallet_ImportSeed(
                "spike nominee miss inquiry fee nothing belt list other "
                "daughter leave valley twelve gossip paper",
                "");
        const_cast<std::string&>(SeedB_) = bob_client_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        const_cast<std::string&>(Alice_) = alice_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, ALICE, SeedA_, 0);
        const_cast<std::string&>(Bob_) = bob_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, BOB, SeedB_, 0);
        const_cast<OTNymID&>(alice_nym_id_) = identifier::Nym::Factory(Alice_);
        const_cast<OTNymID&>(bob_nym_id_) = identifier::Nym::Factory(Bob_);
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_1_.Wallet().Server(server_1_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_1_id_->empty());

        import_server_contract(*server_contract_, alice_client_);
        import_server_contract(*server_contract_, bob_client_);

        alice_ = &alice_client_;
        bob_ = &bob_client_;

        init_ = true;
    }

    std::future<bool> set_callback(
        const std::string& name,
        const WidgetNameMap& nameMap,
        int limit,
        WidgetCallback callback,
        WidgetMap& map)
    {
        Lock lock(callback_lock_);
        auto& [counter, cb, promise] = std::get<2>(map.at(nameMap.at(name)));
        counter += limit;
        cb = callback;
        promise = {};

        return promise.get_future();
    }

    std::future<bool> set_callback_alice(
        const std::string& name,
        int limit,
        WidgetCallback callback)
    {
        return set_callback(
            name, alice_ui_names_, limit, callback, alice_widget_map_);
    }

    std::future<bool> set_callback_bob(
        const std::string& name,
        int limit,
        WidgetCallback callback)
    {
        return set_callback(
            name, bob_ui_names_, limit, callback, bob_widget_map_);
    }

    void subscribe_sockets()
    {
        ASSERT_TRUE(alice_ui_update_listener_->Start(
            alice_client_.Endpoints().WidgetUpdate()));
        ASSERT_TRUE(bob_ui_update_listener_->Start(
            bob_client_.Endpoints().WidgetUpdate()));
    }

    static void widget_updated(
        const Lock& lock,
        const std::string nym,
        WidgetData& data)
    {
        auto& [name, counter, callbackData] = data;
        auto& [limit, callback, future] = callbackData;
        ++counter;

        if (counter >= limit) {
            if (callback) {
                future.set_value(callback());
                callback = {};
                future = {};
                limit = 0;
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(nym)(
                    " missing callback for ")(name)
                    .Flush();
            }
        } else {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Skipping update ")(counter)(
                " to ")(name)
                .Flush();
        }
    }

    static void widget_updated_alice(
        const opentxs::network::zeromq::Message& incoming)
    {
        Lock lock(callback_lock_);
        const auto widgetID = Identifier::Factory(incoming.Body().at(0));

        ASSERT_NE("", widgetID->str().c_str());

        widget_updated(lock, "alice", alice_widget_map_.at(widgetID));
    }

    static void widget_updated_bob(
        const opentxs::network::zeromq::Message& incoming)
    {
        Lock lock(callback_lock_);
        const auto widgetID = Identifier::Factory(incoming.Body().at(0));

        ASSERT_NE("", widgetID->str().c_str());

        widget_updated(lock, "bob", bob_widget_map_.at(widgetID));
    }
};

const opentxs::ArgList Test_Basic::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_Basic::SeedA_{""};
const std::string Test_Basic::SeedB_{""};
const std::string Test_Basic::Alice_{""};
const std::string Test_Basic::Bob_{""};
const OTNymID Test_Basic::alice_nym_id_{identifier::Nym::Factory()};
const OTNymID Test_Basic::bob_nym_id_{identifier::Nym::Factory()};
OTIdentifier Test_Basic::contact_id_alice_bob_{Identifier::Factory()};
OTIdentifier Test_Basic::contact_id_bob_alice_{Identifier::Factory()};
const std::shared_ptr<const ServerContract> Test_Basic::server_contract_{
    nullptr};
Test_Basic::WidgetMap Test_Basic::alice_widget_map_{};
Test_Basic::WidgetNameMap Test_Basic::alice_ui_names_{};
Test_Basic::WidgetMap Test_Basic::bob_widget_map_{};
Test_Basic::WidgetNameMap Test_Basic::bob_ui_names_{};
OTZMQListenCallback Test_Basic::alice_ui_update_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            widget_updated_alice(incoming);
        })};
OTZMQListenCallback Test_Basic::bob_ui_update_callback_{
    opentxs::network::zeromq::ListenCallback::Factory(
        [](const opentxs::network::zeromq::Message& incoming) -> void {
            widget_updated_bob(incoming);
        })};
const opentxs::api::client::Manager* Test_Basic::alice_{nullptr};
const opentxs::api::client::Manager* Test_Basic::bob_{nullptr};
std::mutex Test_Basic::callback_lock_{};
std::string Test_Basic::alice_payment_code_;
std::string Test_Basic::bob_payment_code_;
int Test_Basic::msg_count_ = 0;
std::map<int, std::string> Test_Basic::message_{};

static int view_contact_list(
    const opentxs::api::client::Manager& client,
    const OTNymID& nym_id)
{
    auto& contacts = client.UI().ContactList(nym_id);

    int count = 1;
    auto entry = contacts.First();
    while (entry->Valid()) {
        count++;
        if (entry->Last() == false)
            entry = contacts.Next();
        else
            break;
    }
    return --count;
}

TEST_F(Test_Basic, instantiate_ui_objects)
{
    auto cb1 = [&]() -> bool {
        const auto& profile = alice_->UI().Profile(alice_nym_id_);
        auto paymentCode = alice_->Factory().PaymentCode(SeedA_, 0, 1);

        EXPECT_EQ(paymentCode->asBase58(), profile.PaymentCode());
        EXPECT_STREQ(ALICE, profile.DisplayName().c_str());

        auto row = profile.First();

        EXPECT_FALSE(row->Valid());

        return true;
    };
    auto cb2 = [&]() -> bool {
        const auto& profile = bob_->UI().Profile(bob_nym_id_);
        auto paymentCode = bob_->Factory().PaymentCode(SeedB_, 0, 1);

        EXPECT_EQ(paymentCode->asBase58(), profile.PaymentCode());
        EXPECT_STREQ(BOB, profile.DisplayName().c_str());

        auto row = profile.First();

        EXPECT_FALSE(row->Valid());

        return true;
    };
    auto cb3 = [&]() -> bool {
        const auto& contacts = alice_->UI().ContactList(alice_nym_id_);
        const auto row = contacts.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(
            row->DisplayName() == ALICE || row->DisplayName() == "Owner");
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("ME", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto cb4 = [&]() -> bool {
        const auto& contacts = bob_->UI().ContactList(bob_nym_id_);
        const auto row = contacts.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(row->DisplayName() == BOB || row->DisplayName() == "Owner");
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("ME", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto cb5 = [&]() -> bool {
        const auto& list =
            alice_->UI().PayableList(alice_nym_id_, proto::CITEMTYPE_BTC);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(ALICE, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto cb6 = [&]() -> bool {
        const auto& list =
            bob_->UI().PayableList(bob_nym_id_, proto::CITEMTYPE_BTC);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(BOB, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };

    Lock lock(callback_lock_);
    auto future1 = add_ui_widget(
        PROFILE,
        alice_client_.UI().Profile(alice_nym_id_).WidgetID(),
        alice_widget_map_,
        alice_ui_names_,
        2,
        cb1);
    auto future2 = add_ui_widget(
        PROFILE,
        bob_client_.UI().Profile(bob_nym_id_).WidgetID(),
        bob_widget_map_,
        bob_ui_names_,
        2,
        cb2);
    add_ui_widget(
        ACTIVITY_SUMMARY,
        alice_client_.UI().ActivitySummary(alice_nym_id_).WidgetID(),
        alice_widget_map_,
        alice_ui_names_);
    add_ui_widget(
        ACTIVITY_SUMMARY,
        bob_client_.UI().ActivitySummary(bob_nym_id_).WidgetID(),
        bob_widget_map_,
        bob_ui_names_);
    auto future3 = add_ui_widget(
        CONTACTS_LIST,
        alice_client_.UI().ContactList(alice_nym_id_).WidgetID(),
        alice_widget_map_,
        alice_ui_names_,
        1,
        cb3);
    auto future4 = add_ui_widget(
        CONTACTS_LIST,
        bob_client_.UI().ContactList(bob_nym_id_).WidgetID(),
        bob_widget_map_,
        bob_ui_names_,
        1,
        cb4);
    add_ui_widget(
        PAYABLE_LIST_BCH,
        alice_client_.UI()
            .PayableList(alice_nym_id_, proto::CITEMTYPE_BCH)
            .WidgetID(),
        alice_widget_map_,
        alice_ui_names_);
    add_ui_widget(
        PAYABLE_LIST_BCH,
        bob_client_.UI()
            .PayableList(bob_nym_id_, proto::CITEMTYPE_BCH)
            .WidgetID(),
        bob_widget_map_,
        bob_ui_names_);
    auto future5 = add_ui_widget(
        PAYABLE_LIST_BTC,
        alice_client_.UI()
            .PayableList(alice_nym_id_, proto::CITEMTYPE_BTC)
            .WidgetID(),
        alice_widget_map_,
        alice_ui_names_,
        1,
        cb5);
    auto future6 = add_ui_widget(
        PAYABLE_LIST_BTC,
        bob_client_.UI()
            .PayableList(bob_nym_id_, proto::CITEMTYPE_BTC)
            .WidgetID(),
        bob_widget_map_,
        bob_ui_names_,
        1,
        cb6);
    add_ui_widget(
        ACCOUNT_SUMMARY_BTC,
        alice_client_.UI()
            .AccountSummary(alice_nym_id_, proto::CITEMTYPE_BTC)
            .WidgetID(),
        alice_widget_map_,
        alice_ui_names_);
    add_ui_widget(
        ACCOUNT_SUMMARY_BTC,
        bob_client_.UI()
            .AccountSummary(bob_nym_id_, proto::CITEMTYPE_BTC)
            .WidgetID(),
        bob_widget_map_,
        bob_ui_names_);
    add_ui_widget(
        ACCOUNT_SUMMARY_BCH,
        alice_client_.UI()
            .AccountSummary(alice_nym_id_, proto::CITEMTYPE_BCH)
            .WidgetID(),
        alice_widget_map_,
        alice_ui_names_);
    add_ui_widget(
        ACCOUNT_SUMMARY_BCH,
        bob_client_.UI()
            .AccountSummary(bob_nym_id_, proto::CITEMTYPE_BCH)
            .WidgetID(),
        bob_widget_map_,
        bob_ui_names_);
    add_ui_widget(
        MESSAGABLE_LIST,
        alice_client_.UI().MessagableList(alice_nym_id_).WidgetID(),
        alice_widget_map_,
        alice_ui_names_);
    add_ui_widget(
        MESSAGABLE_LIST,
        bob_client_.UI().MessagableList(bob_nym_id_).WidgetID(),
        bob_widget_map_,
        bob_ui_names_);

    EXPECT_EQ(8, alice_widget_map_.size());
    EXPECT_EQ(8, alice_ui_names_.size());
    EXPECT_EQ(8, bob_widget_map_.size());
    EXPECT_EQ(8, bob_ui_names_.size());

    lock.unlock();

    EXPECT_TRUE(future1.get());
    EXPECT_TRUE(future2.get());
    EXPECT_TRUE(future3.get());
    EXPECT_TRUE(future4.get());
    EXPECT_TRUE(future5.get());
    EXPECT_TRUE(future6.get());
}

TEST_F(Test_Basic, initial_messagable_list_alice)
{
    const auto& list = alice_->UI().MessagableList(alice_nym_id_);
    auto row = list.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, initial_messagable_list_bob)
{
    const auto& list = bob_->UI().MessagableList(bob_nym_id_);
    auto row = list.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, initial_activity_summary_alice)
{
    const auto& summary = alice_->UI().ActivitySummary(alice_nym_id_);
    auto row = summary.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, initial_activity_summary_bob)
{
    const auto& summary = bob_->UI().ActivitySummary(bob_nym_id_);
    auto row = summary.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, initial_account_summary_alice)
{
    const auto& btc =
        alice_->UI().AccountSummary(alice_nym_id_, proto::CITEMTYPE_BTC);
    auto row = btc.First();

    EXPECT_FALSE(row->Valid());

    const auto& bch =
        alice_->UI().AccountSummary(alice_nym_id_, proto::CITEMTYPE_BCH);
    row = bch.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, initial_account_summary_bob)
{
    const auto& btc =
        bob_->UI().AccountSummary(bob_nym_id_, proto::CITEMTYPE_BTC);
    auto row = btc.First();

    EXPECT_FALSE(row->Valid());

    const auto& bch =
        bob_->UI().AccountSummary(bob_nym_id_, proto::CITEMTYPE_BCH);
    row = bch.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_Basic, payment_codes)
{
    auto alice = alice_client_.Wallet().mutable_Nym(alice_nym_id_);
    auto bob = bob_client_.Wallet().mutable_Nym(bob_nym_id_);

    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, alice.Type());
    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, bob.Type());

    auto aliceScopeSet =
        alice.SetScope(opentxs::proto::CITEMTYPE_INDIVIDUAL, ALICE, true);
    auto bobScopeSet = bob.SetScope(proto::CITEMTYPE_INDIVIDUAL, BOB, true);

    EXPECT_TRUE(aliceScopeSet);
    EXPECT_TRUE(bobScopeSet);

    alice_payment_code_ =
        alice_client_.Factory().PaymentCode(SeedA_, 0, 1)->asBase58();
    bob_payment_code_ =
        bob_client_.Factory().PaymentCode(SeedB_, 0, 1)->asBase58();

    EXPECT_FALSE(alice_payment_code_.empty());
    EXPECT_FALSE(bob_payment_code_.empty());

    alice.AddPaymentCode(
        alice_payment_code_, opentxs::proto::CITEMTYPE_BTC, true, true);
    bob.AddPaymentCode(
        bob_payment_code_, opentxs::proto::CITEMTYPE_BTC, true, true);
    alice.AddPaymentCode(
        alice_payment_code_, opentxs::proto::CITEMTYPE_BCH, true, true);
    bob.AddPaymentCode(
        bob_payment_code_, opentxs::proto::CITEMTYPE_BCH, true, true);

    EXPECT_FALSE(alice.PaymentCode(proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(bob.PaymentCode(proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(alice.PaymentCode(proto::CITEMTYPE_BCH).empty());
    EXPECT_FALSE(bob.PaymentCode(proto::CITEMTYPE_BCH).empty());

    alice.Release();
    bob.Release();
}

TEST_F(Test_Basic, introduction_server)
{
    alice_client_.OTX().StartIntroductionServer(alice_nym_id_);
    bob_client_.OTX().StartIntroductionServer(bob_nym_id_);
    auto task1 = alice_client_.OTX().RegisterNymPublic(
        alice_nym_id_, server_1_id_, true);
    auto task2 =
        bob_client_.OTX().RegisterNymPublic(bob_nym_id_, server_1_id_, true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, task1.second.get().first);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, task2.second.get().first);

    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_id_).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_id_).get();
}

TEST_F(Test_Basic, add_contact_preconditions)
{
    // Neither alice nor bob should know about each other yet
    auto alice = bob_client_.Wallet().Nym(alice_nym_id_);
    auto bob = alice_client_.Wallet().Nym(bob_nym_id_);

    EXPECT_FALSE(alice);
    EXPECT_FALSE(bob);
}

TEST_F(Test_Basic, add_contact_Bob_To_Alice)
{
    auto contactListCallback = [&]() -> bool {
        const auto& contacts = alice_->UI().ContactList(alice_nym_id_);
        auto row = contacts.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(row->DisplayName() == ALICE);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("ME", row->Section().c_str());
        EXPECT_FALSE(row->Last());

        if (row->Last()) { return false; }

        row = contacts.Next();

        EXPECT_TRUE(row->DisplayName() == BOB);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("B", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        // We need this later
        contact_id_alice_bob_ = Identifier::Factory(row->ContactID());

        EXPECT_FALSE(contact_id_alice_bob_->empty());

        return true;
    };
    auto payableListBTCCallback = [&]() -> bool {
        const auto& list =
            alice_->UI().PayableList(alice_nym_id_, proto::CITEMTYPE_BTC);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(ALICE, row->DisplayName().c_str());
        EXPECT_FALSE(row->Last());

        if (row->Last()) { return false; }

        row = list.Next();

        EXPECT_STREQ(BOB, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto payableListBCHCallback = [&]() -> bool {
        const auto& list =
            alice_->UI().PayableList(alice_nym_id_, proto::CITEMTYPE_BCH);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(BOB, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        // TODO why isn't Alice in this list?

        return true;
    };
    auto messagableListCallback = [&]() -> bool {
        const auto& list = alice_->UI().MessagableList(alice_nym_id_);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(row->DisplayName() == BOB);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("B", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto contactListDone =
        set_callback_alice(CONTACTS_LIST, 2, contactListCallback);
    auto payableBTCListDone =
        set_callback_alice(PAYABLE_LIST_BTC, 2, payableListBTCCallback);
    auto payableBCHListDone =
        set_callback_alice(PAYABLE_LIST_BCH, 1, payableListBCHCallback);
    auto messagableListDone =
        set_callback_alice(MESSAGABLE_LIST, 1, messagableListCallback);

    // Add the contact
    alice_->UI()
        .ContactList(alice_nym_id_)
        .AddContact(BOB, bob_payment_code_, bob_nym_id_->str());

    EXPECT_TRUE(contactListDone.get());
    EXPECT_TRUE(payableBTCListDone.get());
    EXPECT_TRUE(payableBCHListDone.get());
    EXPECT_TRUE(messagableListDone.get());
}

TEST_F(Test_Basic, activity_thread_alice_bob)
{
    auto callback = [&]() -> bool {
        const auto& thread =
            alice_->UI().ActivityThread(alice_nym_id_, contact_id_alice_bob_);
        auto row = thread.First();

        EXPECT_FALSE(row->Valid());

        return true;
    };

    Lock lock(callback_lock_);
    auto done = add_ui_widget(
        ACTIVITY_THREAD_ALICE_BOB,
        alice_client_.UI()
            .ActivityThread(alice_nym_id_, contact_id_alice_bob_)
            .WidgetID(),
        alice_widget_map_,
        alice_ui_names_,
        2,
        callback);

    EXPECT_EQ(9, alice_widget_map_.size());
    EXPECT_EQ(9, alice_ui_names_.size());

    lock.unlock();

    EXPECT_TRUE(done.get());
}

TEST_F(Test_Basic, send_message_from_Alice_to_Bob_1)
{
    const auto& from_client = alice_client_;
    const auto& from_nym_id_ = alice_nym_id_;
    const auto messageID = ++msg_count_;
    std::stringstream text{};
    text << ALICE << " messaged " << BOB << " with message #"
         << std::to_string(messageID);
    auto& firstMessage = message_[messageID];
    firstMessage = text.str();

    auto aliceActivitySummaryCallback = [&]() -> bool {
        const auto& summary = alice_->UI().ActivitySummary(alice_nym_id_);
        auto row = summary.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), BOB);
        EXPECT_EQ(row->ImageURI(), "");
        EXPECT_EQ(row->Text(), firstMessage);
        EXPECT_FALSE(row->ThreadID().empty());
        EXPECT_EQ(row->Type(), StorageBox::MAILOUTBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobActivitySummaryCallback = [&]() -> bool {
        const auto& summary = bob_->UI().ActivitySummary(bob_nym_id_);
        auto row = summary.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), ALICE);
        EXPECT_EQ(row->ImageURI(), "");
        EXPECT_EQ(row->Text(), firstMessage);
        EXPECT_FALSE(row->ThreadID().empty());
        EXPECT_EQ(row->Type(), StorageBox::MAILINBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto aliceActivityThreadCallback = [&]() -> bool {
        const auto& thread =
            alice_->UI().ActivityThread(alice_nym_id_, contact_id_alice_bob_);
        auto row = thread.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), firstMessage);
        EXPECT_EQ(row->Type(), StorageBox::MAILOUTBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobContactListCallback = [&]() -> bool {
        const auto& contacts = bob_->UI().ContactList(bob_nym_id_);
        auto row = contacts.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(row->DisplayName() == BOB);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("ME", row->Section().c_str());
        EXPECT_FALSE(row->Last());

        row = contacts.Next();

        EXPECT_TRUE(row->DisplayName() == ALICE);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("A", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        contact_id_bob_alice_ = Identifier::Factory(row->ContactID());

        EXPECT_FALSE(contact_id_bob_alice_->empty());

        return true;
    };
    auto bobMessagableListCallback = [&]() -> bool {
        const auto& list = bob_->UI().MessagableList(bob_nym_id_);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_TRUE(row->DisplayName() == ALICE);
        EXPECT_TRUE(row->Valid());
        EXPECT_STREQ("", row->ImageURI().c_str());
        EXPECT_STREQ("A", row->Section().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobPayableListBTCCallback = [&]() -> bool {
        const auto& list =
            bob_->UI().PayableList(bob_nym_id_, proto::CITEMTYPE_BTC);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(ALICE, row->DisplayName().c_str());
        EXPECT_FALSE(row->Last());

        if (row->Last()) { return false; }

        row = list.Next();

        EXPECT_STREQ(BOB, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobPayableListBCHCallback = [&]() -> bool {
        const auto& list =
            bob_->UI().PayableList(bob_nym_id_, proto::CITEMTYPE_BCH);
        auto row = list.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_STREQ(ALICE, row->DisplayName().c_str());
        EXPECT_TRUE(row->Last());

        // TODO why isn't Bob in this list?

        return true;
    };
    auto aliceActivitySummaryDone =
        set_callback_alice(ACTIVITY_SUMMARY, 2, aliceActivitySummaryCallback);
    auto bobActivitySummaryDone =
        set_callback_bob(ACTIVITY_SUMMARY, 2, bobActivitySummaryCallback);
    auto aliceActivityThreadDone = set_callback_alice(
        ACTIVITY_THREAD_ALICE_BOB, 7, aliceActivityThreadCallback);
    auto bobContactListDone =
        set_callback_bob(CONTACTS_LIST, 3, bobContactListCallback);
    auto bobMessagableListDone =
        set_callback_bob(MESSAGABLE_LIST, 2, bobMessagableListCallback);
    auto bobPayableListBTCDone =
        set_callback_bob(PAYABLE_LIST_BTC, 1, bobPayableListBTCCallback);
    auto bobPayableListBCHDone =
        set_callback_bob(PAYABLE_LIST_BCH, 1, bobPayableListBCHCallback);

    const auto& conversation =
        from_client.UI().ActivityThread(alice_nym_id_, contact_id_alice_bob_);
    conversation.SetDraft(firstMessage);

    EXPECT_EQ(conversation.GetDraft(), firstMessage);

    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
    EXPECT_TRUE(aliceActivitySummaryDone.get());
    EXPECT_TRUE(aliceActivityThreadDone.get());
    EXPECT_TRUE(bobContactListDone.get());
    EXPECT_TRUE(bobPayableListBTCDone.get());
    EXPECT_TRUE(bobPayableListBCHDone.get());
    EXPECT_TRUE(bobMessagableListDone.get());
    EXPECT_TRUE(bobActivitySummaryDone.get());
}

TEST_F(Test_Basic, activity_thread_bob_alice)
{
    auto callback = [&]() -> bool {
        const auto& thread =
            bob_->UI().ActivityThread(bob_nym_id_, contact_id_bob_alice_);
        auto row = thread.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), message_.at(msg_count_));
        EXPECT_EQ(row->Type(), StorageBox::MAILINBOX);

        return true;
    };

    Lock lock(callback_lock_);
    auto done = add_ui_widget(
        ACTIVITY_THREAD_BOB_ALICE,
        bob_client_.UI()
            .ActivityThread(bob_nym_id_, contact_id_bob_alice_)
            .WidgetID(),
        bob_widget_map_,
        bob_ui_names_,
        2,
        callback);

    EXPECT_EQ(9, bob_widget_map_.size());
    EXPECT_EQ(9, bob_ui_names_.size());

    lock.unlock();

    EXPECT_TRUE(done.get());
}

TEST_F(Test_Basic, send_message_from_Bob_to_Alice_2)
{
    const auto& firstMessage = message_.at(msg_count_);
    const auto& from_client = bob_client_;
    const auto& from_nym_id_ = bob_nym_id_;
    const auto messageID = ++msg_count_;
    std::stringstream text{};
    text << BOB << " messaged " << ALICE << " with message #"
         << std::to_string(messageID);
    auto& secondMessage = message_[messageID];
    secondMessage = text.str();

    auto aliceActivitySummaryCallback = [&]() -> bool {
        const auto& summary = alice_->UI().ActivitySummary(alice_nym_id_);
        auto row = summary.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), BOB);
        EXPECT_EQ(row->ImageURI(), "");
        EXPECT_EQ(row->Text(), secondMessage);
        EXPECT_FALSE(row->ThreadID().empty());
        EXPECT_EQ(row->Type(), StorageBox::MAILINBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobActivitySummaryCallback = [&]() -> bool {
        const auto& summary = bob_->UI().ActivitySummary(bob_nym_id_);
        auto row = summary.First();

        EXPECT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), ALICE);
        EXPECT_EQ(row->ImageURI(), "");
        EXPECT_EQ(row->Text(), secondMessage);
        EXPECT_FALSE(row->ThreadID().empty());
        EXPECT_EQ(row->Type(), StorageBox::MAILOUTBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto aliceActivityThreadCallback = [&]() -> bool {
        const auto& thread =
            alice_->UI().ActivityThread(alice_nym_id_, contact_id_alice_bob_);
        auto row = thread.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), firstMessage);
        EXPECT_EQ(row->Type(), StorageBox::MAILOUTBOX);
        EXPECT_FALSE(row->Last());

        if (row->Last()) { return false; }

        row = thread.Next();

        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), secondMessage);
        EXPECT_EQ(row->Type(), StorageBox::MAILINBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto bobActivityThreadCallback = [&]() -> bool {
        const auto& thread =
            bob_->UI().ActivityThread(bob_nym_id_, contact_id_bob_alice_);
        auto row = thread.First();

        EXPECT_TRUE(row->Valid());

        if (false == row->Valid()) { return false; }

        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), firstMessage);
        EXPECT_EQ(row->Type(), StorageBox::MAILINBOX);
        EXPECT_FALSE(row->Last());

        if (row->Last()) { return false; }

        row = thread.Next();

        EXPECT_EQ(row->Amount(), 0);
        EXPECT_EQ(row->DisplayAmount(), "");
        EXPECT_FALSE(row->Loading());
        EXPECT_EQ(row->Memo(), "");
        EXPECT_FALSE(row->Pending());
        EXPECT_EQ(row->Text(), secondMessage);
        EXPECT_EQ(row->Type(), StorageBox::MAILOUTBOX);
        EXPECT_TRUE(row->Last());

        return true;
    };
    auto aliceActivitySummaryDone =
        set_callback_alice(ACTIVITY_SUMMARY, 4, aliceActivitySummaryCallback);
    auto aliceActivityThreadDone = set_callback_alice(
        ACTIVITY_THREAD_ALICE_BOB, 10, aliceActivityThreadCallback);
    auto bobActivitySummaryDone =
        set_callback_bob(ACTIVITY_SUMMARY, 4, bobActivitySummaryCallback);
    auto bobActivityThreadDone = set_callback_bob(
        ACTIVITY_THREAD_BOB_ALICE, 7, bobActivityThreadCallback);

    const auto& conversation =
        from_client.UI().ActivityThread(bob_nym_id_, contact_id_bob_alice_);
    conversation.SetDraft(secondMessage);

    EXPECT_EQ(conversation.GetDraft(), secondMessage);

    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
    EXPECT_TRUE(bobActivitySummaryDone.get());
    EXPECT_TRUE(bobActivityThreadDone.get());
    EXPECT_TRUE(aliceActivitySummaryDone.get());
    EXPECT_TRUE(aliceActivityThreadDone.get());
}

TEST_F(Test_Basic, shutdown)
{
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_id_).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_id_).get();
}
}  // namespace
