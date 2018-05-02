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
#include "opentxs/core/Identifier.hpp"

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
              opentxs::proto::CONTACTSECTION_IDENTIFIER,
              opentxs::proto::CITEMTYPE_EMPLOYEE,
              std::string("activeContactItemValue"),
              {opentxs::proto::CITEMATTR_ACTIVE},
              NULL_START,
              NULL_END))
    {
    }

    const opentxs::ContactData contactData_;
    const std::shared_ptr<opentxs::ContactItem> activeContactItem_;

    void testAddItemMethod(
        std::function<opentxs::ContactData(
            const opentxs::ContactData&,
            const std::string&,
            const opentxs::proto::ContactItemType,
            const bool,
            const bool)> contactDataMethod,
        opentxs::proto::ContactSectionName sectionName,
        uint32_t version = CONTACT_CONTACT_DATA_VERSION,
        uint32_t targetVersion = 0);
};

void Test_ContactData::testAddItemMethod(
    std::function<opentxs::ContactData(
        const opentxs::ContactData&,
        const std::string&,
        const opentxs::proto::ContactItemType,
        const bool,
        const bool)> contactDataMethod,
    opentxs::proto::ContactSectionName sectionName,
    uint32_t version,
    uint32_t targetVersion)
{
    // Add a contact to a group with no primary.
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1", sectionName, opentxs::proto::CITEMTYPE_BCH, {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            version,
            version,
            sectionName,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_BCH, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        version,
        version,
        opentxs::ContactData::SectionMap{{sectionName, section1}});

    const auto& data2 = contactDataMethod(
        data1,
        "instrumentDefinitionID1",
        opentxs::proto::CITEMTYPE_BCH,
        false,
        false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        sectionName,
        opentxs::proto::CITEMTYPE_BCH,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID1"));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contact to a group with a primary.
    const auto& data3 = contactDataMethod(
        data2,
        "instrumentDefinitionID2",
        opentxs::proto::CITEMTYPE_BCH,
        false,
        false);

    // Verify that the item wasn't made primary.
    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        sectionName,
        opentxs::proto::CITEMTYPE_BCH,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID2"));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contact for a type with no group.
    const auto& data4 = contactDataMethod(
        data3,
        "instrumentDefinitionID3",
        opentxs::proto::CITEMTYPE_EUR,
        false,
        false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data4.Group(sectionName, opentxs::proto::CITEMTYPE_EUR));
    // Verify that the item was made primary.
    const opentxs::Identifier identifier3(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        sectionName,
        opentxs::proto::CITEMTYPE_EUR,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID3"));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contact.
    const auto& data5 = contactDataMethod(
        data4,
        "instrumentDefinitionID4",
        opentxs::proto::CITEMTYPE_USD,
        false,
        true);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, opentxs::proto::CITEMTYPE_USD));
    // Verify that the item was made active.
    const opentxs::Identifier identifier4(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        sectionName,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID4"));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isActive());

    // Add a primary contact.
    const auto& data6 = contactDataMethod(
        data5,
        "instrumentDefinitionID5",
        opentxs::proto::CITEMTYPE_USD,
        true,
        false);

    // Verify that the item was made primary.
    const opentxs::Identifier identifier5(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        sectionName,
        opentxs::proto::CITEMTYPE_USD,
        NULL_START,
        NULL_END,
        "instrumentDefinitionID5"));
    const auto& contactItem5 = data6.Claim(identifier5);
    ASSERT_NE(nullptr, contactItem5);
    ASSERT_TRUE(contactItem5->isPrimary());
}

static const auto& expectedStringOutput =
    "Version 5 contact data\nSections found: 1\n- Section: Identifier, "
    "version: 5 containing 1 item(s).\n-- Item type: \"employee of\", "
    "value: \"activeContactItemValue\", start: 0, end: 0, version: 5\n--- "
    "Attributes: Active \n";

}  // namespace

