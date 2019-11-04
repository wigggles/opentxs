// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

namespace
{
class Test_NymData : public ::testing::Test
{
public:
    const opentxs::api::client::internal::Manager& client_;
    opentxs::OTPasswordPrompt reason_;
    opentxs::NymData nymData_;

    static std::string ExpectedStringOutput(const std::uint32_t version)
    {
        return std::string{"Version "} + std::to_string(version) +
               std::string(" contact data\nSections found: 1\n- Section: "
                           "Scope, version: ") +
               std::to_string(version) +
               std::string{" containing 1 item(s).\n-- Item type: "
                           "\"Individual\", value: "
                           "\"testNym\", start: 0, end: 0, version: "} +
               std::to_string(version) +
               std::string{"\n--- Attributes: Active Primary \n"};
    }

    Test_NymData()
        : client_(dynamic_cast<const opentxs::api::client::internal::Manager&>(
              opentxs::Context().StartClient({}, 0)))
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
        , nymData_(client_.Wallet().mutable_Nym(
              client_.Wallet().Nym(reason_, "testNym")->ID(),
              reason_))
    {
    }
};

static const std::string paymentCode{
    "PM8TJKxypQfFUaHfSq59nn82EjdGU4SpHcp2ssa4GxPshtzoFtmnjfoRuHpvLiyASD7itH6auP"
    "C66jekGjnqToqS9ZJWWdf1c9L8x4iaFCQ2Gq5hMEFC"};
}  // namespace

TEST_F(Test_NymData, AddClaim)
{
    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        std::string("claimValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});

    auto added = nymData_.AddClaim(claim, reason_);
    EXPECT_TRUE(added);
}

TEST_F(Test_NymData, AddContract)
{
    auto added = nymData_.AddContract(
        "", opentxs::proto::CITEMTYPE_USD, false, false, reason_);
    EXPECT_FALSE(added);

    const auto identifier1(opentxs::identifier::UnitDefinition::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_USD,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID1",
            "")));

    added = nymData_.AddContract(
        identifier1->str(),
        opentxs::proto::CITEMTYPE_USD,
        false,
        false,
        reason_);
    EXPECT_TRUE(added);
}

TEST_F(Test_NymData, AddEmail)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, asPublicNym)
{
    auto credentialIndex = nymData_.asPublicNym();

    EXPECT_TRUE(credentialIndex.IsInitialized());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_NymData, AddPaymentCode)
{
    auto added = nymData_.AddPaymentCode(
        "", opentxs::proto::CITEMTYPE_USD, false, false, reason_);
    EXPECT_FALSE(added);

    added = nymData_.AddPaymentCode(
        paymentCode, opentxs::proto::CITEMTYPE_USD, false, false, reason_);
    EXPECT_TRUE(added);
}
#endif

TEST_F(Test_NymData, AddPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPhoneNumber("", false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, AddPreferredOTServer)
{
    const auto identifier(opentxs::identifier::Server::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            "localhost",
            "")));

    auto added =
        nymData_.AddPreferredOTServer(identifier->str(), false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPreferredOTServer("", false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, AddSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_TWITTER, false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "", opentxs::proto::CITEMTYPE_TWITTER, false, false, reason_);
    EXPECT_FALSE(added);
}

TEST_F(Test_NymData, BestEmail)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email2", false, true, reason_);
    EXPECT_TRUE(added);

    std::string email = nymData_.BestEmail();
    // First email added is made primary.
    EXPECT_STREQ("email1", email.c_str());
}

TEST_F(Test_NymData, BestPhoneNumber)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, true, reason_);
    EXPECT_TRUE(added);

    std::string phone = nymData_.BestPhoneNumber();
    // First phone number added is made primary.
    EXPECT_STREQ("phone1", phone.c_str());
}

TEST_F(Test_NymData, BestSocialMediaProfile)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_YAHOO, false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2", opentxs::proto::CITEMTYPE_YAHOO, false, true, reason_);
    EXPECT_TRUE(added);

    std::string profile =
        nymData_.BestSocialMediaProfile(opentxs::proto::CITEMTYPE_YAHOO);
    // First profile added is made primary.
    EXPECT_STREQ("profile1", profile.c_str());
}

