// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class ServerAction : virtual public opentxs::api::client::ServerAction
{
public:
    Action AcknowledgeBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& instructions) const override;
    Action AcknowledgeConnection(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key) const override;
    Action AcknowledgeNotice(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack) const override;
    Action AcknowledgeOutbailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& details) const override;
    Action ActivateSmartContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract) const override;
    Action AddServerClaim(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::string& value,
        bool primary) const override;
    Action AdjustUsageCredits(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Amount adjustment) const override;
    Action CancelPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const override;
    Action CreateMarketOffer(
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount scale,
        const Amount increment,
        const Amount quantity,
        const Amount price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const std::string& stopSign,
        const Amount activationPrice) const override;
#if OT_CASH
    Action DepositCashPurse(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        std::unique_ptr<Purse>& purse) const override;
#endif  // OT_CASH
    Action DepositCheque(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        std::unique_ptr<Cheque>& cheque) const override;
    Action DepositPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const override;
    bool DownloadAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const bool forceDownload) const override;
    Action DownloadBoxReceipt(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const RemoteBoxType box,
        const TransactionNumber item) const override;
    Action DownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const override;
    Action DownloadMarketList(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    Action DownloadMarketOffers(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& marketID,
        const Amount depth) const override;
    Action DownloadMarketRecentTrades(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& marketID) const override;
#if OT_CASH
    Action DownloadMint(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID) const override;
#endif  // OT_CASH
    Action DownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const override;
    bool DownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    Action DownloadNymMarketOffers(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
    Action ExchangeBasketCurrency(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const override;
#if OT_CASH
    Action ExchangeCash(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        std::unique_ptr<Purse>& purse) const override;
#endif  // OT_CASH
    bool GetTransactionNumbers(
        const Identifier& localNymID,
        const Identifier& serverID,
        const std::size_t quantity) const override;
    Action InitiateBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID) const override;
    Action InitiateOutbailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID,
        const Amount amount,
        const std::string& message) const override;
    Action InitiateRequestConnection(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const proto::ConnectionInfoType& type) const override;
    Action InitiateStoreSecret(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const proto::SecretType& type,
        const std::string& primary,
        const std::string& secondary) const override;
    Action IssueBasketCurrency(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::UnitDefinition& basket) const override;
    Action IssueUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::UnitDefinition& contract) const override;
    Action KillMarketOffer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const override;
    Action KillPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const override;
    Action NotifyBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount) const override;
    Action PayDividend(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const std::string& memo,
        const Amount amountPerShare) const override;
    Action ProcessInbox(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        std::unique_ptr<Ledger>& ledger) const override;
    Action PublishNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const override;
    Action PublishServerContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetServerID) const override;
    Action PublishUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitDefinitionID) const override;
    Action RegisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID) const override;
    Action RegisterNym(const Identifier& localNymID, const Identifier& serverID)
        const override;
    Action RequestAdmin(
        const Identifier& localNymID,
        const Identifier& serverID,
        const std::string& password) const override;
    Action ResyncContext(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
#if OT_CASH
    Action SendCash(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        std::shared_ptr<const Purse>& recipientCopy,
        std::shared_ptr<const Purse>& senderCopy) const override;
#endif  // OT_CASH
    Action SendMessage(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        const std::string& message) const override;
    Action SendPayment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        std::shared_ptr<const OTPayment>& payment) const override;
    Action SendTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& senderAccountID,
        const Identifier& recipientAccountID,
        const Amount amount,
        const std::string& memo) const override;
    Action TriggerClause(
        const Identifier& localNymID,
        const Identifier& serverID,
        const TransactionNumber transactionNumber,
        const std::string& clause,
        const std::string& parameter) const override;
    Action UnregisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const override;
    Action UnregisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const override;
#if OT_CASH
    Action WithdrawCash(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const Amount amount) const override;
#endif  // OT_CASH
    Action WithdrawVoucher(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const Identifier& recipientNymID,
        const Amount amount,
        const std::string& memo) const override;

    ~ServerAction() = default;

private:
    friend opentxs::Factory;

    const api::client::Manager& api_;
    ContextLockCallback lock_callback_;

    ServerAction(
        const api::client::Manager& api,
        const ContextLockCallback& lockCallback);
    ServerAction() = delete;
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace opentxs::api::client::implementation