TEST_F(Test_ContactData, PrintContactData)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const auto& dataString =
        opentxs::ContactData::PrintContactData(data1.Serialize());
    ASSERT_STREQ(expectedStringOutput, dataString.c_str());
}

TEST_F(Test_ContactData, first_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    const opentxs::ContactData::SectionMap map{{section1->Type(), section1}};

    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        map);
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData.Version());
    ASSERT_NE(
        nullptr,
        contactData.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    ASSERT_NE(
        nullptr,
        contactData.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    ASSERT_TRUE(contactData.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        activeContactItem_->Value()));
}

TEST(ContactData, first_constructor_no_sections)
{
    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData.Version());
}

TEST(ContactData, first_constructor_different_versions)
{
    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData.Version());
}

TEST_F(Test_ContactData, second_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    opentxs::proto::ContactData data;
    data.set_version(CONTACT_CONTACT_DATA_VERSION);

    section1->SerializeTo(data, false);

    const opentxs::ContactData contactData(
        std::string("contactDataNym"), CONTACT_CONTACT_DATA_VERSION, data);

    ASSERT_EQ(data.version(), contactData.Version());
    ASSERT_NE(
        nullptr,
        contactData.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    ASSERT_NE(
        nullptr,
        contactData.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    ASSERT_TRUE(contactData.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        activeContactItem_->Value()));
}

TEST(ContactData, second_constructor_no_sections)
{
    opentxs::proto::ContactData data;
    data.set_version(CONTACT_CONTACT_DATA_VERSION);

    const opentxs::ContactData contactData(
        std::string("contactDataNym"), CONTACT_CONTACT_DATA_VERSION, data);
    ASSERT_EQ(data.version(), contactData.Version());
}

TEST_F(Test_ContactData, copy_constructor)
{
    const std::shared_ptr<opentxs::ContactSection> section1(
        new opentxs::ContactSection(
            "testContactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            activeContactItem_));

    const opentxs::ContactData::SectionMap map{{section1->Type(), section1}};

    const opentxs::ContactData contactData(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        map);

    const opentxs::ContactData copiedContactData(contactData);

    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, copiedContactData.Version());
    ASSERT_NE(
        nullptr,
        copiedContactData.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    ASSERT_NE(
        nullptr,
        copiedContactData.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    ASSERT_TRUE(copiedContactData.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, move_constructor)
{
    const opentxs::ContactData movedContactData(std::move<opentxs::ContactData>(
        contactData_.AddItem(activeContactItem_)));

    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, movedContactData.Version());
    ASSERT_NE(
        nullptr,
        movedContactData.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    ASSERT_NE(
        nullptr,
        movedContactData.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    ASSERT_TRUE(movedContactData.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
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
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE,
            std::string("contactItemValue2"),
            {opentxs::proto::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));

    const auto& group2 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup2",
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            contactItem2));

    const auto& section2 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym2",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::ContactSection::GroupMap{{contactItem2->Type(), group2}}));

    const opentxs::ContactData data2(
        std::string("contactDataNym2"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_IDENTIFIER, section2}});

    const auto& data3 = data1 + data2;
    // Verify the section exists.
    ASSERT_NE(
        nullptr, data3.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    // Verify it has one group.
    ASSERT_EQ(
        1, data3.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data3.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    // Verify it has two items.
    ASSERT_EQ(
        2,
        data3
            .Group(
                opentxs::proto::CONTACTSECTION_IDENTIFIER,
                opentxs::proto::CITEMTYPE_EMPLOYEE)
            ->Size());

    // Add a ContactData object with a section of a different type.
    const auto& contactItem4 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem4"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_ADDRESS,
            opentxs::proto::CITEMTYPE_PHYSICAL,
            std::string("contactItemValue4"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));

    const auto& group4 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup4",
            opentxs::proto::CONTACTSECTION_ADDRESS,
            contactItem4));

    const auto& section4 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym4",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_ADDRESS,
            opentxs::ContactSection::GroupMap{{contactItem4->Type(), group4}}));

    const opentxs::ContactData data4(
        std::string("contactDataNym4"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_ADDRESS, section4}});

    const auto& data5 = data3 + data4;
    // Verify the first section exists.
    ASSERT_NE(
        nullptr, data5.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    // Verify it has one group.
    ASSERT_EQ(
        1, data5.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data5.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    // Verify it has two items.
    ASSERT_EQ(
        2,
        data5
            .Group(
                opentxs::proto::CONTACTSECTION_IDENTIFIER,
                opentxs::proto::CITEMTYPE_EMPLOYEE)
            ->Size());

    // Verify the second section exists.
    ASSERT_NE(nullptr, data5.Section(opentxs::proto::CONTACTSECTION_ADDRESS));
    // Verify it has one group.
    ASSERT_EQ(1, data5.Section(opentxs::proto::CONTACTSECTION_ADDRESS)->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data5.Group(
            opentxs::proto::CONTACTSECTION_ADDRESS,
            opentxs::proto::CITEMTYPE_PHYSICAL));
    // Verify it has one item.
    ASSERT_EQ(
        1,
        data5
            .Group(
                opentxs::proto::CONTACTSECTION_ADDRESS,
                opentxs::proto::CITEMTYPE_PHYSICAL)
            ->Size());
}

TEST_F(Test_ContactData, operator_plus_different_version)
{
    // rhs version less than lhs
    const opentxs::ContactData contactData2(
        std::string("contactDataNym"),
        CONTACT_CONTACT_DATA_VERSION - 1,
        CONTACT_CONTACT_DATA_VERSION - 1,
        {});

    const auto& contactData3 = contactData_ + contactData2;
    // Verify the new contact data has the latest version.
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData3.Version());

    // lhs version less than rhs
    const auto& contactData4 = contactData2 + contactData_;
    // Verify the new contact data has the latest version.
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData4.Version());
}

