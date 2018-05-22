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

#include "opentxs/api/Blockchain.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"
using namespace opentxs;

namespace {

  /* Mark an address as used/allocated.
     Checks internal index increase
   */ 
  
TEST(Test_AllocateAddress, testAllocateAddress)
{
  std::cout << "Started testAllocate !!\n";
  
  // create nym and account
  static const proto::ContactItemType INDIVIDUAL = proto::CITEMTYPE_INDIVIDUAL;
  const std::string& Alice = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "Alice", "", 0);
  std::cout << "Created Alice's Nym: " << Alice << " !!\n";
  const std::uint32_t BTC = proto::CITEMTYPE_BTC;
      
  OTIdentifier AliceAccountID = OT::App().Blockchain().NewAccount(
                                                              Identifier(Alice),
                                                              BlockchainAccountType::BIP44,
                                                              static_cast<proto::ContactItemType>(BTC)); 
  std::shared_ptr<proto::Bip44Account> AliceAccount = OT::App().Blockchain().Account(Identifier(Alice), AliceAccountID);

  std::cout << "Created Account " << String(AliceAccountID).Get() << " !!\n";
  // Check that account current index is 0
  ASSERT_EQ((*AliceAccount.get()).internalindex(), 0);
  ASSERT_EQ((*AliceAccount.get()).externalindex(), 0);

  // Allocate address, check internal index is 1
  const std::string& label = "Address label";
  std::unique_ptr<proto::Bip44Address> AccountAddress = opentxs::OT::App()
    .Blockchain().AllocateAddress(
                                  Identifier(Alice),
                                  Identifier(AliceAccountID),
                                  label, false);
  proto::Bip44Address Address = *AccountAddress.get();
  std::cout << "\nCreated Address " << Address.address() << " (length " << Address.address().length()<< ")!!\n";
  // check index count increases
  ASSERT_EQ((*AliceAccount.get()).internalindex(), 0);
  ASSERT_EQ((*AliceAccount.get()).externalindex(), 1);

  // check new Bip44Address properties
  ASSERT_EQ(Address.version(), 1);
  ASSERT_EQ(Address.index(), 0);
  EXPECT_STRNE(Address.address().c_str(), "");
  EXPECT_GE(Address.address().length(), 25);
  EXPECT_LE(Address.address().length(), 35);
  EXPECT_STREQ(Address.label().c_str(), label.c_str());
  EXPECT_STREQ(Address.contact().c_str(), "");
  EXPECT_EQ(Address.incoming_size(), 0); 
}

TEST(Test_AllocateAddress, testGeneratedAddresses_Alice)
{
  const std::string seedID = opentxs::OT::App().API().Exec().Wallet_ImportSeed("response seminar brave tip suit recall often sound stick owner lottery motion", "");
  const std::string& Alice = opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "Alice", seedID, 0);

  OTIdentifier AccountID = OT::App().Blockchain().NewAccount(Identifier(Alice),
                                                             BlockchainAccountType::BIP32,
                                                             static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC)); 
  std::shared_ptr<proto::Bip44Account> Account = OT::App().Blockchain().Account(Identifier(Alice), AccountID);
  
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  ASSERT_EQ((*Account.get()).externalindex(), 0);

  const auto Address = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),                                                                        Identifier(AccountID), "Address 1 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address.get()).address().c_str(), "1K9teXNg8iKYwUPregT8QTmMepb376oTuX");
  ASSERT_EQ((*Address.get()).index(), 0);
  
  // check index count increases   
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  ASSERT_EQ((*Account.get()).externalindex(), 1);  
  
  const auto Address2 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),                                                                        Identifier(AccountID), "Address 2 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address2.get()).address().c_str(), "1GgpoMuPBfaa4ZT6ZeKaTY8NH9Ldx4Q89t");
  ASSERT_EQ((*Address2.get()).index(), 1);
  
  const auto Address3 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),                                                                        Identifier(AccountID), "Address 3 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address3.get()).address().c_str(), "1FXb97adaza32zYQ5U29nxHZS4FmiCfXAJ");
  ASSERT_EQ((*Address3.get()).index(), 2);
  
  const auto Address4 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 4 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address4.get()).address().c_str(), "1Dx4k7daUS1VNNeoDtZe1ujpt99YeW7Yz");

  const auto Address5 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 5 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address5.get()).address().c_str(), "19KhniSVj1CovZWg1P5JvoM199nQR3gkhp");

  const auto Address6 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 6 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address6.get()).address().c_str(), "1CBnxZdo58Vu3upwEt96uTMZLAxVx4Xeg9");

  const auto Address7 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 7 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address7.get()).address().c_str(), "12vm2SqQ7RhhYPi6bJqqQzyJomV6H3j4AX");

  const auto Address8 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 8 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address8.get()).address().c_str(), "1D2fNJYjyWL1jn5qRhJZL6EbGzeyBjHuP3");

  const auto Address9 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 9 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address9.get()).address().c_str(), "19w4gVEse89JjE7TroavXZ9pyfJ78h4arG");

  const auto Address10 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                        Identifier(AccountID), "Address 10 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address10.get()).address().c_str(), "1DVYvYAmTNtvML7vBrhBBhyePaEDVCCNaw");
  ASSERT_EQ((*Address10.get()).index(), 9);
    
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  //ASSERT_EQ((*Account.get()).externalindex(), 10);
}

