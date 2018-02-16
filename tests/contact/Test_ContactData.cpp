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

#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/crypto/ContactCredential.hpp"

namespace
{

class Test_ContactData : public ::testing::Test
{
public:
    Test_ContactData()
        : contactData_(
              std::string("contactDataNym"),
              CONTACT_CONTACT_DATA_VERSION,
              CONTACT_CONTACT_DATA_VERSION,
              {})
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

    const opentxs::ContactData contactData_;
    const std::shared_ptr<opentxs::ContactItem> activeContactItem_;
};

static const auto& expectedStringOutput =
    "Version 5 contact data\nSections found: 1\n- Section: Identifier, "
    "version: 5 containing 1 item(s).\n-- Item type: \"Individual\", "
    "value: \"activeContactItemValue\", start: 0, end: 0, version: 5\n--- "
    "Attributes: Active \n";

}  // namespace

TEST_F(Test_ContactData, PrintContactData)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const auto& dataString =
        opentxs::ContactData::PrintContactData(data1.Serialize());
    ASSERT_STREQ(dataString.c_str(), expectedStringOutput);
}

TEST_F(Test_ContactData, first_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    const opentxs::ContactData::SectionMap map{{section1->Type(), section1}};

    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        map);
    ASSERT_EQ(contactData.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_TRUE(contactData.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    ASSERT_TRUE(contactData.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_TRUE(contactData.HaveClaim(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, first_constructor_no_sections)
{
    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(contactData.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactData, first_constructor_different_versions)
{
    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(contactData.Version(), CONTACT_CONTACT_DATA_VERSION);
}

TEST_F(Test_ContactData, second_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    opentxs::proto::ContactData data;
    data.set_version(CONTACT_CONTACT_DATA_VERSION);

    section1->SerializeTo(data, false);

    const opentxs::ContactData contactData(
        std::string("contactDataNym"), CONTACT_CONTACT_DATA_VERSION, data);

    ASSERT_EQ(contactData.Version(), data.version());
    ASSERT_TRUE(contactData.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    ASSERT_TRUE(contactData.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_TRUE(contactData.HaveClaim(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, second_constructor_no_sections)
{
    opentxs::proto::ContactData data;
    data.set_version(CONTACT_CONTACT_DATA_VERSION);

    const opentxs::ContactData contactData(
        std::string("contactDataNym"), CONTACT_CONTACT_DATA_VERSION, data);
    ASSERT_EQ(contactData.Version(), data.version());
}

TEST_F(Test_ContactData, copy_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    const opentxs::ContactData::SectionMap map{{section1->Type(), section1}};

    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        map);

    const opentxs::ContactData copiedContactData(contactData);

    ASSERT_EQ(copiedContactData.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_TRUE(copiedContactData.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    ASSERT_TRUE(copiedContactData.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_TRUE(copiedContactData.HaveClaim(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, move_constructor)
{
    const opentxs::ContactData movedContactData(std::move<opentxs::ContactData>(
        contactData_.AddItem(activeContactItem_)));

    ASSERT_EQ(movedContactData.Version(), CONTACT_CONTACT_DATA_VERSION);
    ASSERT_TRUE(movedContactData.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    ASSERT_TRUE(movedContactData.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    ASSERT_TRUE(movedContactData.HaveClaim(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, operator_plus)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);

    // Add a ContactData object with a section of the same type.
    const auto& contactItem2 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("contactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));

    const auto& group2 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup2",
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            contactItem2));

    const auto& section2 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym2",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
            opentxs::ContactSection::GroupMap{{contactItem2->Type(), group2}}));

    const opentxs::ContactData data2(
        std::string("contactDataNym2"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
             section2}});

    const auto& data3 = data1 + data2;
    // Verify the section exists.
    ASSERT_TRUE(data3.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    // Verify it has one group.
    ASSERT_EQ(
        data3
            .Section(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER)
            ->Size(),
        1);
    // Verify the group exists.
    ASSERT_TRUE(data3.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    // Verify it has two items.
    ASSERT_EQ(
        data3
            .Group(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
                opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        2);

    // Add a ContactData object with a section of a different type.
    const auto& contactItem4 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem4"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
            opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL,
            std::string("contactItemValue4"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));

    const auto& group4 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup4",
            opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
            contactItem4));

    const auto& section4 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym4",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
            opentxs::ContactSection::GroupMap{{contactItem4->Type(), group4}}));

    const opentxs::ContactData data4(
        std::string("contactDataNym4"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
             section4}});

    const auto& data5 = data3 + data4;
    // Verify the first section exists.
    ASSERT_TRUE(data5.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER));
    // Verify it has one group.
    ASSERT_EQ(
        data5
            .Section(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER)
            ->Size(),
        1);
    // Verify the group exists.
    ASSERT_TRUE(data5.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    // Verify it has two items.
    ASSERT_EQ(
        data5
            .Group(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER,
                opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        2);

    // Verify the second section exists.
    ASSERT_TRUE(data5.Section(
        opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS));
    // Verify it has one group.
    ASSERT_EQ(
        data5
            .Section(opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS)
            ->Size(),
        1);
    // Verify the group exists.
    ASSERT_TRUE(data5.Group(
        opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
        opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL));
    // Verify it has one item.
    ASSERT_EQ(
        data5
            .Group(
                opentxs::proto::ContactSectionName::CONTACTSECTION_ADDRESS,
                opentxs::proto::ContactItemType::CITEMTYPE_INDIVIDUAL)
            ->Size(),
        1);
}

TEST_F(Test_ContactData, operator_string)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const std::string dataString = data1;
    ASSERT_EQ(dataString, expectedStringOutput);
}

TEST_F(Test_ContactData, Serialize_)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);

    // Serialize without ids.
    opentxs::proto::ContactData protoData = data1.Serialize(false);

    ASSERT_EQ(protoData.version(), data1.Version());
    ASSERT_EQ(protoData.section_size(), 1);
    opentxs::proto::ContactSection protoSection = protoData.section(0);
    ASSERT_EQ(
        protoSection.name(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
        protoSection.item_size(),
        data1
            .Section(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER)
            ->Size());
    opentxs::proto::ContactItem protoItem = protoSection.item(0);
    ASSERT_EQ(protoItem.value(), activeContactItem_->Value());
    ASSERT_EQ(protoItem.version(), activeContactItem_->Version());
    ASSERT_EQ(protoItem.type(), activeContactItem_->Type());
    ASSERT_EQ(protoItem.start(), activeContactItem_->Start());
    ASSERT_EQ(protoItem.end(), activeContactItem_->End());

    // Serialize with ids.
    protoData = data1.Serialize(true);

    ASSERT_EQ(protoData.version(), data1.Version());
    ASSERT_EQ(protoData.section_size(), 1);
    protoSection = protoData.section(0);
    ASSERT_EQ(
        protoSection.name(),
        opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER);
    ASSERT_EQ(
        protoSection.item_size(),
        data1
            .Section(
                opentxs::proto::ContactSectionName::CONTACTSECTION_IDENTIFIER)
            ->Size());
    protoItem = protoSection.item(0);
    opentxs::String id;
    activeContactItem_->ID().GetString(id);

    ASSERT_EQ(opentxs::String(protoItem.id()), id);
    ASSERT_EQ(protoItem.value(), activeContactItem_->Value());
    ASSERT_EQ(protoItem.version(), activeContactItem_->Version());
    ASSERT_EQ(protoItem.type(), activeContactItem_->Type());
    ASSERT_EQ(protoItem.start(), activeContactItem_->Start());
    ASSERT_EQ(protoItem.end(), activeContactItem_->End());
}

TEST_F(Test_ContactData, AddContract)
{
    // Add a contract to a group with no primary.
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_BTC,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_BTC, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
             section1}});

    const auto& data2 = data1.AddContract(
        "instrumentDefinitionID1", opentxs::proto::CITEMTYPE_BTC, false, false);

    // Verify that the item was made primary.
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
        opentxs::proto::ContactItemType::CITEMTYPE_BTC,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID1"));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_TRUE(contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contract to a group with a primary.
    const auto& data3 = data2.AddContract(
        "instrumentDefinitionID2", opentxs::proto::CITEMTYPE_BTC, false, false);
    // Verify that the item wasn't made primary.
    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
        opentxs::proto::ContactItemType::CITEMTYPE_BTC,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID2"));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_TRUE(contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contract for a type with no group.
    const auto& data4 = data3.AddContract(
        "instrumentDefinitionID3", opentxs::proto::CITEMTYPE_EUR, false, false);
    // Verify the group was created.
    ASSERT_TRUE(data4.Group(
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_EUR));
    // Verify that the item was made primary.
    const opentxs::Identifier identifier3(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
        opentxs::proto::ContactItemType::CITEMTYPE_EUR,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID3"));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_TRUE(contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contract.
    const auto& data5 = data4.AddContract(
        "instrumentDefinitionID4", opentxs::proto::CITEMTYPE_USD, false, true);
    // Verify the group was created.
    ASSERT_TRUE(data5.Group(
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD));
    // Verify that the item was made active.
    const opentxs::Identifier identifier4(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
        opentxs::proto::ContactItemType::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID4"));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_TRUE(contactItem4);
    ASSERT_TRUE(contactItem4->isActive());

    // Add a primary contract.
    const auto& data6 = data5.AddContract(
        "instrumentDefinitionID5", opentxs::proto::CITEMTYPE_USD, true, false);
    // Verify that the item was made primary.
    const opentxs::Identifier identifier5(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::ContactSectionName::CONTACTSECTION_CONTRACT,
        opentxs::proto::ContactItemType::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID5"));
    const auto& contactItem5 = data6.Claim(identifier5);
    ASSERT_TRUE(contactItem5);
    ASSERT_TRUE(contactItem5->isPrimary());
}
