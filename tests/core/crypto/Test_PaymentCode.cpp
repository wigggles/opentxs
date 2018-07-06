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

#include "opentxs/opentxs.hpp"
#include "opentxs/core/crypto/Bip32.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_PaymentCode : public ::testing::Test
{
public:
    std::string seed, fingerprint, nymID_0, paycode_0, nymID_1, paycode_1, nymID_2, paycode_2, nymID_3, paycode_3;
    opentxs::NymData nymData_0, nymData_1, nymData_2, nymData_3;
    proto::ContactItemType currency = opentxs::proto::CITEMTYPE_BCH;
    proto::ContactItemType currency_2 = opentxs::proto::CITEMTYPE_BTC;

    /* Is evaluated every test, therefore indexes are fixed to 0,1,2,3 */
    Test_PaymentCode()
        : seed("trim thunder unveil reduce crop cradle zone inquiry anchor skate property fringe obey butter text tank drama palm guilt pudding laundry stay axis prosper") 
        , fingerprint(opentxs::OT::App().API().Exec().Wallet_ImportSeed(seed, ""))
        , nymID_0(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                          "PaycodeNym", fingerprint, 0))
        , nymID_1(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                          "PaycodeNym_1", fingerprint, 1))
        , nymID_2(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                          "PaycodeNym_2", fingerprint, 2))
        , nymID_3(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                          "PaycodeNym_3", fingerprint, 3))
        , nymData_0(opentxs::OT::App().Wallet().mutable_Nym(Identifier(nymID_0)))
        , nymData_1(opentxs::OT::App().Wallet().mutable_Nym(Identifier(nymID_1)))
        , nymData_2(opentxs::OT::App().Wallet().mutable_Nym(Identifier(nymID_2)))
        , nymData_3(opentxs::OT::App().Wallet().mutable_Nym(Identifier(nymID_3)))
        , paycode_0("PM8TJhB2CxWDqR8c5y4kWoJwSGRNYaVATdJM85kqfn2dZ9TdSihbFJraQzjYUMYxbsrnMfjPK6oZFAPQ1tWqzwTfKbtunvLFCzDJFVXVGbUAKxhsz7P5")
        , paycode_1("PM8TJWedQTvxaoJpt9Wh25HR54oj5vmor6arAByFk4UTgUh1Tna2srsZLUo2xS3ViBot1ftf4p8ZUN8khB2zvViHXZkrwkfjcePSeEgsYapESKywge9F")
        , paycode_2("PM8TJQmrQ4tSY6Gad59UpzqR8MRMesSYMKXvpMuzdDHByfRXVgvVdiqD5NmjoEH9V6ZrofFVViBwSg9dvVcP8R2CU1pXejhVQQj3XsWk8sLhAsspqk8F")
        , paycode_3("PM8TJbNzqDcdqCcpkMLLa9H83CjoWdHMTQ4Lk11qSpThkyrmDFA4AeGd2kFeLK2sT6UVXy2jwWABsfLd7JmcS4hMAy9zUdWRFRhmu33RiRJCS6qRmGew")
    {
        nymData_0.AddPaymentCode(paycode_0, currency, true, true);

        nymData_1.AddPaymentCode(paycode_0, currency, true, true);
        nymData_1.AddPaymentCode(paycode_1, currency, true, false); // reset nymdata_1 to paymentcode_1
        
        nymData_2.AddPaymentCode(paycode_2, currency_2, false, true); // nymdata_2 resets paycode_2 to primary
        nymData_2.AddPaymentCode(paycode_0, currency_2, false, true); // fail to reset nymdata_2 with primary = false
        
        nymData_3.AddPaymentCode(paycode_3, currency_2, false, false); // nymdata_3 resets paycode_3 to primary 
        nymData_3.AddPaymentCode(paycode_0, currency_2, false, false); // fail to be primary
    }
};

/* Test: Gets the last paymentcode to be set as primary
*/
TEST_F(Test_PaymentCode, primary_paycodes)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str()); // primary and active
    ASSERT_STREQ(paycode_1.c_str(), nymData_1.PaymentCode(currency).c_str()); // primary but inactive overrides primary active
    ASSERT_STREQ(paycode_2.c_str(), nymData_2.PaymentCode(currency_2).c_str()); // not primary but active defaults to primary
    ASSERT_STREQ(paycode_3.c_str(), nymData_3.PaymentCode(currency_2).c_str()); // not primary nor active defaults to primary

    auto nym0 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_0));
    auto nym1 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_1));
    auto nym2 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_2));
    auto nym3 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_3));

    ASSERT_STREQ(nym0->PaymentCode().c_str(), paycode_0.c_str());
    ASSERT_STREQ(nym1->PaymentCode().c_str(), paycode_1.c_str());
    ASSERT_STREQ(nym2->PaymentCode().c_str(), paycode_2.c_str());
    ASSERT_STREQ(nym3->PaymentCode().c_str(), paycode_3.c_str());
}

