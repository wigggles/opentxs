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

proto::BlockchainTransaction* MakeTransaction()
{
  proto::BlockchainTransaction* Tx = new proto::BlockchainTransaction;
  Tx->set_version(1);
  Tx->set_txid("6ddfedaf76b3abd902e1860115e163957aa16f72fc56b1f61bf314fc37781616");
  Tx->set_chain(static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC));
  Tx->set_txversion(1);
  Tx->set_fee(1827);
  Tx->set_memo("memo1");
  Tx->set_confirmations(7);
  return Tx;
}

/* Mark an address as used/allocated. associate tx
   Checks internal index increase, tx id matches
*/ 
TEST(Test_Blockchain, testStoreIncoming)
{
  std::cout << "Started testStoreIncoming !!\n";
  
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

  // Allocate address, check internal index is 1
  const std::string& label = "Address label";
  std::unique_ptr<proto::Bip44Address> AccountAddress = opentxs::OT::App()
    .Blockchain().AllocateAddress(
                                  Identifier(Alice),
                                  Identifier(AliceAccountID),
                                  label, BTC);
  proto::Bip44Address Address = *AccountAddress.get();
  std::cout << "\nCreated Address " << Address.address() << " !!\n";

  proto::BlockchainTransaction* Tx = MakeTransaction();

  // expect transaction serialization to work
  std::string output;
  bool serialized = Tx->SerializeToString(&output);  
  EXPECT_TRUE(serialized);      
  EXPECT_STRNE(output.c_str(), "");

  // expect address to have no incoming transactions  
  std::unique_ptr<proto::Bip44Address> AddrPtr = opentxs::OT::App()
    .Blockchain().LoadAddress(
                              Identifier(Alice),
                              Identifier(AliceAccountID),
                              0, BTC);
  EXPECT_TRUE(bool(AddrPtr));
  proto::Bip44Address LoadedAddress = *AddrPtr.get();
  EXPECT_EQ(LoadedAddress.incoming_size(), 0);
  
  // Associate Address at index 0 of Alice's account with an incoming transaction
  bool Stored = opentxs::OT::App().Blockchain().StoreIncoming(
                                Identifier(Alice), //nymID
                                Identifier(AliceAccountID), 
                                0, BTC, *Tx);

  std::cout << "Stored incoming transaction " << Tx->txid() << " !!\n";
  EXPECT_TRUE(Stored);  
  
  // expect transaction restore to work
  std::string TXID = Tx->txid();  
  std::shared_ptr<proto::BlockchainTransaction> StoredIncomingTx = opentxs::OT::App().Blockchain().Transaction(Tx->txid());
  proto::BlockchainTransaction& StoredTx = *StoredIncomingTx.get();
  
  EXPECT_TRUE(bool(StoredIncomingTx));
  EXPECT_EQ(StoredTx.version(), 1);
  EXPECT_EQ(StoredTx.txversion(), 1);
  EXPECT_EQ(StoredTx.chain(), static_cast<proto::ContactItemType>(BTC));
  EXPECT_EQ(StoredTx.fee(), 1827);
  EXPECT_EQ(StoredTx.confirmations(), 7);
  EXPECT_STREQ(StoredTx.memo().c_str(), "memo1");

  
  // Check assignment: expect address to have an incoming txid matching the Stored tx
  std::unique_ptr<proto::Bip44Address> NewAddrPtr = opentxs::OT::App()
    .Blockchain().LoadAddress(
                              Identifier(Alice),
                              Identifier(AliceAccountID),
                              0, BTC);
  EXPECT_TRUE(bool(NewAddrPtr));
  proto::Bip44Address NewLoadedAddress = *NewAddrPtr.get();
  EXPECT_EQ(NewLoadedAddress.incoming_size(), 1);
  
  EXPECT_EQ(NewLoadedAddress.index(), 0);
  EXPECT_STREQ(NewLoadedAddress.address().c_str(), Address.address().c_str());
  EXPECT_EQ(NewLoadedAddress.incoming_size(), 1);
  EXPECT_STREQ(NewLoadedAddress.incoming(0).c_str(), StoredTx.txid().c_str());
}
}
