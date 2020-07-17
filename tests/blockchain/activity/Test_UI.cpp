// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "Helpers.hpp"
#include "UIHelpers.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"

using Subchain = ot::api::client::blockchain::Subchain;

const auto time_1_ = ot::Clock::from_time_t(1592661306);
const auto time_2_ = ot::Clock::from_time_t(1592663862);
const auto time_3_ = ot::Clock::from_time_t(1592664462);
const auto time_4_ = ot::Clock::from_time_t(1592675062);
std::string txid_1_{};
std::string txid_2_{};
std::string txid_3_{};
std::string txid_4_{};
ot::Bip32Index first_index_{};
ot::Bip32Index second_index_{};
ot::Bip32Index third_index_{};
ot::Bip32Index fourth_index_{};
ot::Bip32Index fifth_index_{};
ot::Bip32Index sixth_index_{};
Counter account_list_{};
Counter account_activity_{};
Counter account_summary_{};
Counter activity_summary_{};
Counter activity_thread_1_{};
Counter activity_thread_2_{};
Counter activity_thread_3_{};

TEST_F(Test_BlockchainActivity, init)
{
    EXPECT_FALSE(nym_1_id().empty());
    EXPECT_FALSE(account_1_id().empty());
    EXPECT_FALSE(contact_1_id().empty());
    EXPECT_FALSE(contact_2_id().empty());
    EXPECT_FALSE(contact_5_id().empty());
    EXPECT_FALSE(contact_6_id().empty());
    EXPECT_FALSE(contact_7_id().empty());

    {
        const auto contact = api_.Contacts().Contact(contact_1_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_1_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_1_id()), nym_1_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_2_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_2_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_2_id()), nym_2_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_5_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_5_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_5_id()), contact_5_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_6_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_6_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_6_id()), contact_6_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_7_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_7_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_7_id()), contact_7_name_);
    }
}

TEST_F(Test_BlockchainActivity, setup_blockchain_account)
{
    const auto& account =
        api_.Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto indexOne =
        account.UseNext(Subchain::External, reason_, contact_5_id());
    const auto indexTwo = account.UseNext(Subchain::External, reason_);
    const auto indexThree = account.UseNext(
        Subchain::External, reason_, contact_6_id(), "Free Ross");
    const auto indexFour = account.UseNext(Subchain::External, reason_);
    const auto indexFive = account.UseNext(Subchain::External, reason_);
    const auto indexSix = account.UseNext(Subchain::External, reason_);

    ASSERT_TRUE(indexOne.has_value());
    ASSERT_TRUE(indexTwo.has_value());
    ASSERT_TRUE(indexThree.has_value());
    ASSERT_TRUE(indexFour.has_value());
    ASSERT_TRUE(indexFive.has_value());
    ASSERT_TRUE(indexSix.has_value());

    first_index_ = indexOne.value();
    second_index_ = indexTwo.value();
    third_index_ = indexThree.value();
    fourth_index_ = indexFour.value();
    fifth_index_ = indexFour.value();
    sixth_index_ = indexFour.value();
}

