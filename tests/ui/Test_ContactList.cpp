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

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#define ALICE_NYM_ID "ot2CyrTzwREHzboZ2RyCT8QsTj3Scaa55JRG"
#define ALICE_NYM_NAME "Alice"
#define DEFAULT_ME_NAME "Owner"
#define BOB_PAYMENT_CODE                                                       \
    "PM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxt" \
    "eQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97"
#define BOB_NYM_NAME "Bob"

using namespace opentxs;

template class opentxs::Pimpl<opentxs::PaymentCode>;

namespace
{
class Test_ContactList : public ::testing::Test
{
public:
    using WidgetUpdateCounter = std::map<std::string, int>;

    const std::string fingerprint_{OT::App().API().Exec().Wallet_ImportSeed(
        "response seminar brave tip suit recall often sound stick owner "
        "lottery motion",
        "")};
    const OTIdentifier nym_id_{
        Identifier::Factory(OT::App().API().Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL,
            ALICE_NYM_NAME,
            fingerprint_,
            0))};
    std::string contact_widget_id_{""};
    WidgetUpdateCounter counter_;
    std::mutex counter_lock_;
    OTZMQListenCallback callback_{network::zeromq::ListenCallback::Factory(
        [=](const network::zeromq::Message& message) -> void {
            ASSERT_EQ(1, message.size());
            IncrementCounter(message.at(0));
        })};
    OTZMQSubscribeSocket subscriber_{setup_listener(callback_)};
    const ui::ContactList& contact_list_{OT::App().UI().ContactList(nym_id_)};
    std::thread loop_{&Test_ContactList::loop, this};
    std::atomic<bool> shutdown_{false};
    const OTPaymentCode bob_payment_code_{
        PaymentCode::Factory(BOB_PAYMENT_CODE)};
    OTIdentifier bob_contact_id_{Identifier::Factory()};

    static OTZMQSubscribeSocket setup_listener(
        const network::zeromq::ListenCallback& callback)
    {
        auto output = OT::App().ZMQ().Context().SubscribeSocket(callback);
        output->Start(network::zeromq::Socket::WidgetUpdateEndpoint);

        return output;
    }

    void IncrementCounter(const std::string& widgetID)
    {
        Lock lock(counter_lock_);
        otErr << "Widget " << widgetID << " update counter set to "
              << ++counter_[widgetID] << std::endl;
    }

    int GetCounter(const std::string& widgetID)
    {
        Lock lock(counter_lock_);

        return counter_[widgetID];
    }

    void loop()
    {
        while (contact_widget_id_.empty()) {
            contact_widget_id_ = contact_list_.WidgetID()->str();
        }

        while (false == shutdown_.load()) {
            switch (GetCounter(contact_widget_id_)) {
                case 0:
                case 1: {
                    const auto first = contact_list_.First();

                    ASSERT_EQ(true, first.get().Valid());
                    EXPECT_EQ(true, first.get().Last());
                    EXPECT_EQ(
                        true,
                        (DEFAULT_ME_NAME == first.get().DisplayName()) ||
                            (ALICE_NYM_NAME == first.get().DisplayName()));

                    if (ALICE_NYM_NAME == first.get().DisplayName()) {
                        IncrementCounter(contact_widget_id_);
                    }
                } break;
                case 2: {
                    const auto first = contact_list_.First();

                    ASSERT_EQ(true, first.get().Valid());
                    EXPECT_EQ(first.get().DisplayName(), ALICE_NYM_NAME);
                } break;
                case 3: {
                    const auto first = contact_list_.First();

                    ASSERT_EQ(true, first.get().Valid());
                    ASSERT_EQ(false, first.get().Last());

                    const auto bob = contact_list_.Next();

                    ASSERT_EQ(true, bob.get().Valid());
                    ASSERT_EQ(true, bob.get().Last());
                    EXPECT_EQ(bob.get().DisplayName(), BOB_NYM_NAME);

                    otErr << "Test complete" << std::endl;
                    IncrementCounter(contact_widget_id_);
                } break;
                default: {
                    shutdown_.store(true);
                }
            }
        }
    }

    ~Test_ContactList() { loop_.join(); }
};

TEST_F(Test_ContactList, Contact_List)
{
    ASSERT_EQ(false, nym_id_->empty());
    ASSERT_EQ(nym_id_->str(), ALICE_NYM_ID);
    ASSERT_EQ(true, bob_payment_code_->VerifyInternally());

    while (GetCounter(contact_widget_id_) < 2) { ; }

    const auto bob = OT::App().Contact().NewContact(
        BOB_NYM_NAME, bob_payment_code_->ID(), bob_payment_code_);

    ASSERT_EQ(true, bool(bob));

    bob_contact_id_ = Identifier::Factory(bob->ID());

    ASSERT_EQ(false, bob_contact_id_->empty());
}
}  // namespace