TEST_F(Test_ContactData, operator_string)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const std::string dataString = data1;
    ASSERT_EQ(expectedStringOutput, dataString);
}

TEST_F(Test_ContactData, Serialize)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);

    // Serialize without ids.
    opentxs::proto::ContactData protoData = data1.Serialize(false);

    ASSERT_EQ(data1.Version(), protoData.version());
    ASSERT_EQ(1, protoData.section_size());
    opentxs::proto::ContactSection protoSection = protoData.section(0);
    ASSERT_EQ(opentxs::proto::CONTACTSECTION_IDENTIFIER, protoSection.name());
    ASSERT_EQ(
        data1.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size(),
        protoSection.item_size());
    opentxs::proto::ContactItem protoItem = protoSection.item(0);
    ASSERT_EQ(activeContactItem_->Value(), protoItem.value());
    ASSERT_EQ(activeContactItem_->Version(), protoItem.version());
    ASSERT_EQ(activeContactItem_->Type(), protoItem.type());
    ASSERT_EQ(activeContactItem_->Start(), protoItem.start());
    ASSERT_EQ(activeContactItem_->End(), protoItem.end());

    // Serialize with ids.
    protoData = data1.Serialize(true);

    ASSERT_EQ(data1.Version(), protoData.version());
    ASSERT_EQ(1, protoData.section_size());
    protoSection = protoData.section(0);
    ASSERT_EQ(opentxs::proto::CONTACTSECTION_IDENTIFIER, protoSection.name());
    ASSERT_EQ(
        data1.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size(),
        protoSection.item_size());
    protoItem = protoSection.item(0);

    ASSERT_EQ(activeContactItem_->ID().str(), protoItem.id());
    ASSERT_EQ(activeContactItem_->Value(), protoItem.value());
    ASSERT_EQ(activeContactItem_->Version(), protoItem.version());
    ASSERT_EQ(activeContactItem_->Type(), protoItem.type());
    ASSERT_EQ(activeContactItem_->Start(), protoItem.start());
    ASSERT_EQ(activeContactItem_->End(), protoItem.end());
}

