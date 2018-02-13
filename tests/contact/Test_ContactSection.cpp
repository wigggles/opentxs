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

#include <gtest/gtest.h>

#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
//#include "opentxs/core/crypto/ContactCredential.hpp"

namespace
{

class Test_ContactSection : public ::testing::Test
{
public:
    Test_ContactSection()
        : contactSection_(
              std::string("testContactSectionNym1"),
			  CONTACT_CONTACT_DATA_VERSION,
			  CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
              {})
//        , primary_(new opentxs::ContactItem(
//              std::string("primaryContactItem"),
//              CONTACT_CONTACT_DATA_VERSION,
//              CONTACT_CONTACT_DATA_VERSION,
//              opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//              std::string("primaryContactItemValue"),
//              {opentxs::proto::ContactItemAttribute::CITEMATTR_PRIMARY},
//              NULL_START,
//              NULL_END))
//        , active_(new opentxs::ContactItem(
//              std::string("activeContactItem"),
//              CONTACT_CONTACT_DATA_VERSION,
//              CONTACT_CONTACT_DATA_VERSION,
//              opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//              std::string("activeContactItemValue"),
//              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
//              NULL_START,
//              NULL_END))
    {
    }

    opentxs::ContactSection contactSection_;
//    std::shared_ptr<opentxs::ContactItem> primary_;
//    std::shared_ptr<opentxs::ContactItem> active_;
};

}  // namespace

TEST_F(Test_ContactSection, first_constructor)
{
    // Test constructing a group with a map containing two primary items.
//    const std::shared_ptr<opentxs::ContactItem> primary2(
//        new opentxs::ContactItem(
//            std::string("primaryContactItemNym2"),
//            CONTACT_CONTACT_DATA_VERSION,
//            CONTACT_CONTACT_DATA_VERSION,
//            opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//            std::string("primaryContactItemValue2"),
//            {opentxs::proto::ContactItemAttribute::CITEMATTR_PRIMARY},
//            NULL_START,
//            NULL_END));
//
//    opentxs::ContactGroup::ItemMap map;
//    map[primary_->ID()] = primary_;
//    map[primary2->ID()] = primary2;
//
//    const opentxs::ContactGroup group1(
//        std::string("testContactGroupNym1"),
//        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//        map);
//    // Verify two items were added.
//    ASSERT_EQ(group1.Size(), 2);
//    ASSERT_EQ(
//        group1.Type(), opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
//    // Verify only one item is primary.
//    if (primary_->ID() == group1.Primary()) {
//        ASSERT_TRUE(group1.Claim(primary_->ID())->isPrimary());
//        ASSERT_FALSE(group1.Claim(primary2->ID())->isPrimary());
//    } else {
//        ASSERT_FALSE(group1.Claim(primary_->ID())->isPrimary());
//        ASSERT_TRUE(group1.Claim(primary2->ID())->isPrimary());
//    }
}

TEST_F(Test_ContactSection, first_constructor_no_items)
{
    // Test constructing a group with a map containing no items.
//    const opentxs::ContactGroup group1(
//        std::string("testContactGroupNym1"),
//        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//        {});
//    // Verify two items were added.
//    ASSERT_EQ(group1.Size(), 0);
//    ASSERT_EQ(
//        group1.Type(), opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
}

TEST_F(Test_ContactSection, second_constructor)
{
//    const opentxs::ContactGroup group1(
//        std::string("testContactGroupNym1"),
//        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//        active_);
//
//    ASSERT_EQ(group1.Size(), 1);
//    // Verify the group type matches the type of the item.
//    ASSERT_EQ(group1.Type(), active_->Type());
//    ASSERT_EQ(group1.begin()->second->ID(), active_->ID());
}

TEST_F(Test_ContactSection, operator_plus)
{
    // Test adding a group with a primary (rhs) to a group without a primary.
//    const auto& group1 = contactGroup_.AddItem(active_);
//    const auto& group2 = contactGroup_.AddItem(primary_);
//    const auto& group3 = group1 + group2;
//    ASSERT_EQ(group3.Size(), 2);
//    // Verify that the primary for the new group comes from rhs.
//    ASSERT_EQ(group3.Primary(), group2.Primary());
//
//    // Test adding a group with 2 items to a group with 1 item.
//    const std::shared_ptr<opentxs::ContactItem> primary2(
//        new opentxs::ContactItem(
//            std::string("primaryContactItemNym2"),
//            CONTACT_CONTACT_DATA_VERSION,
//            CONTACT_CONTACT_DATA_VERSION,
//            opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
//            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//            std::string("primaryContactItemValue2"),
//            {opentxs::proto::ContactItemAttribute::CITEMATTR_PRIMARY},
//            NULL_START,
//            NULL_END));
//    const auto& group4 = contactGroup_.AddItem(primary2);
//    const auto& group5 = contactGroup_.AddItem(primary_);
//    const auto& group6 = group5.AddItem(active_);
//
//    const auto& group7 = group4 + group6;
//    // Verify that the group has 3 items.
//    ASSERT_EQ(group7.Size(), 3);
//    // Verify that the primary of the new group came from the lhs.
//    ASSERT_EQ(group7.Primary(), group4.Primary());
//
//    for (auto it(group7.begin()); it != group7.end(); ++it) {
//        if (it->second->ID() == primary_->ID()) {
//            // Verify that the item that was primary on the rhs doesn't have
//            // the primary attribute.
//            ASSERT_FALSE(it->second->isPrimary());
//        }
//    }
}

