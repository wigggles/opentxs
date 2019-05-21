// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"
#include "Internal.hpp"

#include <gtest/gtest.h>

namespace
{

class Test_ContactItem : public ::testing::Test
{
public:
    Test_ContactItem()
        : contactItem_(
              std::string("testNym"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
              std::string("testValue"),
              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END)
    {
    }

    const opentxs::ContactItem contactItem_;
};

}  // namespace

TEST_F(Test_ContactItem, first_constructor)
{
    const opentxs::ContactItem contactItem1(
        std::string("testContactItemNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        std::string("testValue"),
        {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END);

    const opentxs::OTIdentifier identifier(opentxs::Identifier::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            "testContactItemNym",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            NULL_START,
            NULL_END,
            "testValue")));
    ASSERT_EQ(identifier, contactItem1.ID());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem1.Section());
    ASSERT_EQ(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        contactItem1.Type());
    ASSERT_EQ("testValue", contactItem1.Value());
    ASSERT_EQ(NULL_START, contactItem1.Start());
    ASSERT_EQ(NULL_END, contactItem1.End());

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, first_constructor_different_versions)
{
    const opentxs::ContactItem contactItem1(
        std::string("testContactItemNym"),
        CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        std::string("testValue"),
        {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END);
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
}

TEST_F(Test_ContactItem, second_constructor)
{
    const opentxs::ContactItem contactItem1(
        std::string("testContactItemNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::Claim(
            "",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            "testValue",
            NULL_START,
            NULL_END,
            {opentxs::proto::CITEMATTR_ACTIVE}));

    const opentxs::OTIdentifier identifier(opentxs::Identifier::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            "testContactItemNym",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            NULL_START,
            NULL_END,
            "testValue")));
    ASSERT_EQ(identifier, contactItem1.ID());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem1.Section());
    ASSERT_EQ(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        contactItem1.Type());
    ASSERT_EQ("testValue", contactItem1.Value());
    ASSERT_EQ(NULL_START, contactItem1.Start());
    ASSERT_EQ(NULL_END, contactItem1.End());

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, third_constructor)
{
    opentxs::proto::ContactItem data;
    data.set_version(CONTACT_CONTACT_DATA_VERSION);
    data.set_type(opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE);
    data.set_value("testValue");
    data.add_attribute(opentxs::proto::CITEMATTR_ACTIVE);
    data.set_start(NULL_START);
    data.set_end(NULL_END);

    const opentxs::ContactItem contactItem1(
        std::string("testContactItemNym"),
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        data);

    const opentxs::OTIdentifier identifier(opentxs::Identifier::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            "testContactItemNym",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            NULL_START,
            NULL_END,
            "testValue")));
    ASSERT_EQ(identifier, contactItem1.ID());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem1.Section());
    ASSERT_EQ(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        contactItem1.Type());
    ASSERT_EQ("testValue", contactItem1.Value());
    ASSERT_EQ(NULL_START, contactItem1.Start());
    ASSERT_EQ(NULL_END, contactItem1.End());

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, copy_constructor)
{
    opentxs::ContactItem copiedContactItem(contactItem_);

    ASSERT_EQ(contactItem_.ID(), copiedContactItem.ID());
    ASSERT_EQ(contactItem_.Version(), copiedContactItem.Version());
    ASSERT_EQ(contactItem_.Section(), copiedContactItem.Section());
    ASSERT_EQ(contactItem_.Type(), copiedContactItem.Type());
    ASSERT_EQ(contactItem_.Value(), copiedContactItem.Value());
    ASSERT_EQ(contactItem_.Start(), copiedContactItem.Start());
    ASSERT_EQ(contactItem_.End(), copiedContactItem.End());

    ASSERT_EQ(contactItem_.isActive(), copiedContactItem.isActive());
    ASSERT_EQ(contactItem_.isLocal(), copiedContactItem.isLocal());
    ASSERT_EQ(contactItem_.isPrimary(), copiedContactItem.isPrimary());
}

TEST_F(Test_ContactItem, move_constructor)
{
    opentxs::ContactItem movedContactItem(
        std::move<opentxs::ContactItem>(contactItem_.SetPrimary(true)));

    const opentxs::OTIdentifier identifier(opentxs::Identifier::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            "testNym",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            NULL_START,
            NULL_END,
            "testValue")));
    ASSERT_EQ(identifier, movedContactItem.ID());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, movedContactItem.Version());
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        movedContactItem.Section());
    ASSERT_EQ(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        movedContactItem.Type());
    ASSERT_EQ("testValue", movedContactItem.Value());
    ASSERT_EQ(NULL_START, movedContactItem.Start());
    ASSERT_EQ(NULL_END, movedContactItem.End());

    ASSERT_TRUE(movedContactItem.isActive());
    ASSERT_FALSE(movedContactItem.isLocal());
    ASSERT_TRUE(movedContactItem.isPrimary());
}

TEST_F(Test_ContactItem, operator_equal_true)
{
    ASSERT_EQ(contactItem_, contactItem_);
}

