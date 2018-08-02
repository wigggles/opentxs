// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SERVER_NOTARY_HPP
#define OPENTXS_SERVER_NOTARY_HPP

#include "Internal.hpp"

namespace opentxs
{
class Account;
class ClientContext;
class Nym;
class OTTransaction;

namespace api
{
class Server;

namespace client
{
class Wallet;
}  // namespace client
}  // namespace api

namespace server
{

class Server;

class Notary
{
public:
    void NotarizeProcessInbox(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeProcessNymbox(
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransaction(
        ClientContext& context,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

private:
    friend class Server;

    Server& server_;
    const opentxs::api::Legacy& legacy_;
    const opentxs::api::Server& mint_;
    const opentxs::api::Wallet& wallet_;

    void NotarizeCancelCronItem(
        ClientContext& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeDeposit(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeExchangeBasket(
        ClientContext& context,
        ExclusiveAccount& sourceAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeMarketOffer(
        ClientContext& context,
        ExclusiveAccount& assetAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePayDividend(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizePaymentPlan(
        ClientContext& context,
        ExclusiveAccount& depositorAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeSmartContract(
        ClientContext& context,
        ExclusiveAccount& activatingAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeTransfer(
        ClientContext& context,
        ExclusiveAccount& fromAccount,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);
    void NotarizeWithdrawal(
        ClientContext& context,
        ExclusiveAccount& account,
        OTTransaction& tranIn,
        OTTransaction& tranOut,
        bool& outSuccess);

    explicit Notary(
        Server& server,
        const opentxs::api::Legacy& legacy,
        const opentxs::api::Server& mint,
        const opentxs::api::Wallet& wallet);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    Notary& operator=(const Notary&) = delete;
    Notary& operator=(Notary&&) = delete;
};
}  // namespace server
}  // namespace opentxs

#endif  // OPENTXS_SERVER_NOTARY_HPP
