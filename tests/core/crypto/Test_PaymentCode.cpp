// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
using namespace opentxs;

namespace
{

class Test_PaymentCode : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;
    OTPasswordPrompt reason_;
    std::string seed, fingerprint, nymID_0, paycode_0, nymID_1, paycode_1,
        nymID_2, paycode_2, nymID_3, paycode_3;
    opentxs::NymData nymData_0, nymData_1, nymData_2, nymData_3;
    proto::ContactItemType currency = opentxs::proto::CITEMTYPE_BCH;
    proto::ContactItemType currency_2 = opentxs::proto::CITEMTYPE_BTC;

    /* Is evaluated every test, therefore indexes are fixed to 0,1,2,3 */
    Test_PaymentCode()
        : client_(opentxs::Context().StartClient({}, 0))
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
        , seed("trim thunder unveil reduce crop cradle zone inquiry anchor "
               "skate property fringe obey butter text tank drama palm guilt "
               "pudding laundry stay axis prosper")
        , fingerprint(client_.Exec().Wallet_ImportSeed(seed, ""))
        , nymID_0(client_.Wallet()
                      .Nym(reason_, "PaycodeNym", {fingerprint, 0})
                      ->ID()
                      .str())
        , paycode_0(
              "PM8TJhB2CxWDqR8c5y4kWoJwSGRNYaVATdJM85kqfn2dZ9TdSihbFJraQzjYUMYx"
              "bsrnMfjPK6oZFAPQ1tWqzwTfKbtunvLFCzDJFVXVGbUAKxhsz7P5")
        , nymID_1(client_.Wallet()
                      .Nym(reason_, "PaycodeNym_1", {fingerprint, 1})
                      ->ID()
                      .str())
        , paycode_1(
              "PM8TJWedQTvxaoJpt9Wh25HR54oj5vmor6arAByFk4UTgUh1Tna2srsZLUo2xS3V"
              "iBot1ftf4p8ZUN8khB2zvViHXZkrwkfjcePSeEgsYapESKywge9F")
        , nymID_2(client_.Wallet()
                      .Nym(reason_, "PaycodeNym_2", {fingerprint, 2})
                      ->ID()
                      .str())
        , paycode_2(
              "PM8TJQmrQ4tSY6Gad59UpzqR8MRMesSYMKXvpMuzdDHByfRXVgvVdiqD5NmjoEH9"
              "V6ZrofFVViBwSg9dvVcP8R2CU1pXejhVQQj3XsWk8sLhAsspqk8F")
        , nymID_3(client_.Wallet()
                      .Nym(reason_, "PaycodeNym_3", {fingerprint, 3})
                      ->ID()
                      .str())
        , paycode_3(
              "PM8TJbNzqDcdqCcpkMLLa9H83CjoWdHMTQ4Lk11qSpThkyrmDFA4AeGd2kFeLK2s"
              "T6UVXy2jwWABsfLd7JmcS4hMAy9zUdWRFRhmu33RiRJCS6qRmGew")
        , nymData_0(client_.Wallet().mutable_Nym(
              identifier::Nym::Factory(nymID_0),
              reason_))
        , nymData_1(client_.Wallet().mutable_Nym(
              identifier::Nym::Factory(nymID_1),
              reason_))
        , nymData_2(client_.Wallet().mutable_Nym(
              identifier::Nym::Factory(nymID_2),
              reason_))
        , nymData_3(client_.Wallet().mutable_Nym(
              identifier::Nym::Factory(nymID_3),
              reason_))
    {
        nymData_0.AddPaymentCode(paycode_0, currency, true, true, reason_);

        nymData_1.AddPaymentCode(paycode_0, currency, true, true, reason_);
        nymData_1.AddPaymentCode(
            paycode_1, currency, true, false, reason_);  // reset nymdata_1 to
                                                         // paymentcode_1

        nymData_2.AddPaymentCode(
            paycode_2, currency_2, false, true, reason_);  // nymdata_2 resets
                                                           // paycode_2 to
                                                           // primary
        nymData_2.AddPaymentCode(
            paycode_0, currency_2, false, true, reason_);  // fail to reset
                                                           // nymdata_2 with
                                                           // primary = false

        nymData_3.AddPaymentCode(
            paycode_3, currency_2, false, false, reason_);  // nymdata_3 resets
                                                            // paycode_3 to
                                                            // primary
        nymData_3.AddPaymentCode(
            paycode_0, currency_2, false, false, reason_);  // fail to be
                                                            // primary
    }
};