TEST_F(Test_ContactData, AddContract)
{
    testAddItemMethod(
        std::mem_fn<opentxs::ContactData(
            const std::string&,
            const opentxs::proto::ContactItemType,
            const bool,
            const bool) const>(&opentxs::ContactData::AddContract),
        opentxs::proto::CONTACTSECTION_CONTRACT);
}

TEST_F(Test_ContactData, AddContract_different_versions)
{
    testAddItemMethod(
        std::mem_fn<opentxs::ContactData(
            const std::string&,
            const opentxs::proto::ContactItemType,
            const bool,
            const bool) const>(&opentxs::ContactData::AddContract),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        4);
}

TEST_F(Test_ContactData, AddItem_claim)
{
    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        std::string("contactItemValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});
    const auto& data1 = contactData_.AddItem(claim);
    // Verify the section was added.
    ASSERT_NE(nullptr, data1.Section(opentxs::proto::CONTACTSECTION_CONTRACT));
    // Verify the group was added.
    ASSERT_NE(
        nullptr,
        data1.Group(
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_USD));
    ASSERT_TRUE(data1.HaveClaim(
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        "contactItemValue"));
}

TEST_F(Test_ContactData, AddItem_claim_different_versions)
{
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_BCH,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            3,  // version of CONTACTSECTION_CONTRACT section before
                // CITEMTYPE_BCH was added
            3,
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_BCH, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        3,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_CONTRACT, section1}});

    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_BCH,
        std::string("contactItemValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});

    const auto& data2 = data1.AddItem(claim);

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, AddItem_item)
{
    // Add an item to a ContactData with no section.
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    // Verify the section was added.
    ASSERT_NE(
        nullptr, data1.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
    // Verify the group was added.
    ASSERT_NE(
        nullptr,
        data1.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
    // Verify the item was added.
    ASSERT_TRUE(data1.HaveClaim(
        activeContactItem_->Section(),
        activeContactItem_->Type(),
        activeContactItem_->Value()));

    // Add an item to a ContactData with a section.
    const auto& contactItem2 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE,
            std::string("contactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& data2 = data1.AddItem(contactItem2);
    // Verify the item was added.
    ASSERT_TRUE(data2.HaveClaim(
        contactItem2->Section(), contactItem2->Type(), contactItem2->Value()));
    // Verify the group has two items.
    ASSERT_EQ(
        2,
        data2
            .Group(
                opentxs::proto::CONTACTSECTION_IDENTIFIER,
                opentxs::proto::CITEMTYPE_EMPLOYEE)
            ->Size());
}

TEST_F(Test_ContactData, AddItem_item_different_versions)
{
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_BCH,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            3,  // version of CONTACTSECTION_CONTRACT section before
                // CITEMTYPE_BCH was added
            3,
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_BCH, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        3,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_CONTRACT, section1}});

    const auto& contactItem1 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem1"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_BCH,
            std::string("contactItemValue1"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));

    const auto& data2 = data1.AddItem(contactItem1);

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, AddPaymentCode)
{
    testAddItemMethod(
        std::mem_fn<opentxs::ContactData(
            const std::string&,
            const opentxs::proto::ContactItemType,
            const bool,
            const bool) const>(&opentxs::ContactData::AddPaymentCode),
        opentxs::proto::CONTACTSECTION_PROCEDURE);
}

TEST_F(Test_ContactData, AddPaymentCode_different_versions)
{
    testAddItemMethod(
        std::mem_fn<opentxs::ContactData(
            const std::string&,
            const opentxs::proto::ContactItemType,
            const bool,
            const bool) const>(&opentxs::ContactData::AddContract),
        opentxs::proto::CONTACTSECTION_PROCEDURE,
        3,  // version of CONTACTSECTION_PROCEDURE section before CITEMTYPE_BCH
            // was added
        4);
}