TEST_F(Test_NymData, Claims)
{
    auto contactData = nymData_.Claims();
    const auto expected =
        ExpectedStringOutput(nymData_.Nym().ContactDataVersion());

    std::string output = contactData;
    EXPECT_TRUE(!output.empty());
    EXPECT_STREQ(expected.c_str(), output.c_str());
}

TEST_F(Test_NymData, DeleteClaim)
{
    opentxs::Claim claim = std::make_tuple(
        std::string(""),
        opentxs::proto::CONTACTSECTION_CONTRACT,
        opentxs::proto::CITEMTYPE_USD,
        std::string("claimValue"),
        NULL_START,
        NULL_END,
        std::set<std::uint32_t>{opentxs::proto::CITEMATTR_ACTIVE});

    auto added = nymData_.AddClaim(claim, reason_);
    ASSERT_TRUE(added);

    const auto identifier(opentxs::identifier::UnitDefinition::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_USD,
            NULL_START,
            NULL_END,
            "claimValue",
            "")));
    auto deleted = nymData_.DeleteClaim(identifier, reason_);
    EXPECT_TRUE(deleted);
}

TEST_F(Test_NymData, EmailAddresses)
{
    auto added = nymData_.AddEmail("email1", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email2", false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddEmail("email3", true, false, reason_);
    EXPECT_TRUE(added);

    auto emails = nymData_.EmailAddresses(false);
    EXPECT_TRUE(
        emails.find("email1") != std::string::npos &&
        emails.find("email2") != std::string::npos &&
        emails.find("email3") != std::string::npos);

    emails = nymData_.EmailAddresses();
    // First email added is made primary and active.
    EXPECT_TRUE(
        emails.find("email1") != std::string::npos &&
        emails.find("email3") != std::string::npos);
    EXPECT_TRUE(emails.find("email2") == std::string::npos);
}

TEST_F(Test_NymData, HaveContract)
{
    const auto identifier1(opentxs::identifier::UnitDefinition::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_USD,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID1",
            "")));

    auto added = nymData_.AddContract(
        identifier1->str(),
        opentxs::proto::CITEMTYPE_USD,
        false,
        false,
        reason_);
    ASSERT_TRUE(added);

    auto haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, true, true);
    EXPECT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, true, false);
    EXPECT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, false, true);
    EXPECT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier1, opentxs::proto::CITEMTYPE_USD, false, false);
    EXPECT_TRUE(haveContract);

    const auto identifier2(opentxs::identifier::UnitDefinition::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_CONTRACT,
            opentxs::proto::CITEMTYPE_USD,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID2",
            "")));

    added = nymData_.AddContract(
        identifier2->str(),
        opentxs::proto::CITEMTYPE_USD,
        false,
        false,
        reason_);
    ASSERT_TRUE(added);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, false, false);
    EXPECT_TRUE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, true, false);
    EXPECT_FALSE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, false, true);
    EXPECT_FALSE(haveContract);

    haveContract = nymData_.HaveContract(
        identifier2, opentxs::proto::CITEMTYPE_USD, true, true);
    EXPECT_FALSE(haveContract);
}

TEST_F(Test_NymData, Name) { EXPECT_STREQ("testNym", nymData_.Name().c_str()); }