/* Test: by setting primary = true it resets best payment code 
*/
TEST_F(Test_PaymentCode, test_new_primary)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
    
    ASSERT_TRUE(nymData_0.AddPaymentCode(paycode_2, currency, true, true));
    ASSERT_STREQ(paycode_2.c_str(), nymData_0.PaymentCode(currency).c_str());
    
    ASSERT_TRUE(nymData_0.AddPaymentCode(paycode_3, currency, true, false));
    ASSERT_STREQ(paycode_3.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: by setting primary = false it should not override a previous primary
*/
TEST_F(Test_PaymentCode, test_secondary_doesnt_replace)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
    
    ASSERT_TRUE(nymData_0.AddPaymentCode(paycode_2, currency, false, false));
    ASSERT_TRUE(nymData_0.AddPaymentCode(paycode_3, currency, false, true));
    
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: Valid paycodes
*/
TEST_F(Test_PaymentCode, valid_paycodes)
{
    ASSERT_TRUE(PaymentCode::Factory(paycode_0)->VerifyInternally());
    ASSERT_TRUE(PaymentCode::Factory(paycode_1)->VerifyInternally());
    ASSERT_TRUE(PaymentCode::Factory(paycode_2)->VerifyInternally());
    ASSERT_TRUE(PaymentCode::Factory(paycode_3)->VerifyInternally());
}

/* Test: Invalid paycodes should not be saved
*/  
TEST_F(Test_PaymentCode, empty_paycode)
{
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
    
    ASSERT_FALSE(PaymentCode::Factory("")->VerifyInternally());
    bool added = nymData_0.AddPaymentCode("", currency, true, true);
    ASSERT_FALSE(added);
    
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
    
    std::string invalid_paycode = "XM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxteQ5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97";
    ASSERT_FALSE(PaymentCode::Factory(invalid_paycode)->VerifyInternally());
    
    added = nymData_0.AddPaymentCode(invalid_paycode, currency, true, true);
    ASSERT_FALSE(added);
    ASSERT_STRNE("", nymData_0.PaymentCode(currency).c_str());
    
    ASSERT_STREQ(paycode_0.c_str(), nymData_0.PaymentCode(currency).c_str());
}

/* Test: equals operator
*/  
TEST_F(Test_PaymentCode, paycode_equals)
{
    OTPaymentCode payment_code = PaymentCode::Factory(paycode_0);
    PaymentCode& rep1 = payment_code.get();
    proto::PaymentCode& rep2 = *(payment_code->Serialize());
    ASSERT_TRUE(rep1 == rep2);
}

/* Test: Base58 encoding
*/  
TEST_F(Test_PaymentCode, asBase58)
{
    auto pcode = PaymentCode::Factory(paycode_0);
    ASSERT_STREQ(paycode_0.c_str(), pcode->asBase58().c_str());    
}

/* Test: Paymentcode's nym ID matches nymidsource
 */ 
TEST_F(Test_PaymentCode, paycode_derivation_matches_nymidsource)
{
    auto nym_0_paycode = PaymentCode::Factory(fingerprint, 0, 1); // seed, nym, paycode version
    auto nym_1_paycode = PaymentCode::Factory(fingerprint, 1, 1); // seed, nym, paycode version
    auto nym_2_paycode = PaymentCode::Factory(fingerprint, 2, 1); // seed, nym, paycode version
    auto nym_3_paycode = PaymentCode::Factory(fingerprint, 3, 1); // seed, nym, paycode version
    
    ASSERT_STREQ(paycode_0.c_str(), nym_0_paycode->asBase58().c_str());
    ASSERT_STREQ(paycode_1.c_str(), nym_1_paycode->asBase58().c_str());
    ASSERT_STREQ(paycode_2.c_str(), nym_2_paycode->asBase58().c_str());
    ASSERT_STREQ(paycode_3.c_str(), nym_3_paycode->asBase58().c_str());
    
    auto idsource_0 = new NymIDSource(nym_0_paycode);
    auto idsource_1 = new NymIDSource(nym_1_paycode);
    auto idsource_2 = new NymIDSource(nym_2_paycode);
    auto idsource_3 = new NymIDSource(nym_3_paycode);
    
    ASSERT_STREQ(nymID_0.c_str(), String(idsource_0->NymID()).Get());
    ASSERT_STREQ(nymID_1.c_str(), String(idsource_1->NymID()).Get());
    ASSERT_STREQ(nymID_2.c_str(), String(idsource_2->NymID()).Get());
    ASSERT_STREQ(nymID_3.c_str(), String(idsource_3->NymID()).Get());
}

/* Test: Factory methods create the same paycode
*/  
TEST_F(Test_PaymentCode, factory)
{
    //Factory 0: PaymentCode&
    auto factory_0 = PaymentCode::Factory( PaymentCode::Factory(paycode_0).get());
    ASSERT_STREQ(paycode_0.c_str(), factory_0->asBase58().c_str());

    auto factory_0b = PaymentCode::Factory( PaymentCode::Factory(paycode_1).get());
    ASSERT_STREQ(paycode_1.c_str(), factory_0b->asBase58().c_str());

    // Factory 1: std::string
    OTPaymentCode factory_1 = PaymentCode::Factory(paycode_0);
    ASSERT_STREQ(paycode_0.c_str(), factory_1->asBase58().c_str());

    auto factory_1b = PaymentCode::Factory(paycode_1);
    ASSERT_STREQ(paycode_1.c_str(), factory_1b->asBase58().c_str());

    // Factory 2: proto::PaymentCode&
    auto factory_2 = PaymentCode::Factory(*factory_1->Serialize());
    ASSERT_STREQ(paycode_0.c_str(), factory_2->asBase58().c_str());

    auto factory_2b = PaymentCode::Factory(*factory_1b->Serialize());
    ASSERT_STREQ(paycode_1.c_str(), factory_2b->asBase58().c_str());
    
    // Factory 3: std:
    proto::HDPath path;
    const ConstNym nym = opentxs::OT::App().Wallet().Nym(Identifier(nymID_0));
    EXPECT_TRUE(nym.get()->Path(path));
    std::string fingerprint = path.root();   
    
    auto factory_3 = PaymentCode::Factory(fingerprint, 0, 1); // seed, nym, paycode version           
    ASSERT_STREQ(paycode_0.c_str(), factory_3->asBase58().c_str());

    auto factory_3b = PaymentCode::Factory(fingerprint, 1, 1); // seed, nym, paycode version
    
    ASSERT_STREQ(paycode_1.c_str(), factory_3b->asBase58().c_str());
    
}

/* Test: factory method with nym has private key
 */
TEST_F(Test_PaymentCode, factory_seed_nym)
{
    std::string seed = opentxs::OT::App().Crypto().BIP39().DefaultSeed();
    std::uint32_t nym_idx = 0;
    std::uint8_t version = 1;
    bool bitmessage = false;
    std::uint8_t bitmessage_version = 0;
    std::uint8_t bitmessage_stream = 0;

    const ConstNym nym = opentxs::OT::App().Wallet().Nym(Identifier(nymID_0));
    proto::HDPath path;
    EXPECT_TRUE(nym.get()->Path(path));

    std::string fingerprint = path.root();
    serializedAsymmetricKey privatekey = opentxs::OT::App().Crypto().BIP32().GetPaymentCode(fingerprint, 10);
    proto::AsymmetricKey privKey = *privatekey;
    ASSERT_TRUE(bool(privatekey));
}

/* Test: Two nyms have a paycode each and nymidsource says so
*/
TEST_F(Test_PaymentCode, two_nyms)
{
    const ConstNym nym_0 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_0));
    const ConstNym nym_1 = opentxs::OT::App().Wallet().Nym(Identifier(nymID_1));
    
    auto idsource_0 = new NymIDSource(PaymentCode::Factory(paycode_0));
    auto idsource_1 = new NymIDSource(PaymentCode::Factory(paycode_1));
    
    ASSERT_TRUE(bool(idsource_0));
    ASSERT_TRUE(bool(idsource_1));
    
    ASSERT_STREQ(nym_0->ID().str().c_str(), String(idsource_0->NymID()).Get());
    ASSERT_STREQ(nym_1->ID().str().c_str(), String(idsource_1->NymID()).Get());
    
    ASSERT_STRNE(String(idsource_0->NymID()).Get(),"");
    ASSERT_STRNE(String(idsource_1->NymID()).Get(),"");
}

}  // namespace