TEST_F(Test_ContactData, AddPreferredOTServer)
{
    // Add a server to a group with no primary.
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_OPENTXS, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_COMMUNICATION, section1}});

    const opentxs::Identifier serverIdentifier1(
        opentxs::ContactCredential::ClaimID(
            "contactDataNym1",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            std::string("serverID1")));
    const auto& data2 = data1.AddPreferredOTServer(serverIdentifier1, false);

    // Verify that the item was made primary.
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        opentxs::String(serverIdentifier1).Get()));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a server to a group with a primary.
    const opentxs::Identifier serverIdentifier2(
        opentxs::ContactCredential::ClaimID(
            "contactDataNym1",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            std::string("serverID2")));
    const auto& data3 = data2.AddPreferredOTServer(serverIdentifier2, false);

    // Verify that the item wasn't made primary.
    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "contactDataNym1",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        opentxs::String(serverIdentifier2).Get()));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a server to a ContactData with no group.
    const opentxs::Identifier serverIdentifier3(
        opentxs::ContactCredential::ClaimID(
            "contactDataNym",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            std::string("serverID3")));
    const auto& data4 =
        contactData_.AddPreferredOTServer(serverIdentifier3, false);

    // Verify the group was created.
    ASSERT_NE(
        nullptr,
        data4.Group(
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS));
    // Verify that the item was made primary.
    const opentxs::Identifier identifier3(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        opentxs::String(serverIdentifier3).Get()));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add a primary server.
    const opentxs::Identifier serverIdentifier4(
        opentxs::ContactCredential::ClaimID(
            "contactDataNym",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            std::string("serverID4")));
    const auto& data5 = data4.AddPreferredOTServer(serverIdentifier4, true);

    // Verify that the item was made primary.
    const opentxs::Identifier identifier4(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_COMMUNICATION,
        opentxs::proto::CITEMTYPE_OPENTXS,
        NULL_START,
        NULL_END,
        opentxs::String(serverIdentifier4).Get()));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isPrimary());
    // Verify the previous preferred server is no longer primary.
    const auto& contactItem5 = data5.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem5);
    ASSERT_FALSE(contactItem5->isPrimary());
}

TEST_F(Test_ContactData, AddPreferredOTServer_different_versions)
{
    // Nothing to test for required version of contact section for OTServer
    // because all contact item types have been available for all versions of
    // CONTACTSECTION_COMMUNICATION section.
}

TEST_F(Test_ContactData, Claim_found)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(nullptr, data1.Claim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, Claim_not_found)
{
    ASSERT_FALSE(contactData_.Claim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, Contracts)
{
    const auto& data1 = contactData_.AddContract(
        "instrumentDefinitionID1", opentxs::proto::CITEMTYPE_USD, false, false);
    const std::set<opentxs::Identifier>& contracts =
        data1.Contracts(opentxs::proto::CITEMTYPE_USD, false);
    ASSERT_EQ(1, contracts.size());
}

TEST_F(Test_ContactData, Contracts_onlyactive_found)
{
    const auto& data1 = contactData_.AddContract(
        "instrumentDefinitionID1", opentxs::proto::CITEMTYPE_USD, false, true);
    const auto& data2 = data1.AddContract(
        "instrumentDefinitionID2", opentxs::proto::CITEMTYPE_USD, false, false);
    const std::set<opentxs::Identifier>& contracts =
        data2.Contracts(opentxs::proto::CITEMTYPE_USD, true);
    ASSERT_EQ(1, contracts.size());
}

TEST_F(Test_ContactData, Contracts_onlyactive_not_found)
{
    const auto& data1 = contactData_.AddContract(
        "instrumentDefinitionID1", opentxs::proto::CITEMTYPE_USD, false, false);
    const std::set<opentxs::Identifier>& contracts =
        data1.Contracts(opentxs::proto::CITEMTYPE_USD, true);
    ASSERT_EQ(0, contracts.size());
}

TEST_F(Test_ContactData, Delete)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const auto& contactItem2 =
        std::shared_ptr<opentxs::ContactItem>(new opentxs::ContactItem(
            std::string("contactItem2"),
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE,
            std::string("contactItemValue2"),
            {opentxs::proto::ContactItemAttribute::CITEMATTR_ACTIVE},
            NULL_START,
            NULL_END));
    const auto& data2 = data1.AddItem(contactItem2);

    const auto& data3 = data2.Delete(activeContactItem_->ID());
    // Verify the item was deleted.
    ASSERT_EQ(
        1, data3.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size());
    ASSERT_FALSE(data3.Claim(activeContactItem_->ID()));

    const auto& data4 = data3.Delete(activeContactItem_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(
        1, data4.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER)->Size());

    const auto& data5 = data4.Delete(contactItem2->ID());
    // Verify the section was removed.
    ASSERT_FALSE(data5.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
}

TEST_F(Test_ContactData, Group_found)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(
        nullptr,
        data1.Group(
            opentxs::proto::CONTACTSECTION_IDENTIFIER,
            opentxs::proto::CITEMTYPE_EMPLOYEE));
}

