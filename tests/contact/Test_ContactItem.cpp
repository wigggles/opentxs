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

#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"

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
              opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
              std::string("testValue"),
              {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END)
    {
    }

    opentxs::ContactItem contactItem_;
};

}  // namespace

TEST_F(Test_ContactItem, first_constructor)
{
    const opentxs::ContactItem contactItem1(
        std::string("testContactItemNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        std::string("testValue"),
        {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END);

    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testContactItemNym",
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        "testValue"));
    ASSERT_EQ(contactItem1.ID(), identifier);
    ASSERT_EQ(contactItem1.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(
        contactItem1.Section(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
        contactItem1.Type(),
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
    ASSERT_EQ(contactItem1.Value(), "testValue");
    ASSERT_EQ(contactItem1.Start(), NULL_START);
    ASSERT_EQ(contactItem1.End(), NULL_END);

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
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        std::string("testValue"),
        {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
        NULL_START,
        NULL_END);
    ASSERT_EQ(contactItem1.Version(), CONTACT_CONTACT_DATA_VERSION);
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
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            "testValue",
            NULL_START,
            NULL_END,
            {opentxs::proto::CITEMATTR_ACTIVE}));

    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testContactItemNym",
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        "testValue"));
    ASSERT_EQ(contactItem1.ID(), identifier);
    ASSERT_EQ(contactItem1.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(
        contactItem1.Section(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
        contactItem1.Type(),
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
    ASSERT_EQ(contactItem1.Value(), "testValue");
    ASSERT_EQ(contactItem1.Start(), NULL_START);
    ASSERT_EQ(contactItem1.End(), NULL_END);

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

// TEST_F(Test_ContactItem, third_constructor)
//{
//	opentxs::proto::ContactItem data;
//	data.set_version(CONTACT_CONTACT_DATA_VERSION);
//	data.set_type(opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
//	data.set_value("testValue");
//	data.set_attribute(0, opentxs::proto::CITEMATTR_ACTIVE);
//	data.set_start(NULL_START);
//	data.set_end(NULL_END);
//
//    const opentxs::ContactItem contactItem1(
//        std::string("testContactItemNym"),
//        CONTACT_CONTACT_DATA_VERSION,
//		opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
//		data);
//
//    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
//        "testContactItemNym",
//        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
//        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
//        NULL_START,
//        NULL_END,
//        "testValue"));
//    ASSERT_EQ(contactItem1.ID(), identifier);
//    ASSERT_EQ(contactItem1.Version(), CONTACT_CONTACT_DATA_VERSION);
//    ASSERT_EQ(
//        contactItem1.Section(),
//        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
//    ASSERT_EQ(
//        contactItem1.Type(),
//        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
//    ASSERT_EQ(contactItem1.Value(), "testValue");
//    ASSERT_EQ(contactItem1.Start(), NULL_START);
//    ASSERT_EQ(contactItem1.End(), NULL_END);
//
//    ASSERT_TRUE(contactItem1.isActive());
//    ASSERT_FALSE(contactItem1.isLocal());
//    ASSERT_FALSE(contactItem1.isPrimary());
//}

TEST_F(Test_ContactItem, copy_constructor)
{
    opentxs::ContactItem copiedContactItem(contactItem_);

    ASSERT_EQ(copiedContactItem.ID(), contactItem_.ID());
    ASSERT_EQ(copiedContactItem.Version(), contactItem_.Version());
    ASSERT_EQ(copiedContactItem.Section(), contactItem_.Section());
    ASSERT_EQ(copiedContactItem.Type(), contactItem_.Type());
    ASSERT_EQ(copiedContactItem.Value(), contactItem_.Value());
    ASSERT_EQ(copiedContactItem.Start(), contactItem_.Start());
    ASSERT_EQ(copiedContactItem.End(), contactItem_.End());

    ASSERT_EQ(copiedContactItem.isActive(), contactItem_.isActive());
    ASSERT_EQ(copiedContactItem.isLocal(), contactItem_.isLocal());
    ASSERT_EQ(copiedContactItem.isPrimary(), contactItem_.isPrimary());
}

TEST_F(Test_ContactItem, move_constructor)
{
    opentxs::ContactItem movedContactItem(
        std::move<opentxs::ContactItem>(contactItem_.SetPrimary(true)));

    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        "testValue"));
    ASSERT_EQ(movedContactItem.ID(), identifier);
    ASSERT_EQ(movedContactItem.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_EQ(
    		movedContactItem.Section(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
    		movedContactItem.Type(),
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
    ASSERT_EQ(movedContactItem.Value(), "testValue");
    ASSERT_EQ(movedContactItem.Start(), NULL_START);
    ASSERT_EQ(movedContactItem.End(), NULL_END);

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
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
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

    opentxs::String id;
    contactItem_.ID().GetString(id);

    ASSERT_EQ(opentxs::String(protoItem.id()), id);
    ASSERT_EQ(protoItem.value(), contactItem_.Value());
    ASSERT_EQ(protoItem.version(), contactItem_.Version());
    ASSERT_EQ(protoItem.type(), contactItem_.Type());
    ASSERT_EQ(protoItem.start(), contactItem_.Start());
    ASSERT_EQ(protoItem.end(), contactItem_.End());
}

TEST_F(Test_ContactItem, operator_proto_not_equal)
{
    opentxs::proto::ContactItem protoItem = contactItem_;
    opentxs::proto::ContactItem protoItemNoId = contactItem_.Serialize();

    ASSERT_NE(protoItem.id(), protoItemNoId.id());
}

TEST_F(Test_ContactItem, public_accessors)
{
    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "testNym",
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        "testValue"));
    ASSERT_EQ(contactItem_.ID(), identifier);
    ASSERT_EQ(
        contactItem_.Section(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
        contactItem_.Type(),
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL);
    ASSERT_EQ(contactItem_.Value(), "testValue");
    ASSERT_EQ(contactItem_.Start(), NULL_START);
    ASSERT_EQ(contactItem_.End(), NULL_END);
    ASSERT_EQ(contactItem_.Version(), CONTACT_CONTACT_DATA_VERSION);

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
    ASSERT_EQ(startItem.Start(), now);
    ASSERT_NE(startItem.Start(), NULL_START);

    const auto& endItem = contactItem_.SetEnd(now);
    ASSERT_FALSE(endItem == contactItem_);
    ASSERT_EQ(endItem.End(), now);
    ASSERT_NE(endItem.End(), NULL_END);

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
