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
TEST(Test_Blockchain, testStoreOutgoing)
{
  std::cout << "Started testStoreOutgoing !!\n";
  
  // create nym and account
  static const proto::ContactItemType INDIVIDUAL = proto::CITEMTYPE_INDIVIDUAL;
  const std::string& Alice = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "Alice", "", 0);
  std::cout << "Created Alice's Nym: " << Alice << " !!\n";
  const std::string& Bob = opentxs::OT::App().API().Exec().CreateNymHD(INDIVIDUAL, "Bob", "", 1);
  std::cout << "Created Bob's Nym: " << Bob << " !!\n";
  
  const std::uint32_t BTC = proto::CITEMTYPE_BTC;
      
  OTIdentifier AliceAccountID = OT::App().Blockchain().NewAccount(
                                                              Identifier(Alice),
                                                              BlockchainAccountType::BIP44,
                                                              static_cast<proto::ContactItemType>(BTC)); 
  std::shared_ptr<proto::Bip44Account> AliceAccount = OT::App().Blockchain().Account(Identifier(Alice), AliceAccountID);

  std::cout << "Created Account " << String(AliceAccountID).Get() << " !!\n";

  // expect account to have no outgoing transactions
  ASSERT_EQ((*AliceAccount.get()).outgoing_size(), 0); 

  // Associate Address Bob to an outgoing transaction
  proto::BlockchainTransaction* Tx = MakeTransaction();
  bool Stored = opentxs::OT::App().Blockchain().StoreOutgoing(
                                Identifier(Alice), Identifier(AliceAccountID), 
                                Identifier(Bob), *Tx);

  std::cout << "Stored outgoing transaction " << Tx->txid() << " !!\n";
  EXPECT_TRUE(Stored);  
  
  // expect transaction restore to work
  std::string TXID = Tx->txid();  
  std::shared_ptr<proto::BlockchainTransaction> StoredOutgoingTx = opentxs::OT::App().Blockchain().Transaction(Tx->txid());
  proto::BlockchainTransaction& StoredTx = *StoredOutgoingTx.get();
  
  EXPECT_TRUE(bool(StoredOutgoingTx));
  
  // Check account assignment: expect to have an additional outgoing tx, matching txid
  std::shared_ptr<proto::Bip44Account> ReloadedAliceAccount = OT::App().Blockchain().Account(Identifier(Alice), AliceAccountID);
  ASSERT_EQ((*ReloadedAliceAccount.get()).outgoing_size(), 1);  
  ASSERT_STREQ((*ReloadedAliceAccount.get()).outgoing(0).c_str(), Tx->txid().c_str());
}
}