TEST_F(Test_ContactData, Group_notfound)
{
    ASSERT_FALSE(contactData_.Group(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE));
}

TEST_F(Test_ContactData, HaveClaim_1)
{
    ASSERT_FALSE(contactData_.HaveClaim(activeContactItem_->ID()));

    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_TRUE(data1.HaveClaim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, HaveClaim_2)
{
    // Test for an item in group that doesn't exist.
    ASSERT_FALSE(contactData_.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        "activeContactItemValue"));

    // Test for an item that does exist.
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_TRUE(data1.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        "activeContactItemValue"));

    // Test for an item that doesn't exist in a group that does.
    ASSERT_FALSE(data1.HaveClaim(
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_EMPLOYEE,
        "dummyContactItemValue"));
}

TEST_F(Test_ContactData, Name)
{
    // Verify that Name returns an empty string if there is no scope group.
    ASSERT_STREQ("", contactData_.Name().c_str());

    // Test when the scope group is emtpy.
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::CONTACTSECTION_SCOPE,
            opentxs::proto::CITEMTYPE_INDIVIDUAL,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_SCOPE,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_INDIVIDUAL, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_SCOPE, section1}});
    // Verify that Name returns an empty string.
    ASSERT_STREQ("", data1.Name().c_str());

    // Test when the scope is set.
    const auto& data2 = contactData_.SetScope(
        opentxs::proto::CITEMTYPE_INDIVIDUAL, "activeContactItemValue");
    ASSERT_STREQ("activeContactItemValue", data2.Name().c_str());
}

TEST_F(Test_ContactData, PreferredOTServer)
{
    // Test getting the preferred server with no group.
    const auto& identifier = contactData_.PreferredOTServer();
    ASSERT_TRUE(identifier->empty());

    // Test getting the preferred server with an empty group.
    const auto& group1 =
        std::shared_ptr<opentxs::ContactGroup>(new opentxs::ContactGroup(
            "contactGroup1",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            {}));

    const auto& section1 =
        std::shared_ptr<opentxs::ContactSection>(new opentxs::ContactSection(
            "contactSectionNym1",
            CONTACT_CONTACT_DATA_VERSION,
            CONTACT_CONTACT_DATA_VERSION,
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::ContactSection::GroupMap{
                {opentxs::proto::CITEMTYPE_OPENTXS, group1}}));

    const opentxs::ContactData data1(
        std::string("contactDataNym1"),
        CONTACT_CONTACT_DATA_VERSION,
        CONTACT_CONTACT_DATA_VERSION,
        opentxs::ContactData::SectionMap{
            {opentxs::proto::CONTACTSECTION_COMMUNICATION, section1}});

    const auto& identifier2 = data1.PreferredOTServer();
    ASSERT_TRUE(identifier2->empty());

    // Test getting the preferred server.
    const opentxs::Identifier serverIdentifier2(
        opentxs::ContactCredential::ClaimID(
            "contactDataNym",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            std::string("serverID2")));
    const auto& data2 =
        contactData_.AddPreferredOTServer(serverIdentifier2, true);
    const auto& preferredServer = data2.PreferredOTServer();
    ASSERT_FALSE(preferredServer->empty());
    ASSERT_EQ(serverIdentifier2, preferredServer);
}

