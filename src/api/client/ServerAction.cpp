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

#include "opentxs/stdafx.hpp"

#include "opentxs/api/client/implementation/ServerAction.hpp"

#include "opentxs/cash/Purse.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OTAPI_Func.hpp"
#include "opentxs/client/Utility.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"

#define OT_METHOD "opentxs::api::client::implementation::ServerAction::"

namespace opentxs::api::client::implementation
{
ServerAction::ServerAction(
    std::recursive_mutex& apiLock,
    const OT_API& otapi,
    const OTAPI_Exec& exec,
    const api::client::Wallet& wallet)
    : api_lock_(apiLock)
    , otapi_(otapi)
    , exec_(exec)
    , wallet_(wallet)
{
}

ServerAction::Action ServerAction::AcknowledgeBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& instructions) const
{
    const std::string recipient{String(recipientID).Get()};
    const std::string request{String(requestID).Get()};

    return Action(new OTAPI_Func(
        ACKNOWLEDGE_BAILMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        request,
        instructions));
}

ServerAction::Action ServerAction::AcknowledgeConnection(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key) const
{
    const std::string recipient{String(recipientID).Get()};
    const std::string request{String(requestID).Get()};

    return Action(new OTAPI_Func(
        ACKNOWLEDGE_CONNECTION,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        request,
        url,
        login,
        password,
        key,
        ack));
}

ServerAction::Action ServerAction::AcknowledgeNotice(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const bool ack) const
{
    const std::string recipient{String(recipientID).Get()};
    const std::string request{String(requestID).Get()};

    return Action(new OTAPI_Func(
        ACKNOWLEDGE_NOTICE,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        request,
        ack));
}

ServerAction::Action ServerAction::AcknowledgeOutbailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientID,
    const Identifier& requestID,
    const std::string& details) const
{
    const std::string recipient{String(recipientID).Get()};
    const std::string request{String(requestID).Get()};

    return Action(new OTAPI_Func(
        ACKNOWLEDGE_OUTBAILMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        request,
        details));
}

ServerAction::Action ServerAction::ActivateSmartContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const String& agentName,
    const OTSmartContract& contract) const
{
    const std::string account{String(accountID).Get()};
    const std::string serialized{String(contract).Get()};

    return Action(new OTAPI_Func(
        ACTIVATE_SMART_CONTRACT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        agentName.Get(),
        serialized));
}

ServerAction::Action ServerAction::AddServerClaim(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const std::string& value,
    bool primary) const
{
    return Action(new OTAPI_Func(
        SERVER_ADD_CLAIM,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        primary,
        std::to_string(static_cast<std::uint32_t>(section)),
        std::to_string(static_cast<std::uint32_t>(type)),
        value));
}

ServerAction::Action ServerAction::AdjustUsageCredits(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Amount adjustment) const
{
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        ADJUST_USAGE_CREDITS,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target,
        std::to_string(adjustment)));
}

ServerAction::Action ServerAction::CancelPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    const OTPaymentPlan& plan) const
{
    // NOTE: Normally the SENDER (PAYER) is the one who deposits a payment plan.
    // But in this case, the RECIPIENT (PAYEE) deposits it -- which means
    // "Please cancel this plan." It SHOULD fail, since it's only been signed
    // by the recipient, and not the sender. And that failure is what burns
    // the transaction number on the plan, so that it can no longer be used.
    //
    // So how do we know the difference between an ACTUAL "failure" versus a
    // purposeful "failure" ? Because if the failure comes from cancelling the
    // plan, the server reply transaction will have IsCancelled() set to true.
    const auto accountID = String(plan.GetRecipientAcctID()).Get();
    const std::string serialized{String(plan).Get()};

    return Action(new OTAPI_Func(
        DEPOSIT_PAYMENT_PLAN,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        accountID,
        serialized));
}

ServerAction::Action ServerAction::CreateMarketOffer(
    const Identifier& assetAccountID,
    const Identifier& currencyAccountID,
    const Amount scale,
    const Amount increment,
    const Amount quantity,
    const Amount price,
    const bool selling,
    const std::chrono::seconds lifetime,
    const std::string& stopSign,
    const Amount activationPrice) const
{
    Identifier notaryID{};
    Identifier nymID{};
    const auto assetAccount = otapi_.GetAccount(assetAccountID, __FUNCTION__);

    if (nullptr != assetAccount) {
        nymID = assetAccount->GetNymID();
        notaryID = assetAccount->GetPurportedNotaryID();
    }
    const std::string target{String(assetAccountID).Get()};
    const std::string source{String(currencyAccountID).Get()};

    return Action(new OTAPI_Func(
        CREATE_MARKET_OFFER,
        wallet_,
        nymID,
        notaryID,
        exec_,
        otapi_,
        target,
        source,
        std::to_string(scale),
        std::to_string(increment),
        std::to_string(quantity),
        std::to_string(price),
        selling,
        lifetime.count(),
        activationPrice,
        stopSign));
}

