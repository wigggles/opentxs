// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"

#include "internal/api/client/Client.hpp"
#include "internal/ui/UI.hpp"
#include "Row.hpp"

#include <memory>
#include <thread>

#include "PaymentItem.hpp"

#define OT_METHOD "opentxs::ui::implementation::PaymentItem::"

namespace opentxs
{
ui::implementation::ActivityThreadRowInternal* Factory::PaymentItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::PaymentItem(
        parent, api, publisher, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PaymentItem::PaymentItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    const CustomData& custom) noexcept
    : ActivityThreadItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          true,
          false)
    , display_amount_()
    , memo_()
    , amount_(0)
    , load_(nullptr)
    , payment_()
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())

    switch (box_) {
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            load_.reset(new std::thread(&PaymentItem::load, this));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::BLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    OT_ASSERT(load_)
}

opentxs::Amount PaymentItem::Amount() const noexcept
{
    sLock lock(shared_lock_);

    return amount_;
}

bool PaymentItem::Deposit() const noexcept
{
    switch (box_) {
        case StorageBox::INCOMINGCHEQUE: {
        } break;
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::BLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {

            return false;
        }
    }

    sLock lock(shared_lock_);

    if (false == bool(payment_)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Payment not loaded.").Flush();

        return false;
    }

    auto task = api_.OTX().DepositPayment(nym_id_, payment_);

    if (0 == task.first) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to queue deposit.")
            .Flush();

        return false;
    }

    return true;
}

std::string PaymentItem::DisplayAmount() const noexcept
{
    sLock lock(shared_lock_);

    return display_amount_;
}

void PaymentItem::load() noexcept
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    std::shared_ptr<const std::string> text{nullptr};
    std::string displayAmount{};
    std::string memo{};
    opentxs::Amount amount{0};
    std::shared_ptr<OTPayment> payment{};

    switch (box_) {
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            text = api_.Activity().PaymentText(
                nym_id_, item_id_.str(), account_id_.str());
            const auto [cheque, contract] = api_.Activity().Cheque(
                nym_id_, item_id_.str(), account_id_.str());

            if (cheque) {
                memo = cheque->GetMemo().Get();
                amount = cheque->GetAmount();

                if (0 < contract->Version()) {
                    contract->FormatAmountLocale(
                        amount, displayAmount, ",", ".");
                }

                payment = api_.Factory().Payment(String::Factory(*cheque));

                OT_ASSERT(payment);

                payment->SetTempValues(reason);
            }
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX:
        case StorageBox::BLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }

    OT_ASSERT(text)

    eLock lock(shared_lock_);
    text_ = *text;
    display_amount_ = displayAmount;
    memo_ = memo;
    amount_ = amount;
    loading_->Off();
    pending_->Off();
    payment_ = payment;
    UpdateNotify();
}

std::string PaymentItem::Memo() const noexcept
{
    sLock lock(shared_lock_);

    return memo_;
}

PaymentItem::~PaymentItem()
{
    if (load_ && load_->joinable()) { load_->join(); }
}
}  // namespace opentxs::ui::implementation