TEST_F(Test_BlockchainActivity, setup_ui)
{
    account_list_.expected_ += 1;
    account_activity_.expected_ += 0;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 0;
    activity_thread_1_.expected_ += 2;
    activity_thread_2_.expected_ += 2;
    activity_thread_3_.expected_ += 2;
    api_.UI().AccountList(nym_1_id(), make_cb(account_list_, "account_list_"));
    api_.UI().AccountActivity(
        nym_1_id(),
        api_.Factory().Identifier(std::string{btc_account_id_}),
        make_cb(account_activity_, "account_activity_"));
    api_.UI().AccountSummary(
        nym_1_id(),
        ot::proto::CITEMTYPE_BTC,
        make_cb(account_summary_, "account_summary"));
    api_.UI().ActivitySummary(
        nym_1_id(), make_cb(activity_summary_, "activity_summary"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_5_id(),
        make_cb(activity_thread_1_, "activity_thread_1"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_6_id(),
        make_cb(activity_thread_2_, "activity_thread_2"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_7_id(),
        make_cb(activity_thread_3_, "activity_thread_3"));
}

TEST_F(Test_BlockchainActivity, initial_state_account_list)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto& widget = api_.UI().AccountList(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), btc_account_id_);
    EXPECT_EQ(row->Balance(), 0);
    EXPECT_EQ(row->ContractID(), btc_unit_id_);
    EXPECT_EQ(row->DisplayBalance(), "0.00000000 BTC");
    EXPECT_EQ(row->DisplayUnit(), "BTC");
    EXPECT_EQ(row->Name(), "This device");
    EXPECT_EQ(row->NotaryID(), btc_notary_id_);
    EXPECT_EQ(row->NotaryName(), "Bitcoin");
    EXPECT_EQ(row->Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(row->Unit(), ot::proto::CITEMTYPE_BTC);
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, initial_state_account_list_qt)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto* pWidget = api_.UI().AccountListQt(nym_1_id());

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 4);
    EXPECT_EQ(widget.rowCount(), 1);

    using Model = ot::ui::AccountListQt;

    {
        ASSERT_TRUE(widget.hasIndex(0, 0));
        ASSERT_TRUE(widget.hasIndex(0, 1));
        ASSERT_TRUE(widget.hasIndex(0, 2));
        ASSERT_TRUE(widget.hasIndex(0, 3));

        const auto notaryID =
            widget.data(widget.index(0, 0), Model::NotaryIDRole);
        const auto unit = widget.data(widget.index(0, 0), Model::UnitRole);
        const auto accountID =
            widget.data(widget.index(0, 0), Model::AccountIDRole);
        const auto balance =
            widget.data(widget.index(0, 0), Model::BalanceRole);
        const auto polarity =
            widget.data(widget.index(0, 0), Model::PolarityRole);
        const auto type =
            widget.data(widget.index(0, 0), Model::AccountTypeRole);
        const auto contractID =
            widget.data(widget.index(0, 0), Model::ContractIdRole);
        const auto notaryName = widget.data(widget.index(0, 0));
        const auto unitName = widget.data(widget.index(0, 1));
        const auto accountName = widget.data(widget.index(0, 2));
        const auto displayBalance = widget.data(widget.index(0, 3));

        EXPECT_EQ(notaryID.toString(), btc_notary_id_);
        EXPECT_EQ(unit.toInt(), static_cast<int>(ot::proto::CITEMTYPE_BTC));
        EXPECT_EQ(accountID.toString(), btc_account_id_);
        EXPECT_EQ(balance.toInt(), 0);
        EXPECT_EQ(polarity.toInt(), 0);
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::AccountType::Blockchain));
        EXPECT_EQ(contractID.toString(), btc_unit_id_);
        EXPECT_EQ(notaryName.toString(), "Bitcoin");
        EXPECT_EQ(unitName.toString(), "BTC");
        EXPECT_EQ(accountName.toString(), "This device");
        EXPECT_EQ(displayBalance.toString(), "0.00000000 BTC");
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, initial_state_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 0);
    EXPECT_EQ(widget.BalancePolarity(), 0);
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), "0.00000000 BTC");
    EXPECT_EQ(widget.DisplayUnit(), "BTC");
    EXPECT_EQ(widget.Name(), "This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), "Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::proto::CITEMTYPE_BTC);

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, initial_state_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 0);
    EXPECT_EQ(widget.accountID(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 0);
    EXPECT_EQ(widget.displayBalance(), "0.00000000 BTC");

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, initial_state_account_summary)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));
    // FIXME
}

TEST_F(Test_BlockchainActivity, initial_state_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_1)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_1_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_5_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_2)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_2_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_6_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_3)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_3_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_7_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, receive_assigned)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 5;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 4;
    activity_thread_1_.expected_ += 2;
    activity_thread_2_.expected_ += 2;
    activity_thread_3_.expected_ += 0;
    const auto& account =
        api_.Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto& keyOne =
        account.BalanceElement(Subchain::External, first_index_);
    const auto& keyTwo =
        account.BalanceElement(Subchain::External, second_index_);
    const auto& keyThree =
        account.BalanceElement(Subchain::External, third_index_);
    const auto& keyFour =
        account.BalanceElement(Subchain::External, fourth_index_);

    const auto tx1 = get_test_transaction(keyOne, keyTwo, time_1_);
    const auto tx2 = get_test_transaction(keyThree, keyFour, time_2_);

    ASSERT_TRUE(tx1);
    ASSERT_TRUE(tx2);

    txid_1_ = tx1->ID().asHex();
    txid_2_ = tx2->ID().asHex();

    ASSERT_TRUE(api_.Blockchain().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx1, reason_));
    ASSERT_TRUE(api_.Blockchain().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx2, reason_));
}