#if OT_CASH
ServerAction::Action ServerAction::DepositCashPurse(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Purse& purse) const
{
    const std::string account{String(accountID).Get()};
    const std::string serialized{String(purse).Get()};

    return Action(new OTAPI_Func(
        DEPOSIT_CASH,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        serialized));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::DepositCheque(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Cheque& cheque) const
{
    const std::string account{String(accountID).Get()};
    const std::string serialized{String(cheque).Get()};

    return Action(new OTAPI_Func(
        DEPOSIT_CHEQUE,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        serialized));
}

ServerAction::Action ServerAction::DepositPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    const OTPaymentPlan& plan) const
{
    const std::string accountID{String(plan.GetSenderAcctID()).Get()};
    const std::string serialized{String(plan).Get()};

    return Action(new OTAPI_Func(
        DEPOSIT_PAYMENT_PLAN,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        accountID,
        serialized));
}

bool ServerAction::DownloadAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    rLock lock(api_lock_);
    auto context = wallet_.mutable_ServerContext(localNymID, serverID);
    Utility MsgUtil(context.It(), otapi_);
    const auto output = MsgUtil.getIntermediaryFiles(
        String(serverID).Get(),
        String(localNymID).Get(),
        String(accountID).Get(),
        false);

    return output;
}

ServerAction::Action ServerAction::DownloadBoxReceipt(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const RemoteBoxType box,
    const TransactionNumber item) const
{
    const std::string account{String(accountID).Get()};

    return Action(new OTAPI_Func(
        GET_BOX_RECEIPT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        std::to_string(static_cast<std::int8_t>(box)),
        std::to_string(item)));
}

ServerAction::Action ServerAction::DownloadContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& contractID) const
{
    const std::string contract{String(contractID).Get()};

    return Action(new OTAPI_Func(
        GET_CONTRACT, wallet_, localNymID, serverID, exec_, otapi_, contract));
}

ServerAction::Action ServerAction::DownloadMarketList(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        GET_MARKET_LIST, wallet_, localNymID, serverID, exec_, otapi_));
}

ServerAction::Action ServerAction::DownloadMarketOffers(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& marketID,
    const Amount depth) const
{
    const std::string market{String(marketID).Get()};

    return Action(new OTAPI_Func(
        GET_MARKET_OFFERS,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        market,
        depth));
}

ServerAction::Action ServerAction::DownloadMarketRecentTrades(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& marketID) const
{
    const std::string market{String(marketID).Get()};

    return Action(new OTAPI_Func(
        GET_MARKET_RECENT_TRADES,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        market));
}

#if OT_CASH
ServerAction::Action ServerAction::DownloadMint(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID) const
{
    const std::string unit{String(instrumentDefinitionID).Get()};

    return Action(new OTAPI_Func(
        GET_MINT, wallet_, localNymID, serverID, exec_, otapi_, unit));
}
#endif  // OT_CASH

bool ServerAction::GetTransactionNumbers(
    const Identifier& localNymID,
    const Identifier& serverID,
    const std::size_t quantity) const
{
    rLock lock(api_lock_);
    auto context = wallet_.mutable_ServerContext(localNymID, serverID);
    Utility MsgUtil(context.It(), otapi_);
    auto available = context.It().AvailableNumbers();

    if (available < quantity) {
        otErr << OT_METHOD << __FUNCTION__ << ": Asking server for more numbers"
              << std::endl;
        MsgUtil.getTransactionNumbers(
            String(serverID).Get(), String(localNymID).Get(), true);
        bool msgWasSent{false};
        const auto download = MsgUtil.getAndProcessNymbox_4(
            String(serverID).Get(),
            String(localNymID).Get(),
            msgWasSent,
            false);

        if (0 > download) {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to retrieve nym."
                  << std::endl;

            return false;
        }

        // Try again.
        available = context.It().AvailableNumbers();

        if (available < quantity) {
            otErr << OT_METHOD << __FUNCTION__ << ": Failed to obtain "
                  << quantity << " numbers" << std::endl;

            return false;
        }
    }

    return true;
}

ServerAction::Action ServerAction::DownloadNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        CHECK_NYM, wallet_, localNymID, serverID, exec_, otapi_, target));
}

