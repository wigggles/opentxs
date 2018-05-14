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

using namespace opentxs;

namespace {

  /* Mark an address as used/allocated.
     Checks internal index increase
   */ 
  
TEST(Test_Blockchain, testAllocateAddress)
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
                                  label, BTC);
  proto::Bip44Address Address = *AccountAddress.get();
  std::cout << "\nCreated Address " << Address.address() << " (length " << Address.address().length()<< ")!!\n";
  // check index count increases
  ASSERT_EQ((*AliceAccount.get()).internalindex(), 1);
  ASSERT_EQ((*AliceAccount.get()).externalindex(), 0);

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
}
