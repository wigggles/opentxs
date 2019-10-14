// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class ServerAction final : virtual public opentxs::api::client::ServerAction
{
public:
    Action ActivateSmartContract(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract) const final;
    Action AdjustUsageCredits(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Amount adjustment) const final;
    Action CancelPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const final;
    Action CreateMarketOffer(
        const PasswordPrompt& reason,
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount scale,
        const Amount increment,
        const Amount quantity,
        const Amount price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const std::string& stopSign,
        const Amount activationPrice) const final;
    Action DepositPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const final;
    Action DownloadMarketList(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const final;
    Action DownloadMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& marketID,
        const Amount depth) const final;
    Action DownloadMarketRecentTrades(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& marketID) const final;
    Action DownloadNymMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const final;
    Action ExchangeBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const final;
    Action IssueBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const proto::UnitDefinition& basket,
        const std::string& label) const final;
    Action KillMarketOffer(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const final;
    Action KillPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const final;
    Action PayDividend(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const std::string& memo,
        const Amount amountPerShare) const final;
    Action TriggerClause(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const TransactionNumber transactionNumber,
        const std::string& clause,
        const std::string& parameter) const final;
    Action UnregisterAccount(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID) const final;
    Action UnregisterNym(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const final;
    Action WithdrawVoucher(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const identifier::Nym& recipientNymID,
        const Amount amount,
        const std::string& memo) const final;

    ~ServerAction() final = default;

private:
    friend opentxs::Factory;

    const api::client::internal::Manager& api_;
    ContextLockCallback lock_callback_;

    ServerAction(
        const api::client::internal::Manager& api,
        const ContextLockCallback& lockCallback);
    ServerAction() = delete;
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace opentxs::api::client::implementation