ServerAction::Action ServerAction::DownloadNymMarketOffers(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        GET_NYM_MARKET_OFFERS, wallet_, localNymID, serverID, exec_, otapi_));
}

ServerAction::Action ServerAction::ExchangeBasketCurrency(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Identifier& accountID,
    const Identifier& basketID,
    const bool direction) const
{
    const std::string account{String(accountID).Get()};
    const std::string unit{String(instrumentDefinitionID).Get()};
    const std::string basket{String(basketID).Get()};

    return Action(new OTAPI_Func(
        EXCHANGE_BASKET,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        unit,
        basket,
        account,
        direction,
        otapi_.GetBasketMemberCount(basketID)));
}

#if OT_CASH
ServerAction::Action ServerAction::ExchangeCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Purse& purse) const
{
    const std::string unit{String(instrumentDefinitionID).Get()};
    const std::string serialized{String(purse).Get()};

    return Action(new OTAPI_Func(
        EXCHANGE_CASH,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        unit,
        serialized));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::InitiateBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID) const
{
    const std::string unit{String(instrumentDefinitionID).Get()};
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        INITIATE_BAILMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target,
        unit));
}

ServerAction::Action ServerAction::InitiateOutbailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID,
    const Amount amount,
    const std::string& message) const
{
    const std::string unit{String(instrumentDefinitionID).Get()};
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        INITIATE_OUTBAILMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target,
        unit,
        amount,
        message));
}

ServerAction::Action ServerAction::InitiateRequestConnection(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const proto::ConnectionInfoType& type) const
{
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        REQUEST_CONNECTION,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target,
        static_cast<std::int64_t>(type)));
}

ServerAction::Action ServerAction::InitiateStoreSecret(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const proto::SecretType& type,
    const std::string& primary,
    const std::string& secondary) const
{
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        STORE_SECRET,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target,
        primary,
        secondary,
        static_cast<std::int64_t>(type)));
}

ServerAction::Action ServerAction::IssueBasketCurrency(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::UnitDefinition& basket) const
{
    const std::string temporary =
        proto::ProtoAsArmored(basket, "BASKET CONTRACT").Get();

    return Action(new OTAPI_Func(
        ISSUE_BASKET, wallet_, localNymID, serverID, exec_, otapi_, temporary));
}

ServerAction::Action ServerAction::IssueUnitDefinition(
    const Identifier& localNymID,
    const Identifier& serverID,
    const proto::UnitDefinition& contract) const
{
    const std::string temporary =
        proto::ProtoAsArmored(contract, "UNIT DEFINITION").Get();

    return Action(new OTAPI_Func(
        ISSUE_ASSET_TYPE,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        temporary));
}

ServerAction::Action ServerAction::KillMarketOffer(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    const std::string account{String(accountID).Get()};

    return Action(new OTAPI_Func(
        KILL_MARKET_OFFER,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        std::to_string(number)));
}

ServerAction::Action ServerAction::KillPaymentPlan(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    const std::string account{String(accountID).Get()};

    return Action(new OTAPI_Func(
        KILL_PAYMENT_PLAN,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        std::to_string(number)));
}

ServerAction::Action ServerAction::NotifyBailment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID,
    const Identifier& instrumentDefinitionID,
    const Identifier& requestID,
    const std::string& txid) const
{
    const std::string recipient{String(targetNymID).Get()};
    const std::string unit{String(instrumentDefinitionID).Get()};
    const std::string request{String(requestID).Get()};

    return Action(new OTAPI_Func(
        NOTIFY_BAILMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        unit,
        txid,
        request));
}

ServerAction::Action ServerAction::PayDividend(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID,
    const Identifier& accountID,
    const std::string& memo,
    const Amount amountPerShare) const
{
    const std::string account{String(accountID).Get()};
    const std::string unit{String(instrumentDefinitionID).Get()};

    return Action(new OTAPI_Func(
        PAY_DIVIDEND,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        unit,
        memo,
        amountPerShare));
}

ServerAction::Action ServerAction::ProcessInbox(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Ledger& ledger) const
{
    const std::string account{String(accountID).Get()};
    const std::string serialized{String(ledger).Get()};

    return Action(new OTAPI_Func(
        PROCESS_INBOX,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        serialized));
}

ServerAction::Action ServerAction::PublishNym(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetNymID) const
{
    const std::string target{String(targetNymID).Get()};

    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_NYM,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target));
}

ServerAction::Action ServerAction::PublishServerContract(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& targetServerID) const
{
    const std::string target{String(targetServerID).Get()};

    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_SERVER,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        target));
}