TEST_F(Test_BlockchainActivity, receive_assigned_account_list)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto& widget = api_.UI().AccountList(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), btc_account_id_);
    EXPECT_EQ(row->Balance(), 0);  // FIXME
    EXPECT_EQ(row->ContractID(), btc_unit_id_);
    EXPECT_EQ(row->DisplayBalance(), "0.00000000 BTC");  // FIXME
    EXPECT_EQ(row->DisplayUnit(), "BTC");
    EXPECT_EQ(row->Name(), "This device");
    EXPECT_EQ(row->NotaryID(), btc_notary_id_);
    EXPECT_EQ(row->NotaryName(), "Bitcoin");
    EXPECT_EQ(row->Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(row->Unit(), ot::proto::CITEMTYPE_BTC);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 2761918);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), "0.02761918 BTC");
    EXPECT_EQ(widget.DisplayUnit(), "BTC");
    EXPECT_EQ(widget.Name(), "This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), "Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::proto::CITEMTYPE_BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);

    auto contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "Free Ross");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, receive_assigned_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 2);
    EXPECT_EQ(widget.accountID(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 1);
    EXPECT_EQ(widget.displayBalance(), "0.02761918 BTC");

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        ASSERT_TRUE(widget.hasIndex(0, 0));
        ASSERT_TRUE(widget.hasIndex(0, 1));
        ASSERT_TRUE(widget.hasIndex(0, 2));
        ASSERT_TRUE(widget.hasIndex(0, 3));
        ASSERT_TRUE(widget.hasIndex(0, 4));

        const auto contacts =
            widget.data(widget.index(0, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(0, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(0, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(0, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(0, 0));
        const auto text = widget.data(widget.index(0, 1));
        const auto memo = widget.data(widget.index(0, 2));
        const auto time = widget.data(widget.index(0, 3));
        const auto uuid = widget.data(widget.index(0, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(1, 0));
        ASSERT_TRUE(widget.hasIndex(1, 1));
        ASSERT_TRUE(widget.hasIndex(1, 2));
        ASSERT_TRUE(widget.hasIndex(1, 3));
        ASSERT_TRUE(widget.hasIndex(1, 4));

        const auto contacts =
            widget.data(widget.index(1, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(1, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(1, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(1, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(1, 0));
        const auto text = widget.data(widget.index(1, 1));
        const auto memo = widget.data(widget.index(1, 2));
        const auto time = widget.data(widget.index(1, 3));
        const auto uuid = widget.data(widget.index(1, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), "Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, receive_assigned_account_summary)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));
    // FIXME
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_thread_1)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_1_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_5_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_thread_2)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_2_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_6_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), "Free Ross");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, send)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 2;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 2;
    activity_thread_1_.expected_ += 0;
    activity_thread_2_.expected_ += 0;
    activity_thread_3_.expected_ += 2;

    const auto& account =
        api_.Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto& keyOne = account.BalanceElement(
        Subchain::External, first_index_);  // 0.0033045 BTC
    const auto& keyTwo = account.BalanceElement(
        Subchain::External, second_index_);  // 0.01050509 BTC
    const auto& keyThree = account.BalanceElement(
        Subchain::External, third_index_);  // 0.0033045 BTC
    const auto& keyFour =
        account.BalanceElement(Subchain::External, fourth_index_);
    const auto tx1 = api_.Blockchain().LoadTransactionBitcoin(txid_1_);
    const auto tx2 = api_.Blockchain().LoadTransactionBitcoin(txid_2_);

    ASSERT_TRUE(tx1);
    ASSERT_TRUE(tx2);

    auto input1 = ot::proto::BlockchainTransactionOutput{};
    auto input2 = ot::proto::BlockchainTransactionOutput{};
    auto input3 = ot::proto::BlockchainTransactionOutput{};

    ASSERT_TRUE(tx1->Outputs().at(1).Serialize(api_.Blockchain(), input1));
    ASSERT_TRUE(tx1->Outputs().at(0).Serialize(api_.Blockchain(), input2));
    ASSERT_TRUE(tx2->Outputs().at(0).Serialize(api_.Blockchain(), input3));

    input2.set_index(18);
    input3.set_index(27);

    const auto address =
        api_.Blockchain().DecodeAddress("17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem");
    const auto tx3 = get_test_transaction(
        keyTwo,
        keyOne,
        keyThree,
        keyFour,
        input1,
        input2,
        input3,
        std::get<0>(address)->asHex(),
        time_3_);

    ASSERT_TRUE(tx3);

    txid_3_ = tx3->ID().asHex();

    ASSERT_TRUE(api_.Blockchain().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx3, reason_));
}

TEST_F(Test_BlockchainActivity, send_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 1380959);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), "0.01380959 BTC");
    EXPECT_EQ(widget.DisplayUnit(), "BTC");
    EXPECT_EQ(widget.Name(), "This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), "Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::proto::CITEMTYPE_BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1380959);

    auto contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_7_id().str());
    EXPECT_EQ(row->DisplayAmount(), "-0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "Free Ross");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, send_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 3);
    EXPECT_EQ(widget.accountID(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 1);
    EXPECT_EQ(widget.displayBalance(), "0.01380959 BTC");

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        ASSERT_TRUE(widget.hasIndex(0, 0));
        ASSERT_TRUE(widget.hasIndex(0, 1));
        ASSERT_TRUE(widget.hasIndex(0, 2));
        ASSERT_TRUE(widget.hasIndex(0, 3));
        ASSERT_TRUE(widget.hasIndex(0, 4));

        const auto contacts =
            widget.data(widget.index(0, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(0, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(0, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(0, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(0, 0));
        const auto text = widget.data(widget.index(0, 1));
        const auto memo = widget.data(widget.index(0, 2));
        const auto time = widget.data(widget.index(0, 3));
        const auto uuid = widget.data(widget.index(0, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(1, 0));
        ASSERT_TRUE(widget.hasIndex(1, 1));
        ASSERT_TRUE(widget.hasIndex(1, 2));
        ASSERT_TRUE(widget.hasIndex(1, 3));
        ASSERT_TRUE(widget.hasIndex(1, 4));

        const auto contacts =
            widget.data(widget.index(1, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(1, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(1, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(1, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(1, 0));
        const auto text = widget.data(widget.index(1, 1));
        const auto memo = widget.data(widget.index(1, 2));
        const auto time = widget.data(widget.index(1, 3));
        const auto uuid = widget.data(widget.index(1, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), "Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(2, 0));
        ASSERT_TRUE(widget.hasIndex(2, 1));
        ASSERT_TRUE(widget.hasIndex(2, 2));
        ASSERT_TRUE(widget.hasIndex(2, 3));
        ASSERT_TRUE(widget.hasIndex(2, 4));

        const auto contacts =
            widget.data(widget.index(2, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(2, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(2, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(2, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(2, 0));
        const auto text = widget.data(widget.index(2, 1));
        const auto memo = widget.data(widget.index(2, 2));
        const auto time = widget.data(widget.index(2, 3));
        const auto uuid = widget.data(widget.index(2, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_7_id().str());
        }

        EXPECT_EQ(polarity.toInt(), -1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "-0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Outgoing Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_3_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, send_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    EXPECT_EQ(row->DisplayName(), contact_7_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Outgoing Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_7_id().str());
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, send_activity_thread_3)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_3_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_7_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), "-0.01380959 BTC");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), "Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_unassigned)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 2;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 0;
    activity_thread_1_.expected_ += 0;
    activity_thread_2_.expected_ += 0;
    activity_thread_3_.expected_ += 0;
    const auto& account =
        api_.Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto& keyFive =
        account.BalanceElement(Subchain::External, fifth_index_);
    const auto& keySix =
        account.BalanceElement(Subchain::External, sixth_index_);

    const auto tx = get_test_transaction(keyFive, keySix, time_4_);

    ASSERT_TRUE(tx);

    txid_4_ = tx->ID().asHex();

    ASSERT_TRUE(api_.Blockchain().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx, reason_));
}

TEST_F(Test_BlockchainActivity, receive_unassigned_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 2761918);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), "0.02761918 BTC");
    EXPECT_EQ(widget.DisplayUnit(), "BTC");
    EXPECT_EQ(widget.Name(), "This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), "Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::proto::CITEMTYPE_BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);

    auto contacts = row->Contacts();

    EXPECT_EQ(contacts.size(), 0);
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_4_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), -1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_7_id().str());
    EXPECT_EQ(row->DisplayAmount(), "-0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "Free Ross");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), "0.01380959 BTC");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, receive_unassigned_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 4);
    EXPECT_EQ(widget.accountID(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 1);
    EXPECT_EQ(widget.displayBalance(), "0.02761918 BTC");

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        ASSERT_TRUE(widget.hasIndex(0, 0));
        ASSERT_TRUE(widget.hasIndex(0, 1));
        ASSERT_TRUE(widget.hasIndex(0, 2));
        ASSERT_TRUE(widget.hasIndex(0, 3));
        ASSERT_TRUE(widget.hasIndex(0, 4));

        const auto contacts =
            widget.data(widget.index(0, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(0, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(0, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(0, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(0, 0));
        const auto text = widget.data(widget.index(0, 1));
        const auto memo = widget.data(widget.index(0, 2));
        const auto time = widget.data(widget.index(0, 3));
        const auto uuid = widget.data(widget.index(0, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(1, 0));
        ASSERT_TRUE(widget.hasIndex(1, 1));
        ASSERT_TRUE(widget.hasIndex(1, 2));
        ASSERT_TRUE(widget.hasIndex(1, 3));
        ASSERT_TRUE(widget.hasIndex(1, 4));

        const auto contacts =
            widget.data(widget.index(1, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(1, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(1, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(1, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(1, 0));
        const auto text = widget.data(widget.index(1, 1));
        const auto memo = widget.data(widget.index(1, 2));
        const auto time = widget.data(widget.index(1, 3));
        const auto uuid = widget.data(widget.index(1, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), "Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(2, 0));
        ASSERT_TRUE(widget.hasIndex(2, 1));
        ASSERT_TRUE(widget.hasIndex(2, 2));
        ASSERT_TRUE(widget.hasIndex(2, 3));
        ASSERT_TRUE(widget.hasIndex(2, 4));

        const auto contacts =
            widget.data(widget.index(2, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(2, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(2, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(2, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(2, 0));
        const auto text = widget.data(widget.index(2, 1));
        const auto memo = widget.data(widget.index(2, 2));
        const auto time = widget.data(widget.index(2, 3));
        const auto uuid = widget.data(widget.index(2, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_7_id().str());
        }

        EXPECT_EQ(polarity.toInt(), -1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "-0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Outgoing Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_3_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }

    {
        ASSERT_TRUE(widget.hasIndex(3, 0));
        ASSERT_TRUE(widget.hasIndex(3, 1));
        ASSERT_TRUE(widget.hasIndex(3, 2));
        ASSERT_TRUE(widget.hasIndex(3, 3));
        ASSERT_TRUE(widget.hasIndex(3, 4));

        const auto contacts =
            widget.data(widget.index(3, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(3, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(3, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(3, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(3, 0));
        const auto text = widget.data(widget.index(3, 1));
        const auto memo = widget.data(widget.index(3, 2));
        const auto time = widget.data(widget.index(3, 3));
        const auto uuid = widget.data(widget.index(3, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 0);
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), "");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), "0.01380959 BTC");
        EXPECT_EQ(text.toString(), "Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), "");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_4_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}

TEST_F(Test_BlockchainActivity, receive_unassigned_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    EXPECT_EQ(row->DisplayName(), contact_7_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Outgoing Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_7_id().str());
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}
#endif  // OT_QT

// TEST_F(Test_BlockchainActivity, shutdown)
// {
//     std::cout << "Waiting for extra events.\n";
//     ot::Sleep(std::chrono::second(30));
// }
