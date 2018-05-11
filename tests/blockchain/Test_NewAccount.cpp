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
#include "opentxs/api/Native.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Api.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"


using namespace opentxs;

namespace
{

class Test_NewAccount : public :: testing :: Test
{
public:          
  
  //const std::string aliceNymID = "Alice";
  //static std::string alice_seed;
};


  TEST(Test_NewAccount, TestNymDoesNotExist)
{
  const std::string& nymID_ = "Inexistent NYM";
  proto::ContactItemType type = opentxs::proto::ContactItemType::CITEMTYPE_EMPLOYEE;
  
  std::string Result = String(OT::App()
                         .Blockchain()
                         .NewAccount( Identifier(nymID_),
                                      BlockchainAccountType::BIP32,
                                      type))
    .Get();

  EXPECT_STREQ(Result.c_str(),"");
}

   /* Test:  when you create a nym with seed A, then the root of every HDPath for a blockchain account associated with that nym should also be A.
    */

  TEST(Test_NewAccount, TestSeedRoot)
  {
    static const proto::ContactItemType INDIVIDUAL = proto::CITEMTYPE_INDIVIDUAL;
    
    const std::string& seedA = "seed A";

    const std::string seedID = opentxs::OT::App().API().Exec().Wallet_ImportSeed(seedA, "");

    const std::string& alias = "Alias 1";
    const std::string& Nym0 = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, alias, seedID, 0);
    std::cout << "Created Nym 0: " << Nym0 << " !!\n";
    const std::string& Nym1 = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, alias, seedID, 1);    
    std::cout << "Created Nym 1: " << Nym1 << " !!\n";
  
    EXPECT_STRNE(Nym0.c_str(), Nym1.c_str());

    const std::uint32_t BTC = proto::CITEMTYPE_BTC;

    OTIdentifier Acc0ID = OT::App().Blockchain().NewAccount(Identifier(Nym0),
                                                              BlockchainAccountType::BIP32,
                                                              static_cast<proto::ContactItemType>(BTC));    

    std::cout << "Created Nym 0's Account: " << String(Acc0ID).Get() << " !!\n";
    OTIdentifier Acc1ID = OT::App().Blockchain().NewAccount(Identifier(Nym1),
                                                              BlockchainAccountType::BIP32,
                                                              static_cast<proto::ContactItemType>(BTC));

    
    std::cout << "Created Nym 1's Account: " << String(Acc1ID).Get() << " !!\n";   

    EXPECT_STRNE(String(Acc0ID).Get(), String(Acc1ID).Get());
    std::shared_ptr<proto::Bip44Account> Acc0 = OT::App().Blockchain().Account(Identifier(Nym0), Acc0ID);
    ASSERT_TRUE(bool(Acc0));
    std::shared_ptr<proto::Bip44Account> Acc1 = OT::App().Blockchain().Account(Identifier(Nym1), Acc1ID);    
    ASSERT_TRUE(bool(Acc1));
    
    proto::Bip44Account& Acc_0 = *Acc0.get();
    proto::Bip44Account& Acc_1 = *Acc1.get();    
    auto fingerprint0 = Acc_0.path().root();
    auto fingerprint1 = Acc_1.path().root();

    ASSERT_EQ(fingerprint0, fingerprint1);
  }
  
TEST(Test_NewAccount, testName)
{
  proto::ContactItemType INDIVIDUAL = proto::CITEMTYPE_INDIVIDUAL;
  const std::string& Alice = "Alice";
  const std::string& Bob = "Bob";    
  const std::string& myServer = "";
    
  // Create Alice Nym
  const std::string& CreatedAlice0 = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, Alice, "", 0);

  const std::string& CreatedAlice1 = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, Alice, "", 1);

  // Create Bob Nym
  const std::string& CreatedBob0 = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, Bob, "", 0);

  // 
  std::cout << "Created Alice 0 Nym: " << CreatedAlice0 << " !!\n";
  std::cout << "Created Alice 1 Nym: " << CreatedAlice1 << " !!\n";
  std::cout << "Created Bob 0 Nym: " << CreatedBob0 << " !!\n";
  
  const Identifier& nymID = Identifier(CreatedBob0);

  const std::uint32_t chain = proto::CITEMTYPE_BTC;
  const std::uint32_t chain2 = proto::CITEMTYPE_LTC;
  
  // Create one account
  OTIdentifier Account1 = OT::App().Blockchain().NewAccount(
                      Identifier(CreatedAlice0),
                      BlockchainAccountType::BIP32,
                      static_cast<proto::ContactItemType>(chain));
  std::string Account1Desc = String(Account1).Get();
  std::cout << "Created Account: " << Account1Desc.c_str() << " !!\n";
  
  // Create another account
  OTIdentifier Account2 = OT::App().Blockchain().NewAccount(
                      Identifier(CreatedAlice1),
                      BlockchainAccountType::BIP32,
                      static_cast<proto::ContactItemType>(chain2));
  std::string Account2Desc = String(Account2).Get();
  std::cout << "Created Account: " << Account2Desc.c_str() << " !!\n";
  
  const std::string& myAccount = Account1Desc.c_str();
  const Identifier& accountID = Identifier(myAccount);
  //const Identifier accountID(accountName);
  
  
  //const auto StoredAcc = OT::App().Blockchain().Account( nymID, accountID);

  //std::string Account1LoadDesc = String(StoredAcc).Get();
  //std::cout << "Ack: \n" << Account1LoadDesc;
  
  //std::string e = String(OT::App().Blockchain().NewAccount(
  //                    nymID,
  //                    BlockchainAccountType::BIP44               //     ,
  //                    INDIVIDUAL))
  //  .Get();

  //std::cout << "Created Account: " << e.c_str() << " !!\n";
      
  //opentxs::api::Blockchain1 blockchain = opentxs::OT::App().Blockchain();
  
  //String AccountName = String(OT.App().Blockchain().NewAccount(
  //                    nymID,
  //                    BlockchainAccountType::BIP44,
  //                   type)) .Get();
  
  //OT::App().Blockchain().NewAccount(identi

  //EXPECT_STREQ(Account1Desc.c_str(),Account2Desc.c_str());
  //EXPECT_STREQ(d.c_str(), e.c_str());
  //    ASSERT_EQ(1, 2);
  //ASSERT_EQ(AccountName, AccountName);
}

} //namespace
