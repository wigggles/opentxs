// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

/*
 */

TEST(Test_Blockchain, testAssignIncomingAddress)
{
    // create nym and account
    static const proto::ContactItemType INDIVIDUAL =
        proto::CITEMTYPE_INDIVIDUAL;
    const auto Alice = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "Alice", "", 30);
    std::cout << "Created Alice's Nym: " << Alice << " !!\n";
    const auto Bob = opentxs::OT::App().Client().Exec().CreateNymHD(
        INDIVIDUAL, "Bob", "", 40);
    std::cout << "Created Bob's Nym: " << Bob << " !!\n";

    OTIdentifier AliceAccountID = OT::App().Blockchain().NewAccount(
        Identifier::Factory(Alice),
        BlockchainAccountType::BIP44,
        proto::CITEMTYPE_BTC);
    std::shared_ptr<proto::Bip44Account> AliceAccount =
        OT::App().Blockchain().Account(
            Identifier::Factory(Alice), AliceAccountID);

    // std::cout << "Created Account " << String(AliceAccount).Get() << " !!\n";
    // std::cout << "\nCreated Address " << Address.address() << " (length " <<
    // Address.address().length()<< ")!!\n";

    // Check that account current index is 0
    ASSERT_EQ((*AliceAccount.get()).internalindex(), 0);
    ASSERT_EQ((*AliceAccount.get()).externalindex(), 0);

    // Allocate address, check internal index is 1
    const std::string label = "Address label";
    std::unique_ptr<proto::Bip44Address> AccountAddress =
        opentxs::OT::App().Blockchain().AllocateAddress(
            Identifier::Factory(Alice),
            Identifier::Factory(AliceAccountID),
            label,
            proto::CITEMTYPE_BTC);
    proto::Bip44Address Address = *AccountAddress.get();
    std::cout << "\nCreated Address " << Address.address() << " (length "
              << Address.address().length() << ")!!\n";
    // check index count increases
    ASSERT_EQ((*AliceAccount.get()).internalindex(), 1);
    ASSERT_EQ((*AliceAccount.get()).externalindex(), 0);

    //

    bool assigned = opentxs::OT::App().Blockchain().AssignAddress(
        Identifier::Factory(Alice),
        Identifier::Factory(AliceAccountID),
        0,
        Identifier::Factory(Bob),
        proto::CITEMTYPE_BTC);

    std::cout << "Assigned address: " << assigned << " !!\n";
    EXPECT_TRUE(assigned);

    // Check assignment
    std::unique_ptr<proto::Bip44Address> AddrPtr =
        opentxs::OT::App().Blockchain().LoadAddress(
            Identifier::Factory(Alice),
            Identifier::Factory(AliceAccountID),
            0,
            proto::CITEMTYPE_BTC);

    EXPECT_TRUE(bool(AddrPtr));

    proto::Bip44Address LoadedAddress = *AddrPtr.get();

    EXPECT_STREQ(LoadedAddress.contact().c_str(), Bob.c_str());
    EXPECT_EQ(LoadedAddress.index(), 0);
    EXPECT_EQ(LoadedAddress.version(), 1);
    EXPECT_STREQ(LoadedAddress.address().c_str(), Address.address().c_str());
    EXPECT_EQ(LoadedAddress.incoming_size(), 0);
}
}  // namespace