ServerAction::Action ServerAction::PublishUnitDefinition(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& unitDefinitionID) const
{
    const std::string unit{String(unitDefinitionID).Get()};

    return Action(new OTAPI_Func(
        REGISTER_CONTRACT_UNIT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        unit));
}

ServerAction::Action ServerAction::RegisterAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& instrumentDefinitionID) const
{
    const std::string unit{String(instrumentDefinitionID).Get()};

    return Action(new OTAPI_Func(
        CREATE_ASSET_ACCT, wallet_, localNymID, serverID, exec_, otapi_, unit));
}

ServerAction::Action ServerAction::RegisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        REGISTER_NYM, wallet_, localNymID, serverID, exec_, otapi_));
}

ServerAction::Action ServerAction::RequestAdmin(
    const Identifier& localNymID,
    const Identifier& serverID,
    const std::string& password) const
{
    return Action(new OTAPI_Func(
        REQUEST_ADMIN, wallet_, localNymID, serverID, exec_, otapi_, password));
}

#if OT_CASH
ServerAction::Action ServerAction::SendCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    const Purse& recipientCopy,
    const Purse& senderCopy) const
{
    String pubkey{""};
    const auto nym = wallet_.Nym(recipientNymID);

    if (nym) {
        nym->GetPublicEncrKey().GetPublicKey(pubkey);
    }

    const std::string recipient{String(recipientNymID).Get()};
    const std::string key{String(pubkey).Get()};
    const std::string senderVersion{String(senderCopy).Get()};
    const std::string recipientVersion{String(recipientCopy).Get()};

    return Action(new OTAPI_Func(
        SEND_USER_INSTRUMENT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        key,
        recipientVersion,
        senderVersion));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::SendMessage(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    const std::string& message) const
{
    String pubkey{""};
    const auto nym = wallet_.Nym(recipientNymID);

    if (nym) {
        nym->GetPublicEncrKey().GetPublicKey(pubkey);
    }

    const std::string recipient{String(recipientNymID).Get()};
    const std::string key{String(pubkey).Get()};

    return Action(new OTAPI_Func(
        SEND_USER_MESSAGE,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        recipient,
        key,
        message));
}

ServerAction::Action ServerAction::SendPayment(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& recipientNymID,
    const OTPayment& payment) const
{
    return SendMessage(
        localNymID, serverID, recipientNymID, String(payment).Get());
}

ServerAction::Action ServerAction::SendTransfer(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& senderAccountID,
    const Identifier& recipientAccountID,
    const Amount amount,
    const std::string& memo) const
{
    const std::string source{String(senderAccountID).Get()};
    const std::string target{String(recipientAccountID).Get()};

    return Action(new OTAPI_Func(
        SEND_TRANSFER,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        source,
        target,
        amount,
        memo));
}

ServerAction::Action ServerAction::TriggerClause(
    const Identifier& localNymID,
    const Identifier& serverID,
    const TransactionNumber transactionNumber,
    const std::string& clause,
    const std::string& parameter) const
{
    return Action(new OTAPI_Func(
        TRIGGER_CLAUSE,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        std::to_string(transactionNumber),
        clause,
        parameter));
}

ServerAction::Action ServerAction::UnregisterAccount(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID) const
{
    const std::string account{String(accountID).Get()};

    return Action(new OTAPI_Func(
        DELETE_ASSET_ACCT,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account));
}

ServerAction::Action ServerAction::UnregisterNym(
    const Identifier& localNymID,
    const Identifier& serverID) const
{
    return Action(new OTAPI_Func(
        DELETE_NYM, wallet_, localNymID, serverID, exec_, otapi_));
}

#if OT_CASH
ServerAction::Action ServerAction::WithdrawCash(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Amount amount) const
{
    const std::string account{String(accountID).Get()};

    return Action(new OTAPI_Func(
        WITHDRAW_CASH,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        amount));
}
#endif  // OT_CASH

ServerAction::Action ServerAction::WithdrawVoucher(
    const Identifier& localNymID,
    const Identifier& serverID,
    const Identifier& accountID,
    const Identifier& recipientNymID,
    const Amount amount,
    const std::string& memo) const
{
    const std::string account{String(accountID).Get()};
    const std::string recipient{String(recipientNymID).Get()};

    return Action(new OTAPI_Func(
        WITHDRAW_VOUCHER,
        wallet_,
        localNymID,
        serverID,
        exec_,
        otapi_,
        account,
        recipient,
        memo,
        amount));
}
}  // namespace opentxs::api::implementation
