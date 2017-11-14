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

#include "opentxs/Version.hpp"

namespace opentxs
{

class Account;
class ClientContext;
class Nym;
class OTTransaction;

namespace api
{
class Server;
class Wallet;
}  // namespace api

namespace server
{

class Server;

class Notary
{
public:
    void NotarizeProcessInbox(
        Nym& nym,
        ClientContext& context,
        Account& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeProcessNymbox(
        Nym& nym,
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransaction(
        Nym& nym,
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

private:
    friend class Server;

    Server& server_;
    opentxs::api::Server& mint_;
    opentxs::api::Wallet& wallet_;

    void NotarizeCancelCronItem(
        Nym& nym,
        ClientContext& context,
        Account& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeDeposit(
        Nym& nym,
        ClientContext& context,
        Account& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeExchangeBasket(
        Nym& nym,
        ClientContext& context,
        Account& sourceAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeMarketOffer(
        Nym& nym,
        ClientContext& context,
        Account& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePayDividend(
        Nym& nym,
        ClientContext& context,
        Account& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePaymentPlan(
        Nym& nym,
        ClientContext& context,
        Account& depositorAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeSmartContract(
        Nym& nym,
        ClientContext& context,
        Account& activatingAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransfer(
        Nym& nym,
        ClientContext& context,
        Account& fromAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeWithdrawal(
        Nym& nym,
        ClientContext& context,
        Account& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

    explicit Notary(
        Server& server,
        opentxs::api::Server& mint,
        opentxs::api::Wallet& wallet);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    Notary& operator=(const Notary&) = delete;
    Notary& operator=(Notary&&) = delete;
};
}  // namespace server
}  // namespace opentxs

#endif  // OPENTXS_SERVER_NOTARY_HPP
