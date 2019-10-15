// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/ServerAction.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include "client/OTAPI_Func.hpp"
#include "internal/api/client/Client.hpp"

#include "ServerAction.hpp"

//#define OT_METHOD "opentxs::api::client::implementation::ServerAction::"

namespace opentxs
{
api::client::ServerAction* Factory::ServerAction(
    const api::client::internal::Manager& api,
    const ContextLockCallback& lockCallback)
{
    return new api::client::implementation::ServerAction(api, lockCallback);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
ServerAction::ServerAction(
    const api::client::internal::Manager& api,
    const ContextLockCallback& lockCallback)
    : api_(api)
    , lock_callback_(lockCallback)
{
    // WARNING: do not access api_.Wallet() during construction
}

ServerAction::Action ServerAction::ActivateSmartContract(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID,
    const std::string& agentName,
    std::unique_ptr<OTSmartContract>& contract) const
{
    return Action(new OTAPI_Func(reason,
        ACTIVATE_SMART_CONTRACT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        agentName,
        contract*/));
}

ServerAction::Action ServerAction::AdjustUsageCredits(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const Amount adjustment) const
{
    return Action(new OTAPI_Func(reason,
        ADJUST_USAGE_CREDITS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        targetNymID,
        adjustment*/));
}

ServerAction::Action ServerAction::CancelPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const
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
    return Action(new OTAPI_Func(reason,
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        plan->GetRecipientAcctID(),
        plan*/));
}

ServerAction::Action ServerAction::CreateMarketOffer(
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
    const Amount activationPrice) const
{
    auto notaryID = identifier::Server::Factory();
    auto nymID = identifier::Nym::Factory();
    const auto assetAccount = api_.Wallet().Account(assetAccountID, reason);

    if (assetAccount) {
        nymID = assetAccount.get().GetNymID();
        notaryID = assetAccount.get().GetPurportedNotaryID();
    }

    return Action(new OTAPI_Func(reason,
        CREATE_MARKET_OFFER,
        lock_callback_({nymID->str(), notaryID->str()}),
        api_,
        nymID,
        notaryID/*,
        assetAccountID,
        currencyAccountID,
        scale,
        increment,
        quantity,
        price,
        selling,
        lifetime.count(),
        activationPrice,
        stopSign*/));
}

ServerAction::Action ServerAction::DepositPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    std::unique_ptr<OTPaymentPlan>& plan) const
{
    return Action(new OTAPI_Func(reason,
        DEPOSIT_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        plan->GetSenderAcctID(),
        plan*/));
}

ServerAction::Action ServerAction::DownloadMarketList(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID) const
{
    return Action(new OTAPI_Func(
        reason,
        GET_MARKET_LIST,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

ServerAction::Action ServerAction::DownloadMarketOffers(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& marketID,
    const Amount depth) const
{
    return Action(new OTAPI_Func(reason,
        GET_MARKET_OFFERS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        marketID,
        depth*/));
}

ServerAction::Action ServerAction::DownloadMarketRecentTrades(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& marketID) const
{
    return Action(new OTAPI_Func(reason,
        GET_MARKET_RECENT_TRADES,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        marketID*/));
}

ServerAction::Action ServerAction::DownloadNymMarketOffers(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID) const
{
    return Action(new OTAPI_Func(
        reason,
        GET_NYM_MARKET_OFFERS,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

ServerAction::Action ServerAction::ExchangeBasketCurrency(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Identifier& accountID,
    const Identifier& basketID,
    const bool direction) const
{
    return Action(new OTAPI_Func(reason,
        EXCHANGE_BASKET,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        instrumentDefinitionID,
        basketID,
        accountID,
        direction,
        api_.OTAPI().GetBasketMemberCount(basketID)*/));
}

ServerAction::Action ServerAction::IssueBasketCurrency(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const proto::UnitDefinition& basket,
    const std::string& label) const
{
    return Action(new OTAPI_Func(reason,
        ISSUE_BASKET,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        basket,
        label*/));
}

ServerAction::Action ServerAction::KillMarketOffer(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    return Action(new OTAPI_Func(reason,
        KILL_MARKET_OFFER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        number*/));
}

ServerAction::Action ServerAction::KillPaymentPlan(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID,
    const TransactionNumber number) const
{
    return Action(new OTAPI_Func(reason,
        KILL_PAYMENT_PLAN,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        number*/));
}

ServerAction::Action ServerAction::PayDividend(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Identifier& accountID,
    const std::string& memo,
    const Amount amountPerShare) const
{
    return Action(new OTAPI_Func(reason,
        PAY_DIVIDEND,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        instrumentDefinitionID,
        amountPerShare,
        memo*/));
}

ServerAction::Action ServerAction::TriggerClause(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const TransactionNumber transactionNumber,
    const std::string& clause,
    const std::string& parameter) const
{
    return Action(new OTAPI_Func(reason,
        TRIGGER_CLAUSE,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        transactionNumber,
        clause,
        parameter*/));
}

ServerAction::Action ServerAction::UnregisterAccount(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID) const
{
    return Action(new OTAPI_Func(reason,
        DELETE_ASSET_ACCT,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID*/));
}

ServerAction::Action ServerAction::UnregisterNym(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID) const
{
    return Action(new OTAPI_Func(
        reason,
        DELETE_NYM,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID));
}

ServerAction::Action ServerAction::WithdrawVoucher(
    const PasswordPrompt& reason,
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID,
    const identifier::Nym& recipientNymID,
    const Amount amount,
    const std::string& memo) const
{
    return Action(new OTAPI_Func(reason,
        WITHDRAW_VOUCHER,
        lock_callback_({localNymID.str(), serverID.str()}),
        api_,
        localNymID,
        serverID/*,
        accountID,
        recipientNymID,
        amount,
        memo*/));
}
}  // namespace opentxs::api::client::implementation