/* Test: Gets the last paymentcode to be set as primary
 */
TEST_F(Test_PaymentCode, primary_paycodes)
{
    ASSERT_STREQ(
        paycode_0.c_str(),
        nymData_0.PaymentCode(currency).c_str());  // primary and active
    ASSERT_STREQ(
        paycode_1.c_str(),
        nymData_1.PaymentCode(currency).c_str());  // primary but inactive
                                                   // overrides primary active
    ASSERT_STREQ(
        paycode_2.c_str(),
        nymData_2.PaymentCode(currency_2).c_str());  // not primary but active
                                                     // defaults to primary
    ASSERT_STREQ(
        paycode_3.c_str(),
        nymData_3.PaymentCode(currency_2).c_str());  // not primary nor active
                                                     // defaults to primary

    auto nym0 =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_0), reason_);
    auto nym1 =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_1), reason_);
    auto nym2 =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_2), reason_);
    auto nym3 =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_3), reason_);

    ASSERT_STREQ(nym0->PaymentCode(reason_).c_str(), paycode_0.c_str());
    ASSERT_STREQ(nym1->PaymentCode(reason_).c_str(), paycode_1.c_str());
    ASSERT_STREQ(nym2->PaymentCode(reason_).c_str(), paycode_2.c_str());
    ASSERT_STREQ(nym3->PaymentCode(reason_).c_str(), paycode_3.c_str());
}

/* Test: by setting primary = true it resets best payment code
 */
TEST_F(Test_PaymentCode, test_new_primary)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());

    ASSERT_TRUE(
        nymData_0.AddPaymentCode(paycode_2, currency, true, true, reason_));
    ASSERT_STREQ(paycode_2.c_str(), nymData_0.PaymentCode(currency).c_str());

    ASSERT_TRUE(
        nymData_0.AddPaymentCode(paycode_3, currency, true, false, reason_));
    ASSERT_STREQ(paycode_3.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: by setting primary = false it should not override a previous primary
 */
TEST_F(Test_PaymentCode, test_secondary_doesnt_replace)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());

    ASSERT_TRUE(
        nymData_0.AddPaymentCode(paycode_2, currency, false, false, reason_));
    ASSERT_TRUE(
        nymData_0.AddPaymentCode(paycode_3, currency, false, true, reason_));

    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: Valid paycodes
 */
TEST_F(Test_PaymentCode, valid_paycodes)
{
    ASSERT_TRUE(
        client_.Factory().PaymentCode(paycode_0, reason_)->VerifyInternally());
    ASSERT_TRUE(
        client_.Factory().PaymentCode(paycode_1, reason_)->VerifyInternally());
    ASSERT_TRUE(
        client_.Factory().PaymentCode(paycode_2, reason_)->VerifyInternally());
    ASSERT_TRUE(
        client_.Factory().PaymentCode(paycode_3, reason_)->VerifyInternally());
}

/* Test: Invalid paycodes should not be saved
 */
TEST_F(Test_PaymentCode, empty_paycode)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());

    ASSERT_FALSE(
        client_.Factory().PaymentCode("", reason_)->VerifyInternally());
    bool added = nymData_0.AddPaymentCode("", currency, true, true, reason_);
    ASSERT_FALSE(added);

    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());

    std::string invalid_paycode =
        "XM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjB"
        "xteQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97";
    ASSERT_FALSE(client_.Factory()
                     .PaymentCode(invalid_paycode, reason_)
                     ->VerifyInternally());

    added = nymData_0.AddPaymentCode(
        invalid_paycode, currency, true, true, reason_);
    ASSERT_FALSE(added);
    ASSERT_STRNE("", nymData_0.PaymentCode(currency).c_str());

    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: equals operator
 */
