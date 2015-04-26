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

#ifndef OPENTXS_SERVER_NOTARY_HPP
#define OPENTXS_SERVER_NOTARY_HPP

namespace opentxs
{

class OTTransaction;
class Nym;
class Account;
class OTServer;

class Notary
{
public:
    explicit Notary(OTServer* server);

    // If the server receives a notarizeTransaction command, it will be
    // accompanied by a payload containing a ledger to be notarized.
    // UserCmdNotarizeTransaction will loop through that ledger,
    // and for each transaction within, it calls THIS method.
    void NotarizeTransaction(Nym& nym, OTTransaction& tranIn,
                             OTTransaction& tranOut, bool& outSuccess);
    void NotarizeTransfer(Nym& nym, Account& fromAccount, OTTransaction& tranIn,
                          OTTransaction& tranOut, bool& outSuccess);
    void NotarizeDeposit(Nym& nym, Account& account, OTTransaction& tranIn,
                         OTTransaction& tranOut, bool& outSuccess);
    void NotarizeWithdrawal(Nym& nym, Account& account, OTTransaction& tranIn,
                            OTTransaction& tranOut, bool& outSuccess);
    void NotarizeProcessInbox(Nym& nym, Account& account, OTTransaction& tranIn,
                              OTTransaction& tranOut, bool& outSuccess);
    void NotarizeProcessNymbox(Nym& nym, OTTransaction& tranIn,
                               OTTransaction& tranOut, bool& outSuccess);
    void NotarizeMarketOffer(Nym& nym, Account& assetAccount,
                             OTTransaction& tranIn, OTTransaction& tranOut,
                             bool& outSuccess);
    void NotarizePaymentPlan(Nym& nym, Account& depositorAccount,
                             OTTransaction& tranIn, OTTransaction& tranOut,
                             bool& outSuccess);
    void NotarizeSmartContract(Nym& nym, Account& activatingAccount,
                               OTTransaction& tranIn, OTTransaction& tranOut,
                               bool& outSuccess);
    void NotarizeCancelCronItem(Nym& nym, Account& assetAccount,
                                OTTransaction& tranIn, OTTransaction& tranOut,
                                bool& outSuccess);
    void NotarizeExchangeBasket(Nym& nym, Account& sourceAccount,
                                OTTransaction& tranIn, OTTransaction& tranOut,
                                bool& outSuccess);
    void NotarizePayDividend(Nym& nym, Account& account, OTTransaction& tranIn,
                             OTTransaction& tranOut, bool& outSuccess);

private:
    OTServer* server_;
};

} // namespace opentxs

#endif // OPENTXS_SERVER_NOTARY_HPP
