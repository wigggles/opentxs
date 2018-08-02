// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

TEST(Test_NewAccount, TestNymDoesNotExist)
{
    const std::string nym = "Inexistent Nym";
    const std::string Result =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(nym),
                   BlockchainAccountType::BIP32,
                   proto::CITEMTYPE_BTC))
            .Get();
    EXPECT_STREQ(Result.c_str(), "");

    const std::string Result2 =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(nym),
                   BlockchainAccountType::BIP32,
                   proto::CITEMTYPE_BTC))
            .Get();
    EXPECT_STREQ(Result2.c_str(), "");
}

/* Test: when you create a nym with seed A, then the root of every HDPath for a
 * blockchain account associated with that nym should also be A.
 */

TEST(Test_NewAccount, TestSeedRoot)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;

    const std::string seedA = "seed A";

    const std::string seedID =
        opentxs::OT::App().Client().Exec().Wallet_ImportSeed(seedA, "");

    const std::string alias = "Alias 1";
    const auto Nym0 = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, alias, seedID, 0);
    std::cout << "Created Nym 0: " << Nym0 << " !!\n";
    const auto Nym1 = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, alias, seedID, 1);
    std::cout << "Created Nym 1: " << Nym1 << " !!\n";

    EXPECT_STRNE(Nym0.c_str(), Nym1.c_str());

    OTIdentifier Acc0ID = OT::App().Client().Blockchain().NewAccount(
        Identifier::Factory(Nym0),
        BlockchainAccountType::BIP32,
        proto::CITEMTYPE_BTC);

    std::cout << "Created Nym 0's Account: " << String(Acc0ID).Get() << " !!\n";
    OTIdentifier Acc1ID = OT::App().Client().Blockchain().NewAccount(
        Identifier::Factory(Nym1),
        BlockchainAccountType::BIP32,
        proto::CITEMTYPE_BTC);

    std::cout << "Created Nym 1's Account: " << String(Acc1ID).Get() << " !!\n";

    // Test difference in index on BIP32 implies a different account
    EXPECT_STRNE(String(Acc0ID).Get(), String(Acc1ID).Get());
    std::shared_ptr<proto::Bip44Account> Acc0 =
        OT::App().Client().Blockchain().Account(
            Identifier::Factory(Nym0), Acc0ID);
    ASSERT_TRUE(bool(Acc0));
    std::shared_ptr<proto::Bip44Account> Acc1 =
        OT::App().Client().Blockchain().Account(
            Identifier::Factory(Nym1), Acc1ID);
    ASSERT_TRUE(bool(Acc1));

    proto::Bip44Account& Acc_0 = *Acc0.get();
    proto::Bip44Account& Acc_1 = *Acc1.get();
    auto fingerprint0 = Acc_0.path().root();
    auto fingerprint1 = Acc_1.path().root();

    ASSERT_EQ(fingerprint0, fingerprint1);
}

/** Test that two nyms (Alice & Bob) create different accounts for the same
 * chain (BTC).
 */
TEST(Test_NewAccount, TestNymsDiff)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;

    const auto AliceNymID = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "testNymsDiff_Alice", "", 50);
    const auto BobNymID = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "testNymsDiff_Bob", "", 60);

    std::cout << "Created Alice: " << AliceNymID << " \n";
    std::cout << "Created Bob: " << BobNymID << " \n";

    OTIdentifier AliceAccountID = OT::App().Client().Blockchain().NewAccount(
        Identifier::Factory(AliceNymID),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_BTC);

    OTIdentifier BobAccountID = OT::App().Client().Blockchain().NewAccount(
        Identifier::Factory(BobNymID),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_BTC);

    std::shared_ptr<proto::Bip44Account> AliceAccount =
        OT::App().Client().Blockchain().Account(
            Identifier::Factory(AliceNymID), AliceAccountID);
    std::shared_ptr<proto::Bip44Account> BobAccount =
        OT::App().Client().Blockchain().Account(
            Identifier::Factory(BobNymID), BobAccountID);

    ASSERT_TRUE(bool(AliceAccount));
    ASSERT_TRUE(bool(BobAccount));

    ASSERT_EQ((*AliceAccount.get()).version(), 1);
    ASSERT_EQ((*AliceAccount.get()).id(), String(AliceAccountID).Get());
    ASSERT_EQ((*AliceAccount.get()).type(), proto::CITEMTYPE_BTC);
    ASSERT_EQ((*AliceAccount.get()).internalindex(), 0);
    ASSERT_EQ((*AliceAccount.get()).externalindex(), 0);

    const std::string AliceAccDescr = String(AliceAccountID).Get();
    const std::string BobAccDescr = String(BobAccountID).Get();
    EXPECT_STRNE(AliceAccDescr.c_str(), BobAccDescr.c_str());
}