TEST_F(Test_ContactData, Section)
{
    ASSERT_FALSE(
        contactData_.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));

    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(
        nullptr, data1.Section(opentxs::proto::CONTACTSECTION_IDENTIFIER));
}

TEST_F(Test_ContactData, SetCommonName)
{
    const auto& data1 = contactData_.SetCommonName("commonName");
    const opentxs::Identifier identifier(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_IDENTIFIER,
        opentxs::proto::CITEMTYPE_COMMONNAME,
        NULL_START,
        NULL_END,
        std::string("commonName")));
    const auto& commonNameItem = data1.Claim(identifier);
    ASSERT_NE(nullptr, commonNameItem);
    ASSERT_TRUE(commonNameItem->isPrimary());
    ASSERT_TRUE(commonNameItem->isActive());
}

TEST_F(Test_ContactData, SetName)
{
    const auto& data1 = contactData_.SetScope(
        opentxs::proto::CITEMTYPE_INDIVIDUAL, "firstName");

    // Test that SetName creates a scope item.
    const auto& data2 = data1.SetName("secondName");
    // Verify the item was created in the scope section and made primary.
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_SCOPE,
        opentxs::proto::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        std::string("secondName")));
    const auto& scopeItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem1);
    ASSERT_TRUE(scopeItem1->isPrimary());
    ASSERT_TRUE(scopeItem1->isActive());

    // Test that SetName creates an item in the scope section without making it
    // primary.
    const auto& data3 = data2.SetName("thirdName", false);
    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_SCOPE,
        opentxs::proto::CITEMTYPE_INDIVIDUAL,
        NULL_START,
        NULL_END,
        std::string("thirdName")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());
    ASSERT_TRUE(contactItem2->isActive());
}

TEST_F(Test_ContactData, SetScope)
{
    const auto& data1 = contactData_.SetScope(
        opentxs::proto::CITEMTYPE_ORGANIZATION, "organizationScope");
    // Verify the scope item was created.
    const opentxs::Identifier identifier1(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_SCOPE,
        opentxs::proto::CITEMTYPE_ORGANIZATION,
        NULL_START,
        NULL_END,
        std::string("organizationScope")));
    const auto& scopeItem1 = data1.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem1);
    ASSERT_TRUE(scopeItem1->isPrimary());
    ASSERT_TRUE(scopeItem1->isActive());

    // Test when there is an existing scope.
    const auto& data2 =
        data1.SetScope(opentxs::proto::CITEMTYPE_ORGANIZATION, "businessScope");
    // Verify the item wasn't added.
    const opentxs::Identifier identifier2(opentxs::ContactCredential::ClaimID(
        "contactDataNym",
        opentxs::proto::CONTACTSECTION_SCOPE,
        opentxs::proto::CITEMTYPE_BUSINESS,
        NULL_START,
        NULL_END,
        std::string("businessScope")));
    ASSERT_FALSE(data2.Claim(identifier2));
    // Verify the scope wasn't changed.
    const auto& scopeItem2 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem2);
    ASSERT_TRUE(scopeItem2->isPrimary());
    ASSERT_TRUE(scopeItem2->isActive());
}

TEST_F(Test_ContactData, SetScope_different_versions)
{
    const opentxs::ContactData data1(
        std::string("dataNym1"),
        3,  // version of CONTACTSECTION_SCOPE section before CITEMTYPE_BOT
            // was added
        3,
        {});

    const auto& data2 =
        data1.SetScope(opentxs::proto::CITEMTYPE_BOT, "botScope");

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, Type)
{
    ASSERT_EQ(opentxs::proto::CITEMTYPE_UNKNOWN, contactData_.Type());

    const auto& data1 = contactData_.SetScope(
        opentxs::proto::CITEMTYPE_INDIVIDUAL, "scopeName");
    ASSERT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, data1.Type());
}

TEST_F(Test_ContactData, Version)
{
    ASSERT_EQ(CONTACT_CONTACT_DATA_VERSION, contactData_.Version());
}
