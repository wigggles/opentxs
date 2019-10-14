// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include "internal/api/client/Client.hpp"

#include <gtest/gtest.h>

namespace
{

class Test_ContactSection : public ::testing::Test
{
public:
    Test_ContactSection()
        : api_(dynamic_cast<const opentxs::api::client::internal::Manager&>(
              opentxs::Context().StartClient({}, 0)))
        , contactSection_(
              api_,
              std::string("testContactSectionNym1"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::ContactSection::GroupMap{})
        , contactGroup_(new opentxs::ContactGroup(
              std::string("testContactGroupNym1"),
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
              {}))
        , activeContactItem_(new opentxs::ContactItem(
              api_,
              std::string("activeContactItem"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
              std::string("activeContactItemValue"),
              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END,
              ""))
    {
    }

    const opentxs::api::client::internal::Manager& api_;
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
        {opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE, group1}};

    const opentxs::ContactSection section1(
        api_,
        "testContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        groupMap);
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        section1.Type());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section1.Version());
    ASSERT_EQ(1, section1.Size());
    ASSERT_EQ(
        group1->Size(),
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());
    ASSERT_NE(section1.end(), section1.begin());
}

TEST_F(Test_ContactSection, first_constructor_different_versions)
{
    // Test private static method check_version.
    const opentxs::ContactSection section2(
        api_,
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::ContactSection::GroupMap{});
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section2.Version());
}

TEST_F(Test_ContactSection, second_constructor)
{
    const opentxs::ContactSection section1(
        api_,
        "testContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        activeContactItem_);
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        section1.Type());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section1.Version());
    ASSERT_EQ(1, section1.Size());
    ASSERT_NE(
        nullptr,
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE));
    ASSERT_EQ(
        1,
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());
    ASSERT_NE(section1.end(), section1.begin());
}

TEST_F(Test_ContactSection, third_constructor)
{
    opentxs::proto::ContactSection protoContactSection;
    protoContactSection.set_name(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    protoContactSection.set_version(CONTACT_CONTACT_DATA_VERSION);
    //    opentxs::proto::ContactItem * item = protoContactSection.add_item();

    const opentxs::ContactSection section1(
        api_,
        "deserializedContactSectionNym1",
        CONTACT_CONTACT_DATA_VERSION,
        protoContactSection);
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        section1.Type());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section1.Version());
}

TEST_F(Test_ContactSection, copy_constructor)
{
    const auto& section1(contactSection_.AddItem(activeContactItem_));

    opentxs::ContactSection copiedContactSection(section1);
    ASSERT_EQ(section1.Type(), copiedContactSection.Type());
    ASSERT_EQ(section1.Version(), copiedContactSection.Version());
    ASSERT_EQ(1, copiedContactSection.Size());
    ASSERT_NE(
        nullptr,
        copiedContactSection.Group(
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE));
    ASSERT_EQ(
        1,
        copiedContactSection
            .Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());
    ASSERT_NE(copiedContactSection.end(), copiedContactSection.begin());
}

TEST_F(Test_ContactSection, operator_plus)
{
    // Combine two sections with one item each of the same type.
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const opentxs::ContactSection section2(
        api_,
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem2);

    const auto& section3 = section1 + section2;
    // Verify the section has one group.
    ASSERT_EQ(1, section3.Size());
    // Verify the group has two items.
    ASSERT_EQ(
        2,
        section3.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());

    // Add a section that has one group with one item of the same type, and
    // another group with one item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const std::shared_ptr<opentxs::ContactItem> contactItem4(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem4"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue4"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const opentxs::ContactSection section4(
        api_,
        "testContactSectionNym4",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem3);
    const auto& section5 = section4.AddItem(contactItem4);

    const auto& section6 = section3 + section5;
    // Verify the section has two groups.
    ASSERT_EQ(2, section6.Size());
    // Verify the first group has three items.
    ASSERT_EQ(
        3,
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());
    // Verify the second group has one item.
    ASSERT_EQ(
        1,
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_SSL)->Size());
}

TEST_F(Test_ContactSection, operator_plus_different_versions)
{
    // rhs version less than lhs
    const opentxs::ContactSection section2(
        api_,
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION - 1,
        CONTACT_CONTACT_DATA_VERSION - 1,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::ContactSection::GroupMap{});

    const auto& section3 = contactSection_ + section2;
    // Verify the new section has the latest version.
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section3.Version());

    // lhs version less than rhs
    const auto& section4 = section2 + contactSection_;
    // Verify the new section has the latest version.
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, section4.Version());
}