TEST_F(Test_ContactItem, operator_equal_false)
{
    opentxs::ContactItem contactItem2(
        std::string("testNym2"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        std::string("testValue2"),
        {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END);

    // Can't use ASSERT_NE because there's no != operator defined for
    // ContactItem.
    ASSERT_FALSE(contactItem_ == contactItem2);
}

TEST_F(Test_ContactItem, operator_proto_equal)
{
    opentxs::proto::ContactItem protoItem = contactItem_;

    ASSERT_EQ(contactItem_.ID().str(), protoItem.id());
    ASSERT_EQ(contactItem_.Value(), protoItem.value());
    ASSERT_EQ(contactItem_.Version(), protoItem.version());
    ASSERT_EQ(contactItem_.Type(), protoItem.type());
    ASSERT_EQ(contactItem_.Start(), protoItem.start());
    ASSERT_EQ(contactItem_.End(), protoItem.end());
}

TEST_F(Test_ContactItem, operator_proto_not_equal)
{
    opentxs::proto::ContactItem protoItem = contactItem_;
    opentxs::proto::ContactItem protoItemNoId = contactItem_.Serialize();

    ASSERT_NE(protoItemNoId.id(), protoItem.id());
}

TEST_F(Test_ContactItem, public_accessors)
{
    const opentxs::OTIdentifier identifier(opentxs::Identifier::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            "testNym",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
            NULL_START,
            NULL_END,
            "testValue")));
    ASSERT_EQ(identifier, contactItem_.ID());
    ASSERT_EQ(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        contactItem_.Section());
    ASSERT_EQ(
        opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE,
        contactItem_.Type());
    ASSERT_EQ("testValue", contactItem_.Value());
    ASSERT_EQ(NULL_START, contactItem_.Start());
    ASSERT_EQ(NULL_END, contactItem_.End());
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactItem_.Version());

    ASSERT_TRUE(contactItem_.isActive());
    ASSERT_FALSE(contactItem_.isLocal());
    ASSERT_FALSE(contactItem_.isPrimary());
}

TEST_F(Test_ContactItem, public_setters)
{
    const auto now = std::time(nullptr);

    const auto& valueItem = contactItem_.SetValue("newTestValue");
    ASSERT_FALSE(valueItem == contactItem_);
    ASSERT_STREQ(valueItem.Value().c_str(), "newTestValue");

    const auto& startItem = contactItem_.SetStart(now);
    ASSERT_FALSE(startItem == contactItem_);
    ASSERT_EQ(now, startItem.Start());
    ASSERT_NE(NULL_START, startItem.Start());

    const auto& endItem = contactItem_.SetEnd(now);
    ASSERT_FALSE(endItem == contactItem_);
    ASSERT_EQ(now, endItem.End());
    ASSERT_NE(NULL_END, endItem.End());

    // _contactItem is active, so test setting active to false first.
    const auto& notActiveItem = contactItem_.SetActive(false);
    ASSERT_FALSE(notActiveItem == contactItem_);
    ASSERT_FALSE(notActiveItem.isActive());
    const auto& activeItem = notActiveItem.SetActive(true);
    ASSERT_FALSE(activeItem == notActiveItem);
    ASSERT_TRUE(activeItem.isActive());

    const auto& localItem = contactItem_.SetLocal(true);
    ASSERT_FALSE(localItem == contactItem_);
    ASSERT_TRUE(localItem.isLocal());
    const auto& notLocalItem = localItem.SetLocal(false);
    ASSERT_FALSE(notLocalItem == localItem);
    ASSERT_FALSE(notLocalItem.isLocal());

    // First, create an item with no attributes.
    const auto& notPrimaryItem = contactItem_.SetActive(false);
    ASSERT_FALSE(notPrimaryItem == contactItem_);
    ASSERT_FALSE(notPrimaryItem.isPrimary());
    ASSERT_FALSE(notPrimaryItem.isActive());
    ASSERT_FALSE(notPrimaryItem.isLocal());
    // Now, set the primary attribute, and test for primary and active.
    const auto& primaryItem = notPrimaryItem.SetPrimary(true);
    ASSERT_FALSE(primaryItem == notPrimaryItem);
    ASSERT_TRUE(primaryItem.isPrimary());
    ASSERT_TRUE(primaryItem.isActive());
}

TEST_F(Test_ContactItem, Serialize)
{
    // Test without id.
    opentxs::proto::ContactItem protoItem = contactItem_.Serialize();

    ASSERT_EQ(contactItem_.Value(), protoItem.value());
    ASSERT_EQ(contactItem_.Version(), protoItem.version());
    ASSERT_EQ(contactItem_.Type(), protoItem.type());
    ASSERT_EQ(contactItem_.Start(), protoItem.start());
    ASSERT_EQ(contactItem_.End(), protoItem.end());

    // Test with id.
    protoItem = contactItem_.Serialize(true);

    ASSERT_EQ(contactItem_.ID().str(), protoItem.id());
    ASSERT_EQ(contactItem_.Value(), protoItem.value());
    ASSERT_EQ(contactItem_.Version(), protoItem.version());
    ASSERT_EQ(contactItem_.Type(), protoItem.type());
    ASSERT_EQ(contactItem_.Start(), protoItem.start());
    ASSERT_EQ(contactItem_.End(), protoItem.end());
}