/** Test that one onym creates the same account for the same chain (BIP32 or
 * BIP44).
 */
TEST(Test_NewAccount, TestNym_AccountIdempotence)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const auto Charly = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "testNymIdempotence_Charly", "", 70);

    const std::string CharlyBIP32AccountID =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(Charly),
                   BlockchainAccountType::BIP32,
                   proto::CITEMTYPE_BTC))
            .Get();

    const std::string CharlyBIP44AccountID =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(Charly),
                   BlockchainAccountType::BIP44,
                   proto::CITEMTYPE_BTC))
            .Get();

    EXPECT_STREQ(CharlyBIP32AccountID.c_str(), CharlyBIP44AccountID.c_str());

    const std::string CharlyBIP44DupAccountID =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(Charly),
                   BlockchainAccountType::BIP44,
                   proto::CITEMTYPE_BTC))
            .Get();

    EXPECT_STREQ(CharlyBIP44AccountID.c_str(), CharlyBIP44DupAccountID.c_str());
}

/** Test that the same nym creates different accounts for two chains
 */
TEST(Test_NewAccount, TestChainDiff)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const auto Alice = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "testChainDiff_Alice", "", 80);

    const std::string AliceBTCAccountID =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(Alice),
                   BlockchainAccountType::BIP32,
                   proto::CITEMTYPE_BTC))
            .Get();

    const std::string AliceLTCAccountID =
        String(OT::App().Client().Blockchain().NewAccount(
                   Identifier::Factory(Alice),
                   BlockchainAccountType::BIP44,
                   proto::CITEMTYPE_LTC))
            .Get();

    EXPECT_STRNE(AliceBTCAccountID.c_str(), AliceLTCAccountID.c_str());
}

/** Test mnemonic and passphrase vector
 */
TEST(Test_NewAccount, TestSeedPassphrase)
{
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const std::string seedID =
        opentxs::OT::App().Client().Exec().Wallet_ImportSeed(
            "fruit wave dwarf banana earth journey tattoo true farm silk olive "
            "fence",
            "banana");
    const std::string alias = "Alias 1";
    const auto Nym0 = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, alias, seedID, 0);

    EXPECT_STRNE(Nym0.c_str(), "");
    EXPECT_STRNE(alias.c_str(), Nym0.c_str());

    OTIdentifier AccountID = OT::App().Client().Blockchain().NewAccount(
        Identifier::Factory(Nym0),
        BlockchainAccountType::BIP32,
        proto::CITEMTYPE_BTC);
    std::shared_ptr<proto::Bip44Account> Acc =
        OT::App().Client().Blockchain().Account(
            Identifier::Factory(Nym0), AccountID);
    // std::string rootKey =
    // "xprv9s21ZrQH143K25QhxbucbDDuQ4naNntJRi4KUfWT7xo4EKsHt2QJDu7KXp1A3u7Bi1j8ph3EGsZ9Xvz9dGuVrtHHs7pXeTzjuxBrCmmhgC6";

    proto::Bip44Account& Account = *Acc.get();
    EXPECT_EQ(Account.path().child().size(), 1);
}
}  // namespace