TEST_F(Test_ContactSection, AddItem)
{
    // Add an item to a SCOPE section.
    const std::shared_ptr<opentxs::ContactItem> scopeContactItem(
        new opentxs::ContactItem(
            api_,
            std::string("scopeContactItem"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("scopeContactItemValue"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_LOCAL},
            NULL_START,
            NULL_END,
            ""));
    const opentxs::ContactSection section1(
        api_,
        "testContactSectionNym2",
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        opentxs::ContactSection::GroupMap{});
    const auto& section2 = section1.AddItem(scopeContactItem);
    ASSERT_EQ(1, section2.Size());
    ASSERT_EQ(
        1,
        section2.Group(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size());
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isPrimary());
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isActive());

    // Add an item to a non-scope section.
    const auto& section4 = contactSection_.AddItem(activeContactItem_);
    ASSERT_EQ(1, section4.Size());
    ASSERT_EQ(
        1,
        section4.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());
    // Add a second item of the same type.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section5 = section4.AddItem(contactItem2);
    // Verify there are two items.
    ASSERT_EQ(1, section5.Size());
    ASSERT_EQ(
        2,
        section5.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section6 = section5.AddItem(contactItem3);
    // Verify there are two groups.
    ASSERT_EQ(2, section6.Size());
    // Verify there is one in the second group.
    ASSERT_EQ(
        1,
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_SSL)->Size());
}

TEST_F(Test_ContactSection, AddItem_different_versions)
{
    // Add an item with a newer version to a SCOPE section.
    const std::shared_ptr<opentxs::ContactItem> scopeContactItem(
        new opentxs::ContactItem(
            api_,
            std::string("scopeContactItem"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
            opentxs::proto::ContactItemType::CITEMTYPE_BOT,
            std::string("scopeContactItemValue"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_LOCAL},
            NULL_START,
            NULL_END,
            ""));
    const opentxs::ContactSection section1(
        api_,
        "testContactSectionNym2",
        3,  // version of CONTACTSECTION_SCOPE section before CITEMTYPE_BOT was
            // added
        3,
        opentxs::proto::ContactSectionName::CONTACTSECTION_SCOPE,
        opentxs::ContactSection::GroupMap{});
    const auto& section2 = section1.AddItem(scopeContactItem);
    ASSERT_EQ(1, section2.Size());
    ASSERT_EQ(
        1,
        section2.Group(opentxs::proto::ContactItemType::CITEMTYPE_BOT)->Size());
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isPrimary());
    ASSERT_TRUE(section2.Claim(scopeContactItem->ID())->isActive());
    // Verify the section version has been updated to the minimum version to
    // support CITEMTYPE_BOT.
    ASSERT_EQ(4, section2.Version());

    // Add an item with a newer version to a non-scope section.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("contactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_RELATIONSHIP,
            opentxs::proto::ContactItemType::CITEMTYPE_OWNER,
            std::string("contactItem2Value"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_LOCAL},
            NULL_START,
            NULL_END,
            ""));
    const opentxs::ContactSection section3(
        api_,
        "testContactSectionNym3",
        3,  // version of CONTACTSECTION_RELATIONSHIP section before
            // CITEMTYPE_OWNER was added
        3,
        opentxs::proto::ContactSectionName::CONTACTSECTION_RELATIONSHIP,
        opentxs::ContactSection::GroupMap{});
    const auto& section4 = section3.AddItem(contactItem2);
    ASSERT_EQ(1, section4.Size());
    ASSERT_EQ(
        1,
        section4.Group(opentxs::proto::ContactItemType::CITEMTYPE_OWNER)
            ->Size());
    // Verify the section version has been updated to the minimum version to
    // support CITEMTYPE_OWNER.
    ASSERT_EQ(4, section4.Version());
}

TEST_F(Test_ContactSection, begin)
{
    opentxs::ContactSection::GroupMap::const_iterator it =
        contactSection_.begin();
    ASSERT_EQ(contactSection_.end(), it);
    ASSERT_EQ(0, std::distance(it, contactSection_.end()));

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    it = section1.begin();
    ASSERT_NE(section1.end(), it);
    ASSERT_EQ(1, std::distance(it, section1.end()));

    std::advance(it, 1);
    ASSERT_EQ(section1.end(), it);
    ASSERT_EQ(0, std::distance(it, section1.end()));
}

TEST_F(Test_ContactSection, Claim_found)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    const std::shared_ptr<opentxs::ContactItem>& claim =
        section1.Claim(activeContactItem_->ID());
    ASSERT_NE(nullptr, claim);
    ASSERT_EQ(activeContactItem_->ID(), claim->ID());

    // Find a claim in a different group.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section2 = section1.AddItem(contactItem2);
    const std::shared_ptr<opentxs::ContactItem>& claim2 =
        section2.Claim(contactItem2->ID());
    ASSERT_NE(nullptr, claim2);
    ASSERT_EQ(contactItem2->ID(), claim2->ID());
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
    ASSERT_EQ(contactSection_.begin(), it);
    ASSERT_EQ(0, std::distance(contactSection_.begin(), it));

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    it = section1.end();
    ASSERT_NE(section1.begin(), it);
    ASSERT_EQ(1, std::distance(section1.begin(), it));

    std::advance(it, -1);
    ASSERT_EQ(section1.begin(), it);
    ASSERT_EQ(0, std::distance(section1.begin(), it));
}

