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
              opentxs::ContactSection::GroupMap{})
        , contactGroup_(new opentxs::ContactGroup(
              std::string("testContactGroupNym1"),
              opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
              {}))
        , activeContactItem_(new opentxs::ContactItem(
              std::string("activeContactItem"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
              std::string("activeContactItemValue"),
              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END))
    {
    }

    opentxs::ContactSection contactSection_;
    std::shared_ptr<opentxs::ContactGroup> contactGroup_;
    std::shared_ptr<opentxs::ContactItem> activeContactItem_;
};

}  // namespace

TEST_F(Test_ContactSection, first_constructor)
{
    const auto& group1 = std::shared_ptr<opentxs::ContactGroup>(
        new opentxs::ContactGroup(contactGroup_->AddItem(activeContactItem_)));
    opentxs::ContactSection::GroupMap groupMap{
        {opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL, group1}};

    const opentxs::ContactSection section1(
        "testContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        groupMap);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
    ASSERT_EQ(section1.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(section1.Size(), 1);
    ASSERT_EQ(
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        group1->Size());
    ASSERT_NE(section1.begin(), section1.end());
}

TEST_F(Test_ContactSection, first_constructor_different_versions)
{
    // Test private static method check_version.
    const opentxs::ContactSection section2(
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        opentxs::ContactSection::GroupMap{});
    ASSERT_EQ(section2.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactSection, second_constructor)
{
    const opentxs::ContactSection section1(
        "testContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        activeContactItem_);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
    ASSERT_EQ(section1.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(section1.Size(), 1);
    ASSERT_TRUE(
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_EQ(
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
    ASSERT_NE(section1.begin(), section1.end());
}

TEST_F(Test_ContactSection, third_constructor)
{
    opentxs::proto::ContactSection protoContactSection;
    protoContactSection.set_name(
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
    protoContactSection.set_version(CONTACT_CONTACT_DATA_VERSION);
    //    opentxs::proto::ContactItem * item = protoContactSection.add_item();

    const opentxs::ContactSection section1(
        "deserializedContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        protoContactSection);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
    ASSERT_EQ(section1.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactSection, copy_constructor)
{
    const auto& section1(contactSection_.AddItem(activeContactItem_));

    opentxs::ContactSection copiedContactSection(section1);
    ASSERT_EQ(copiedContactSection.Type(), section1.Type());
    ASSERT_EQ(copiedContactSection.Version(), section1.Version());
    ASSERT_EQ(copiedContactSection.Size(), 1);
    ASSERT_TRUE(copiedContactSection.Group(
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_EQ(
        copiedContactSection
            .Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
    ASSERT_NE(copiedContactSection.begin(), copiedContactSection.end());
}

TEST_F(Test_ContactSection, move_constructor)
{
    opentxs::ContactSection movedContactSection(
        std::move<opentxs::ContactSection>(
            contactSection_.AddItem(activeContactItem_)));

    ASSERT_EQ(
        movedContactSection.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE);
    ASSERT_EQ(movedContactSection.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(movedContactSection.Size(), 1);
    ASSERT_TRUE(movedContactSection.Group(
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_EQ(
        movedContactSection
            .Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
    ASSERT_NE(movedContactSection.begin(), movedContactSection.end());
    // Verify the source section doesn't have any items in its group.
    ASSERT_EQ(contactSection_.Size(), 0);
}

TEST_F(Test_ContactSection, operator_plus_scope)
{
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
    //    opentxs::ContactGroup::ItemMap::const_iterator it =
    //    contactGroup_.begin(); ASSERT_EQ(it, contactGroup_.end());
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
    //    opentxs::ContactGroup::ItemMap::const_iterator it =
    //    contactGroup_.end(); ASSERT_EQ(it, contactGroup_.begin());
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
    //    // Add a second item to help testing the size after trying to delete
    //    twice. const auto& group2 = group1.AddItem(primary_);
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