TEST(Test_AllocateAddress, testGeneratedAddresses_Bob)
{
  const std::string seedID = opentxs::OT::App().API().Exec().Wallet_ImportSeed("reward upper indicate eight swift arch injury crystal super wrestle already dentist", "");
  const std::string& Bob = opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "Bob", seedID, 0);  

  // create account
  OTIdentifier AccountID = OT::App().Blockchain().NewAccount(
                                                             Identifier(Bob),
                                                             BlockchainAccountType::BIP32,
                                                             static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC)); 
  std::shared_ptr<proto::Bip44Account> Account = OT::App().Blockchain().Account(Identifier(Bob), AccountID);

  std::cout << "Created Account " << String(AccountID).Get() << " !!\n";
  // Check that account current index is 0
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  //ASSERT_EQ((*Account.get()).externalindex(), 1);

  // Allocate address, check internal index is 1
  const std::string& label = "Address label";

  std::unique_ptr<proto::Bip44Address> AddressPtr = opentxs::OT::App()
    .Blockchain().AllocateAddress(
                                  Identifier(Bob),
                                  Identifier(AccountID),
                                  "Label", EXTERNAL_CHAIN);
  proto::Bip44Address Address = *AddressPtr.get();
  std::cout << "\nCreated Bob's Address " << Address.address() << " (length " << Address.address().length()<< ")!!\n";


  // check index count increases   
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  ASSERT_EQ((*Account.get()).externalindex(), 1);
    
  // check new Bip44Address properties*/
  ASSERT_EQ(Address.version(), 1);
  ASSERT_EQ(Address.index(), 0);
  EXPECT_STREQ(Address.address().c_str(), "1AngXb5xQoQ4nT8Bn6dDdr6AFS4yMZU2y");
  
  const auto Address2 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),                                                                        Identifier(AccountID), "Address 2 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address2.get()).address().c_str(), "1FQMy3HkD5C3gGZZHeeH9rjHgyqurxC44q");
  ASSERT_EQ((*Address2.get()).index(), 1);
  
  const auto Address3 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),                                                                        Identifier(AccountID), "Address 3 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address3.get()).address().c_str(), "1APXZ5bCTbj2ZRV3ZHyAa59CmsXRP4HkTh");
  ASSERT_EQ((*Address3.get()).index(), 2);
  
  const auto Address4 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 4 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address4.get()).address().c_str(), "1M966pvtChYbceTsou73eB2hutwoZ7QtVv");

  const auto Address5 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 5 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address5.get()).address().c_str(), "1HcN6BWFZKLNEdBo15oUPQGXpDJ26SVKQE");

  const auto Address6 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 6 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address6.get()).address().c_str(), "1NcaLRLFr4edY4hUcR81aNMpveHaRqzxPR");

  const auto Address7 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 7 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address7.get()).address().c_str(), "1CT86ZmqRFZW57aztRscjWuzkhJjgHjiMS");

  const auto Address8 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 8 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address8.get()).address().c_str(), "1CXT6sU5s4mxP4UattFA6fGN7yW4dkkARn");

  const auto Address9 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 9 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address9.get()).address().c_str(), "12hwhKpxTyfiSGDdQw63SWVzefRuRxrFqb");

  const auto Address10 = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Bob),
                                                                        Identifier(AccountID), "Address 10 label", EXTERNAL_CHAIN);
  EXPECT_STREQ((*Address10.get()).address().c_str(), "18SRAzD6bZ2GsTK4J4RohhYneEyZAUvyqp");
  ASSERT_EQ((*Address10.get()).index(), 9);
    
  ASSERT_EQ((*Account.get()).internalindex(), 0);
  //ASSERT_EQ((*Account.get()).externalindex(), 10);
} 
}
