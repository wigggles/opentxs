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
#include "opentxs/core/Identifier.hpp"

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
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::ContactSection::GroupMap{})
        , contactGroup_(new opentxs::ContactGroup(
              std::string("testContactGroupNym1"),
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
              {}))
        , activeContactItem_(new opentxs::ContactItem(
              std::string("activeContactItem"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
              std::string("activeContactItemValue"),
              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END))
    {
    }

    const opentxs::ContactSection contactSection_;
    const std::shared_ptr<opentxs::ContactGroup> contactGroup_;
    const std::shared_ptr<opentxs::ContactItem> activeContactItem_;
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
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        groupMap);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
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
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::ContactSection::GroupMap{});
    ASSERT_EQ(section2.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactSection, second_constructor)
{
    const opentxs::ContactSection section1(
        "testContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        activeContactItem_);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(section1.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(section1.Size(), 1);
    ASSERT_NE(
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL),
        nullptr);
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
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    protoContactSection.set_version(CONTACT_CONTACT_DATA_VERSION);
    //    opentxs::proto::ContactItem * item = protoContactSection.add_item();

    const opentxs::ContactSection section1(
        "deserializedContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        protoContactSection);
    ASSERT_EQ(
        section1.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(section1.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactSection, copy_constructor)
{
    const auto& section1(contactSection_.AddItem(activeContactItem_));

    opentxs::ContactSection copiedContactSection(section1);
    ASSERT_EQ(copiedContactSection.Type(), section1.Type());
    ASSERT_EQ(copiedContactSection.Version(), section1.Version());
    ASSERT_EQ(copiedContactSection.Size(), 1);
    ASSERT_NE(
        copiedContactSection.Group(
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL),
        nullptr);
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
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(movedContactSection.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(movedContactSection.Size(), 1);
    ASSERT_NE(
        movedContactSection.Group(
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL),
        nullptr);
    ASSERT_EQ(
        movedContactSection
            .Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
    ASSERT_NE(movedContactSection.begin(), movedContactSection.end());
}

TEST_F(Test_ContactSection, operator_plus)
{
    // Combine two sections with one item each of the same type.
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const opentxs::ContactSection section2(
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem2);

    const auto& section3 = section1 + section2;
    // Verify the section has one group.
    ASSERT_EQ(section3.Size(), 1);
    // Verify the group has two items.
    ASSERT_EQ(
        section3.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        2);

    // Add a section that has one group with one item of the same type, and
    // another group with one item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const std::shared_ptr<opentxs::ContactItem> contactItem4(
        new opentxs::ContactItem(
            std::string("activeContactItem4"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue4"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const opentxs::ContactSection section4(
        "testContactSectionNym4",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem3);
    const auto& section5 = section4.AddItem(contactItem4);

    const auto& section6 = section3 + section5;
    // Verify the section has two groups.
    ASSERT_EQ(section6.Size(), 2);
    // Verify the first group has three items.
    ASSERT_EQ(
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        3);
    // Verify the second group has one item.
    ASSERT_EQ(
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION)
            ->Size(),
        1);
}

TEST_F(Test_ContactSection, AddItem)
{
    // Add an item to a SCOPE section.
    const std::shared_ptr<opentxs::ContactItem> scopeContactItem(
        new opentxs::ContactItem(
            std::string("scopeContactItem"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("scopeContactItemValue"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_LOCAL},
            NULL_START,
            NULL_END));
    const opentxs::ContactSection section1(
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        opentxs::ContactSection::GroupMap{});
    const auto& section2 = section1.AddItem(scopeContactItem);
    ASSERT_EQ(section2.Size(), 1);
    ASSERT_EQ(
        section2.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isPrimary());
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isActive());

    // Add two items of the same type.
    const auto& section4 = contactSection_.AddItem(activeContactItem_);
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section5 = section4.AddItem(contactItem2);
    // Verify there are two items.
    ASSERT_EQ(section5.Size(), 1);
    ASSERT_EQ(
        section5.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        2);

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section6 = section5.AddItem(contactItem3);
    // Verify there are two groups.
    ASSERT_EQ(section6.Size(), 2);
    // Verify there is one in the second group.
    ASSERT_EQ(
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION)
            ->Size(),
        1);
}

TEST_F(Test_ContactSection, begin)
{
    opentxs::ContactSection::GroupMap::const_iterator it =
        contactSection_.begin();
    ASSERT_EQ(it, contactSection_.end());
    ASSERT_EQ(std::distance(it, contactSection_.end()), 0);

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    it = section1.begin();
    ASSERT_NE(it, section1.end());
    ASSERT_EQ(std::distance(it, section1.end()), 1);

    std::advance(it, 1);
    ASSERT_EQ(it, section1.end());
    ASSERT_EQ(std::distance(it, section1.end()), 0);
}

TEST_F(Test_ContactSection, Claim_found)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    const std::shared_ptr<opentxs::ContactItem>& claim =
        section1.Claim(activeContactItem_->ID());
    ASSERT_NE(claim, nullptr);
    ASSERT_EQ(claim->ID(), activeContactItem_->ID());

    // Find a claim in a different group.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section2 = section1.AddItem(contactItem2);
    const std::shared_ptr<opentxs::ContactItem>& claim2 =
        section2.Claim(contactItem2->ID());
    ASSERT_NE(claim2, nullptr);
    ASSERT_EQ(claim2->ID(), contactItem2->ID());
}

TEST_F(Test_ContactSection, Claim_notfound)
{
    const std::shared_ptr<opentxs::ContactItem>& claim =
        contactSection_.Claim(activeContactItem_->ID());
    ASSERT_FALSE(claim);
}

TEST_F(Test_ContactSection, end)
{
    opentxs::ContactSection::GroupMap::const_iterator it =
        contactSection_.end();
    ASSERT_EQ(it, contactSection_.begin());
    ASSERT_EQ(std::distance(contactSection_.begin(), it), 0);

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    it = section1.end();
    ASSERT_NE(it, section1.begin());
    ASSERT_EQ(std::distance(section1.begin(), it), 1);

    std::advance(it, -1);
    ASSERT_EQ(it, section1.begin());
    ASSERT_EQ(std::distance(section1.begin(), it), 0);
}

TEST_F(Test_ContactSection, Group_found)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_NE(
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL),
        nullptr);
}

TEST_F(Test_ContactSection, Group_notfound)
{
    ASSERT_FALSE(contactSection_.Group(
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
}

TEST_F(Test_ContactSection, HaveClaim_true)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    ASSERT_TRUE(section1.HaveClaim(activeContactItem_->ID()));

    // Find a claim in a different group.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section2 = section1.AddItem(contactItem2);
    ASSERT_TRUE(section2.HaveClaim(contactItem2->ID()));
}

TEST_F(Test_ContactSection, HaveClaim_false)
{
    ASSERT_FALSE(contactSection_.HaveClaim(activeContactItem_->ID()));
}

TEST_F(Test_ContactSection, Delete)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_TRUE(section1.HaveClaim(activeContactItem_->ID()));

    // Add a second item to help testing the size after trying to delete twice.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section2 = section1.AddItem(contactItem2);
    ASSERT_EQ(
        section2.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        2);

    const auto& section3 = section2.Delete(activeContactItem_->ID());
    // Verify the item was deleted.
    ASSERT_FALSE(section3.HaveClaim(activeContactItem_->ID()));
    ASSERT_EQ(
        section3.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);

    const auto& section4 = section3.Delete(activeContactItem_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(
        section4.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section5 = section4.AddItem(contactItem3);
    // Verify the section has two groups.
    ASSERT_EQ(section5.Size(), 2);
    ASSERT_EQ(
        section5.Group(opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION)
            ->Size(),
        1);
    ASSERT_TRUE(section5.HaveClaim(contactItem3->ID()));

    const auto& section6 = section5.Delete(contactItem3->ID());
    // Verify the item was deleted and the group was removed.
    ASSERT_EQ(section6.Size(), 1);
    ASSERT_FALSE(section6.HaveClaim(contactItem3->ID()));
    ASSERT_FALSE(section6.Group(
        opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION));
}

TEST_F(Test_ContactSection, SerializeTo)
{
    // Serialize without ids.
    opentxs::proto::ContactData contactData1;

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_TRUE(section1.SerializeTo(contactData1, false));
    ASSERT_EQ(contactData1.section_size(), section1.Size());

    opentxs::proto::ContactSection contactDataSection = contactData1.section(0);
    ASSERT_EQ(
        contactDataSection.name(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);

    opentxs::proto::ContactItem contactDataItem = contactDataSection.item(0);
    ASSERT_EQ(contactDataItem.value(), activeContactItem_->Value());
    ASSERT_EQ(contactDataItem.version(), activeContactItem_->Version());
    ASSERT_EQ(contactDataItem.type(), activeContactItem_->Type());
    ASSERT_EQ(contactDataItem.start(), activeContactItem_->Start());
    ASSERT_EQ(contactDataItem.end(), activeContactItem_->End());

    // Serialize with ids.
    opentxs::proto::ContactData contactData2;

    ASSERT_TRUE(section1.SerializeTo(contactData2, true));
    ASSERT_EQ(contactData2.section_size(), section1.Size());

    contactDataSection = contactData2.section(0);
    ASSERT_EQ(
        contactDataSection.name(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);

    contactDataItem = contactDataSection.item(0);
    opentxs::String id;
    activeContactItem_->ID().GetString(id);

    ASSERT_EQ(opentxs::String(contactDataItem.id()), id);
    ASSERT_EQ(contactDataItem.value(), activeContactItem_->Value());
    ASSERT_EQ(contactDataItem.version(), activeContactItem_->Version());
    ASSERT_EQ(contactDataItem.type(), activeContactItem_->Type());
    ASSERT_EQ(contactDataItem.start(), activeContactItem_->Start());
    ASSERT_EQ(contactDataItem.end(), activeContactItem_->End());
}

TEST_F(Test_ContactSection, Size)
{
    ASSERT_EQ(contactSection_.Size(), 0);
    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_EQ(section1.Size(), 1);

    // Add a second item of the same type.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section2 = section1.AddItem(contactItem2);
    // Verify the size is the same.
    ASSERT_EQ(section2.Size(), 1);

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_ORGANIZATION,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& section3 = section2.AddItem(contactItem3);
    // Verify the size is now two.
    ASSERT_EQ(section3.Size(), 2);

    // Delete an item from the first group.
    const auto& section4 = section3.Delete(contactItem2->ID());
    // Verify the size is still two.
    ASSERT_EQ(section4.Size(), 2);

    // Delete the item from the second group.
    const auto& section5 = section4.Delete(contactItem3->ID());
    // Verify that the size is now one.
    ASSERT_EQ(section5.Size(), 1);
}

TEST_F(Test_ContactSection, Type)
{
    ASSERT_EQ(
        contactSection_.Type(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
}

TEST_F(Test_ContactSection, Version)
{
    ASSERT_EQ(contactSection_.Version(), CONTACT_CONTACT_DATA_VERSION);
}