TEST_F(Test_NymData, Nym)
{
    EXPECT_STREQ("testNym", nymData_.Nym().Name().c_str());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
TEST_F(Test_NymData, PaymentCode)
{
    auto added = nymData_.AddPaymentCode(
        paymentCode, opentxs::proto::CITEMTYPE_BTC, true, true, reason_);
    ASSERT_TRUE(added);

    auto paymentcode = nymData_.PaymentCode(opentxs::proto::CITEMTYPE_BTC);
    EXPECT_TRUE(!paymentcode.empty());
    EXPECT_STREQ(paymentCode.c_str(), paymentcode.c_str());

    paymentcode = nymData_.PaymentCode(opentxs::proto::CITEMTYPE_USD);
    EXPECT_TRUE(paymentcode.empty());
}
#endif

TEST_F(Test_NymData, PhoneNumbers)
{
    auto added = nymData_.AddPhoneNumber("phone1", false, false, reason_);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone2", false, false, reason_);
    ASSERT_TRUE(added);

    added = nymData_.AddPhoneNumber("phone3", true, false, reason_);
    ASSERT_TRUE(added);

    auto phones = nymData_.PhoneNumbers(false);
    EXPECT_TRUE(
        phones.find("phone1") != std::string::npos &&
        phones.find("phone2") != std::string::npos &&
        phones.find("phone3") != std::string::npos);

    phones = nymData_.PhoneNumbers();
    // First phone number added is made primary and active.
    EXPECT_TRUE(
        phones.find("phone1") != std::string::npos &&
        phones.find("phone3") != std::string::npos);
    EXPECT_TRUE(phones.find("phone2") == std::string::npos);
}

TEST_F(Test_NymData, PreferredOTServer)
{
    auto preferred = nymData_.PreferredOTServer();
    EXPECT_TRUE(preferred.empty());

    const auto identifier(opentxs::identifier::Server::Factory(
        opentxs::identity::credential::Contact::ClaimID(
            client_,
            "testNym",
            opentxs::proto::CONTACTSECTION_COMMUNICATION,
            opentxs::proto::CITEMTYPE_OPENTXS,
            NULL_START,
            NULL_END,
            "localhost",
            "")));
    auto added =
        nymData_.AddPreferredOTServer(identifier->str(), true, reason_);
    EXPECT_TRUE(added);

    preferred = nymData_.PreferredOTServer();
    EXPECT_TRUE(!preferred.empty());
    EXPECT_STREQ(identifier->str().c_str(), preferred.c_str());
}

TEST_F(Test_NymData, PrintContactData)
{
    const auto& text = nymData_.PrintContactData();
    const auto expected =
        ExpectedStringOutput(nymData_.Nym().ContactDataVersion());

    EXPECT_STREQ(expected.c_str(), text.c_str());
}

TEST_F(Test_NymData, SetContactData)
{
    const opentxs::ContactData contactData(
        client_,
        std::string("contactData"),
        nymData_.Nym().ContactDataVersion(),
        nymData_.Nym().ContactDataVersion(),
        {});

    auto data = contactData.Serialize(true);
    auto set = nymData_.SetContactData(data, reason_);
    EXPECT_TRUE(set);
}

TEST_F(Test_NymData, SetScope)
{
    auto set = nymData_.SetScope(
        opentxs::proto::CITEMTYPE_ORGANIZATION,
        "organizationScope",
        true,
        reason_);
    EXPECT_TRUE(set);

    set = nymData_.SetScope(
        opentxs::proto::CITEMTYPE_BUSINESS, "businessScope", false, reason_);
    EXPECT_TRUE(set);
}

TEST_F(Test_NymData, SocialMediaProfiles)
{
    auto added = nymData_.AddSocialMediaProfile(
        "profile1", opentxs::proto::CITEMTYPE_FACEBOOK, false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile2", opentxs::proto::CITEMTYPE_FACEBOOK, false, false, reason_);
    EXPECT_TRUE(added);

    added = nymData_.AddSocialMediaProfile(
        "profile3", opentxs::proto::CITEMTYPE_FACEBOOK, true, false, reason_);
    EXPECT_TRUE(added);

    auto profiles =
        nymData_.SocialMediaProfiles(opentxs::proto::CITEMTYPE_FACEBOOK, false);
    EXPECT_TRUE(
        profiles.find("profile1") != std::string::npos &&
        profiles.find("profile2") != std::string::npos &&
        profiles.find("profile3") != std::string::npos);

    profiles = nymData_.SocialMediaProfiles(opentxs::proto::CITEMTYPE_FACEBOOK);
    // First profile added is made primary and active.
    EXPECT_TRUE(
        profiles.find("profile1") != std::string::npos &&
        profiles.find("profile3") != std::string::npos);
    EXPECT_TRUE(profiles.find("profile2") == std::string::npos);
}

TEST_F(Test_NymData, SocialMediaProfileTypes)
{
    std::set<opentxs::proto::ContactItemType> profileTypes =
        opentxs::proto::AllowedItemTypes().at(
            opentxs::proto::ContactSectionVersion(
                CONTACT_CONTACT_DATA_VERSION,
                opentxs::proto::CONTACTSECTION_PROFILE));

    EXPECT_EQ(profileTypes, nymData_.SocialMediaProfileTypes());
}

TEST_F(Test_NymData, Type)
{
    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, nymData_.Type());
}

TEST_F(Test_NymData, Valid) { EXPECT_TRUE(nymData_.Valid()); }