TEST_F(Test_ContactSection, Group_found)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_NE(
        nullptr,
        section1.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE));
}

TEST_F(Test_ContactSection, Group_notfound)
{
    ASSERT_FALSE(contactSection_.Group(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE));
}

TEST_F(Test_ContactSection, HaveClaim_true)
{
    const auto& section1 = contactSection_.AddItem(activeContactItem_);

    ASSERT_TRUE(section1.HaveClaim(activeContactItem_->ID()));

    // Find a claim in a different group.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
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
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section2 = section1.AddItem(contactItem2);
    ASSERT_EQ(
        2,
        section2.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());

    const auto& section3 = section2.Delete(activeContactItem_->ID());
    // Verify the item was deleted.
    ASSERT_FALSE(section3.HaveClaim(activeContactItem_->ID()));
    ASSERT_EQ(
        1,
        section3.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());

    const auto& section4 = section3.Delete(activeContactItem_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(
        1,
        section4.Group(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE)
            ->Size());

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section5 = section4.AddItem(contactItem3);
    // Verify the section has two groups.
    ASSERT_EQ(2, section5.Size());
    ASSERT_EQ(
        1,
        section5.Group(opentxs::proto::ContactItemType::CITEMTYPE_SSL)->Size());
    ASSERT_TRUE(section5.HaveClaim(contactItem3->ID()));

    const auto& section6 = section5.Delete(contactItem3->ID());
    // Verify the item was deleted and the group was removed.
    ASSERT_EQ(1, section6.Size());
    ASSERT_FALSE(section6.HaveClaim(contactItem3->ID()));
    ASSERT_FALSE(
        section6.Group(opentxs::proto::ContactItemType::CITEMTYPE_SSL));
}

TEST_F(Test_ContactSection, SerializeTo)
{
    // Serialize without ids.
    opentxs::proto::ContactData contactData1;

    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_TRUE(section1.SerializeTo(contactData1, false));
    ASSERT_EQ(section1.Size(), contactData1.section_size());

    opentxs::proto::ContactSection contactDataSection = contactData1.section(0);
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactDataSection.name());

    opentxs::proto::ContactItem contactDataItem = contactDataSection.item(0);
    ASSERT_EQ(activeContactItem_->Value(), contactDataItem.value());
    ASSERT_EQ(activeContactItem_->Version(), contactDataItem.version());
    ASSERT_EQ(activeContactItem_->Type(), contactDataItem.type());
    ASSERT_EQ(activeContactItem_->Start(), contactDataItem.start());
    ASSERT_EQ(activeContactItem_->End(), contactDataItem.end());

    // Serialize with ids.
    opentxs::proto::ContactData contactData2;

    ASSERT_TRUE(section1.SerializeTo(contactData2, true));
    ASSERT_EQ(section1.Size(), contactData2.section_size());

    contactDataSection = contactData2.section(0);
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactDataSection.name());

    contactDataItem = contactDataSection.item(0);

    ASSERT_EQ(activeContactItem_->ID().str(), contactDataItem.id());
    ASSERT_EQ(activeContactItem_->Value(), contactDataItem.value());
    ASSERT_EQ(activeContactItem_->Version(), contactDataItem.version());
    ASSERT_EQ(activeContactItem_->Type(), contactDataItem.type());
    ASSERT_EQ(activeContactItem_->Start(), contactDataItem.start());
    ASSERT_EQ(activeContactItem_->End(), contactDataItem.end());
}

TEST_F(Test_ContactSection, Size)
{
    ASSERT_EQ(0, contactSection_.Size());
    const auto& section1 = contactSection_.AddItem(activeContactItem_);
    ASSERT_EQ(1, section1.Size());

    // Add a second item of the same type.
    const std::shared_ptr<opentxs::ContactItem> contactItem2(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            std::string("activeContactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section2 = section1.AddItem(contactItem2);
    // Verify the size is the same.
    ASSERT_EQ(1, section2.Size());

    // Add an item of a different type.
    const std::shared_ptr<opentxs::ContactItem> contactItem3(
        new opentxs::ContactItem(
            api_,
            std::string("activeContactItem3"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_SSL,
            std::string("activeContactItemValue3"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END,
            ""));
    const auto& section3 = section2.AddItem(contactItem3);
    // Verify the size is now two.
    ASSERT_EQ(2, section3.Size());

    // Delete an item from the first group.
    const auto& section4 = section3.Delete(contactItem2->ID());
    // Verify the size is still two.
    ASSERT_EQ(2, section4.Size());

    // Delete the item from the second group.
    const auto& section5 = section4.Delete(contactItem3->ID());
    // Verify that the size is now one.
    ASSERT_EQ(1, section5.Size());
}

TEST_F(Test_ContactSection, Type)
{
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactSection_.Type());
}

TEST_F(Test_ContactSection, Version)
{
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactSection_.Version());
}