TEST_F(Test_PaymentCode, paycode_equals)
{
    OTPaymentCode payment_code =
        client_.Factory().PaymentCode(paycode_0, reason_);
    PaymentCode& rep1 = payment_code.get();
    auto serializedPaymentcode = payment_code->Serialize();
    proto::PaymentCode& rep2 = *serializedPaymentcode;
    ASSERT_TRUE(rep1 == rep2);
}

/* Test: Base58 encoding
 */
TEST_F(Test_PaymentCode, asBase58)
{
    auto pcode = client_.Factory().PaymentCode(paycode_0, reason_);
    ASSERT_STREQ(paycode_0.c_str(), pcode->asBase58().c_str());
}

/* Test: Factory methods create the same paycode
 */
TEST_F(Test_PaymentCode, factory)
{
    // Factory 0: PaymentCode&
    auto factory_0 = client_.Factory().PaymentCode(paycode_0, reason_);
    ASSERT_STREQ(paycode_0.c_str(), factory_0->asBase58().c_str());

    auto factory_0b = client_.Factory().PaymentCode(paycode_1, reason_);
    ASSERT_STREQ(paycode_1.c_str(), factory_0b->asBase58().c_str());

    // Factory 1: std::string
    OTPaymentCode factory_1 = client_.Factory().PaymentCode(paycode_0, reason_);
    ASSERT_STREQ(paycode_0.c_str(), factory_1->asBase58().c_str());

    auto factory_1b = client_.Factory().PaymentCode(paycode_1, reason_);
    ASSERT_STREQ(paycode_1.c_str(), factory_1b->asBase58().c_str());

    // Factory 2: proto::PaymentCode&
    auto factory_2 =
        client_.Factory().PaymentCode(*factory_1->Serialize(), reason_);
    ASSERT_STREQ(paycode_0.c_str(), factory_2->asBase58().c_str());

    auto factory_2b =
        client_.Factory().PaymentCode(*factory_1b->Serialize(), reason_);
    ASSERT_STREQ(paycode_1.c_str(), factory_2b->asBase58().c_str());

    // Factory 3: std:
    proto::HDPath path;
    const Nym_p nym =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_0), reason_);
    EXPECT_TRUE(nym.get()->Path(path));
    std::string fingerprint = path.root();

    auto factory_3 = client_.Factory().PaymentCode(
        fingerprint, 0, 1, reason_);  // seed, nym, paycode version
    ASSERT_STREQ(paycode_0.c_str(), factory_3->asBase58().c_str());

    auto factory_3b = client_.Factory().PaymentCode(
        fingerprint, 1, 1, reason_);  // seed, nym, paycode version

    ASSERT_STREQ(paycode_1.c_str(), factory_3b->asBase58().c_str());
}

/* Test: factory method with nym has private key
 */
TEST_F(Test_PaymentCode, factory_seed_nym)
{
    std::string seed = client_.Seeds().DefaultSeed();
    [[maybe_unused]] std::uint32_t nym_idx = 0;
    [[maybe_unused]] std::uint8_t version = 1;
    [[maybe_unused]] bool bitmessage = false;
    [[maybe_unused]] std::uint8_t bitmessage_version = 0;
    [[maybe_unused]] std::uint8_t bitmessage_stream = 0;

    const Nym_p nym =
        client_.Wallet().Nym(identifier::Nym::Factory(nymID_0), reason_);
    proto::HDPath path;
    EXPECT_TRUE(nym.get()->Path(path));

    std::string fingerprint = path.root();
    auto privatekey = client_.Seeds().GetPaymentCode(fingerprint, 10, reason_);
    proto::AsymmetricKey privKey = *privatekey;
    ASSERT_TRUE(bool(privatekey));
}
}  // namespace
#endif
