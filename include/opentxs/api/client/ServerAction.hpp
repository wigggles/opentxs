// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_SERVERACTION_HPP
#define OPENTXS_API_CLIENT_SERVERACTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <chrono>
#include <string>

namespace opentxs
{
namespace api
{
namespace client
{
class ServerAction
{
public:
    using Action = Pimpl<opentxs::client::ServerAction>;

    EXPORT virtual Action AcknowledgeBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& instructions) const = 0;
    EXPORT virtual Action AcknowledgeConnection(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key) const = 0;
    EXPORT virtual Action AcknowledgeNotice(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const bool ack) const = 0;
    EXPORT virtual Action AcknowledgeOutbailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientID,
        const Identifier& requestID,
        const std::string& details) const = 0;
    EXPORT virtual Action ActivateSmartContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract) const = 0;
    EXPORT virtual Action AddServerClaim(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::string& value,
        bool primary) const = 0;
    EXPORT virtual Action AdjustUsageCredits(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Amount adjustment) const = 0;
    EXPORT virtual Action CancelPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const = 0;
    EXPORT virtual Action CreateMarketOffer(
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount scale,
        const Amount increment,
        const Amount quantity,
        const Amount price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const std::string& stopSign,
        const Amount activationPrice) const = 0;
#if OT_CASH
    EXPORT virtual Action DepositCashPurse(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const std::shared_ptr<blind::Purse>& purse) const = 0;
#endif  // OT_CASH
    EXPORT virtual Action DepositCheque(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        std::unique_ptr<Cheque>& cheque) const = 0;
    EXPORT virtual Action DepositPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const = 0;
    EXPORT virtual bool DownloadAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const bool forceDownload) const = 0;
    EXPORT virtual Action DownloadBoxReceipt(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const RemoteBoxType box,
        const TransactionNumber item) const = 0;
    EXPORT virtual Action DownloadContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& contractID) const = 0;
    EXPORT virtual Action DownloadMarketList(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual Action DownloadMarketOffers(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& marketID,
        const Amount depth) const = 0;
    EXPORT virtual Action DownloadMarketRecentTrades(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& marketID) const = 0;
#if OT_CASH
    EXPORT virtual Action DownloadMint(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID) const = 0;
#endif  // OT_CASH
    EXPORT virtual Action DownloadNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const = 0;
    EXPORT virtual bool DownloadNymbox(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual Action DownloadNymMarketOffers(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual Action ExchangeBasketCurrency(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const = 0;
    EXPORT virtual bool GetTransactionNumbers(
        const Identifier& localNymID,
        const Identifier& serverID,
        const std::size_t quantity) const = 0;
    EXPORT virtual Action InitiateBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID) const = 0;
    EXPORT virtual Action InitiateOutbailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID,
        const Amount amount,
        const std::string& message) const = 0;
    EXPORT virtual Action InitiateRequestConnection(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const proto::ConnectionInfoType& type) const = 0;
    EXPORT virtual Action InitiateStoreSecret(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const proto::SecretType& type,
        const std::string& primary,
        const std::string& secondary) const = 0;
    EXPORT virtual Action IssueBasketCurrency(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::UnitDefinition& basket,
        const std::string& label = "") const = 0;
    EXPORT virtual Action IssueUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const proto::UnitDefinition& contract,
        const std::string& label = "") const = 0;
    EXPORT virtual Action KillMarketOffer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const = 0;
    EXPORT virtual Action KillPaymentPlan(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const = 0;
    EXPORT virtual Action NotifyBailment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID,
        const Identifier& instrumentDefinitionID,
        const Identifier& requestID,
        const std::string& txid,
        const Amount amount) const = 0;
    EXPORT virtual Action PayDividend(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const Identifier& accountID,
        const std::string& memo,
        const Amount amountPerShare) const = 0;
    EXPORT virtual Action ProcessInbox(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        std::unique_ptr<Ledger>& ledger) const = 0;
    EXPORT virtual Action PublishNym(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetNymID) const = 0;
    EXPORT virtual Action PublishServerContract(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& targetServerID) const = 0;
    EXPORT virtual Action PublishUnitDefinition(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& unitDefinitionID) const = 0;
    EXPORT virtual Action RegisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& instrumentDefinitionID,
        const std::string& label = "") const = 0;
    EXPORT virtual Action RegisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
    EXPORT virtual Action RequestAdmin(
        const Identifier& localNymID,
        const Identifier& serverID,
        const std::string& password) const = 0;
    /** WARNING: Do not use this unless absolutely necessary */
    EXPORT virtual Action ResyncContext(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
#if OT_CASH
    EXPORT virtual Action SendCash(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        const std::shared_ptr<blind::Purse> purse) const = 0;
#endif  // OT_CASH
    EXPORT virtual Action SendMessage(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        const std::string& message) const = 0;
    EXPORT virtual Action SendPayment(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& recipientNymID,
        std::shared_ptr<const OTPayment>& payment) const = 0;
    EXPORT virtual Action SendTransfer(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& senderAccountID,
        const Identifier& recipientAccountID,
        const Amount amount,
        const std::string& memo) const = 0;
    EXPORT virtual Action TriggerClause(
        const Identifier& localNymID,
        const Identifier& serverID,
        const TransactionNumber transactionNumber,
        const std::string& clause,
        const std::string& parameter) const = 0;
    EXPORT virtual Action UnregisterAccount(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID) const = 0;
    EXPORT virtual Action UnregisterNym(
        const Identifier& localNymID,
        const Identifier& serverID) const = 0;
#if OT_CASH
    EXPORT virtual Action WithdrawCash(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const Amount amount) const = 0;
#endif  // OT_CASH
    EXPORT virtual Action WithdrawVoucher(
        const Identifier& localNymID,
        const Identifier& serverID,
        const Identifier& accountID,
        const Identifier& recipientNymID,
        const Amount amount,
        const std::string& memo) const = 0;

    EXPORT virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    ServerAction& operator=(const ServerAction&) = delete;
    ServerAction& operator=(ServerAction&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