TEST_F(Test_ContactSection, AddItem)
{
//    const auto& group1 = contactGroup_.AddItem(active_);
//    ASSERT_EQ(group1.Size(), 1);
//    ASSERT_EQ(group1.begin()->second->ID(), active_->ID());
//
//    // Test whether AddItem handles items that have already been added.
//    const auto& group2 = group1.AddItem(active_);
//    // Verify that there is still only one item.
//    ASSERT_EQ(group2.Size(), 1);
//
//    // Test that AddItem handles adding a primary.
//    const auto& group3 = contactGroup_.AddItem(primary_);
//    ASSERT_EQ(group3.Size(), 1);
//    ASSERT_EQ(group3.Primary(), primary_->ID());
//    ASSERT_TRUE(group3.begin()->second->isPrimary());
}

TEST_F(Test_ContactSection, begin)
{
//    opentxs::ContactGroup::ItemMap::const_iterator it = contactGroup_.begin();
//    ASSERT_EQ(it, contactGroup_.end());
//    ASSERT_EQ(std::distance(it, contactGroup_.end()), 0);
//
//    const auto& group1 = contactGroup_.AddItem(active_);
//    it = group1.begin();
//    ASSERT_NE(it, group1.end());
//    ASSERT_EQ(std::distance(it, group1.end()), 1);
//
//    std::advance(it, 1);
//    ASSERT_EQ(it, group1.end());
//    ASSERT_EQ(std::distance(it, group1.end()), 0);
}

TEST_F(Test_ContactSection, Claim_found)
{
//    const auto& group1 = contactGroup_.AddItem(active_);
//
//    const std::shared_ptr<opentxs::ContactItem>& claim =
//        group1.Claim(active_->ID());
//    ASSERT_TRUE(claim);
//    ASSERT_EQ(claim->ID(), active_->ID());
}

TEST_F(Test_ContactSection, Claim_notfound)
{
//    //
//    const std::shared_ptr<opentxs::ContactItem>& claim =
//        contactGroup_.Claim(active_->ID());
//    ASSERT_FALSE(claim);
}

TEST_F(Test_ContactSection, end)
{
//    opentxs::ContactGroup::ItemMap::const_iterator it = contactGroup_.end();
//    ASSERT_EQ(it, contactGroup_.begin());
//    ASSERT_EQ(std::distance(it, contactGroup_.begin()), 0);
//
//    const auto& group1 = contactGroup_.AddItem(active_);
//    it = group1.end();
//    ASSERT_NE(it, group1.begin());
//    ASSERT_EQ(std::distance(it, group1.begin()), 1);
//
//    std::advance(it, -1);
//    ASSERT_EQ(it, group1.begin());
//    ASSERT_EQ(std::distance(it, group1.begin()), 0);
}

TEST_F(Test_ContactSection, HaveClaim_true)
{
//    const auto& group1 = contactGroup_.AddItem(active_);
//
//    ASSERT_TRUE(group1.HaveClaim(active_->ID()));
}

TEST_F(Test_ContactSection, HaveClaim_false)
{
//    ASSERT_FALSE(contactGroup_.HaveClaim(active_->ID()));
}

TEST_F(Test_ContactSection, Delete)
{
//    const auto& group1 = contactGroup_.AddItem(active_);
//    ASSERT_TRUE(group1.HaveClaim(active_->ID()));
//
//    // Add a second item to help testing the size after trying to delete twice.
//    const auto& group2 = group1.AddItem(primary_);
//    ASSERT_EQ(group2.Size(), 2);
//
//    const auto& group3 = group2.Delete(active_->ID());
//    // Verify the item was deleted.
//    ASSERT_FALSE(group3.HaveClaim(active_->ID()));
//    ASSERT_EQ(group3.Size(), 1);
//
//    const auto& group4 = group3.Delete(active_->ID());
//    // Verify trying to delete the item again didn't change anything.
//    ASSERT_EQ(group4.Size(), 1);
}

TEST_F(Test_ContactSection, SerializeTo)
{
//    opentxs::proto::ContactSection contactSection1;
//    contactSection1.set_name(
//        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
//
//	const auto& group1 = contactGroup_.AddItem(active_);
//	ASSERT_TRUE(group1.SerializeTo(contactSection1, false));
//	ASSERT_EQ(contactSection1.item_size(), group1.Size());
//	ASSERT_EQ(contactSection1.name(),
//		opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
//	
//    opentxs::proto::ContactSection contactSection2;
//    contactSection2.set_name(
//        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
//
//	ASSERT_TRUE(group1.SerializeTo(contactSection2, true));
//	ASSERT_EQ(contactSection2.item_size(), group1.Size());
//	ASSERT_EQ(contactSection2.name(),
//		opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
//	
//    opentxs::proto::ContactSection contactSection3;
//    contactSection3.set_name(
//        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
//    ASSERT_FALSE(group1.SerializeTo(contactSection3, false));
}

TEST_F(Test_ContactSection, Size)
{
//    ASSERT_EQ(contactGroup_.Size(), 0);
//    const auto& group1 = contactGroup_.AddItem(primary_);
//    ASSERT_EQ(group1.Size(), 1);
//    const auto& group2 = group1.AddItem(active_);
//    ASSERT_EQ(group2.Size(), 2);
//    const auto& group3 = group2.Delete(active_->ID());
//    ASSERT_EQ(group3.Size(), 1);
}
