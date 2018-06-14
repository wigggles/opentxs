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

#include "stdafx.hpp"

#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/ui/BalanceItem.hpp"

#include "AccountActivityParent.hpp"
#include "Row.hpp"

#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

#include "BalanceItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::BalanceItem>;

#define OT_METHOD "opentxs::ui::implementation::BalanceItem::"

namespace opentxs
{
ui::BalanceItem* Factory::BalanceItem(
    const ui::implementation::AccountActivityParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const proto::PaymentWorkflow& workflow,
    const proto::PaymentEvent& event,
    const Identifier& nymID,
    const Identifier& accountID)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {

            return new ui::implementation::ChequeBalanceItem(
                parent,
                zmq,
                publisher,
                contact,
                sync,
                wallet,
                workflow,
                event,
                nymID,
                accountID);
        }
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            otErr << "Factory::" << __FUNCTION__ << ": Unhandled workflow type"
                  << std::endl;
        }
    }

    return nullptr;
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
BalanceItem::BalanceItem(
    const ui::implementation::AccountActivityParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const proto::PaymentWorkflow& workflow,
    const proto::PaymentEvent& event,
    const Identifier& nymID,
    const Identifier& accountID)
    : BalanceItemRow(
          parent,
          zmq,
          publisher,
          contact,
          {Identifier::Factory(workflow.id()), event.type()},
          true)
    , sync_(sync)
    , wallet_(wallet)
    , nym_id_(Identifier::Factory(nymID))
    , type_(extract_type(workflow))
    , text_("")
    , time_()
    , startup_(nullptr)
    , account_id_(Identifier::Factory(accountID))
{
}

ChequeBalanceItem::ChequeBalanceItem(
    const AccountActivityParent& parent,
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const proto::PaymentWorkflow& workflow,
    const proto::PaymentEvent& event,
    const Identifier& nymID,
    const Identifier& accountID)
    : BalanceItem(
          parent,
          zmq,
          publisher,
          contact,
          sync,
          wallet,
          workflow,
          event,
          nymID,
          accountID)
    , cheque_(nullptr)
{
    startup_.reset(
        new std::thread(&ChequeBalanceItem::startup, this, workflow, event));

    OT_ASSERT(startup_)
}

StorageBox BalanceItem::extract_type(const proto::PaymentWorkflow& workflow)
{
    switch (workflow.type()) {
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE: {

            return StorageBox::OUTGOINGCHEQUE;
        }
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE: {

            return StorageBox::INCOMINGCHEQUE;
        }
        case proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE:
        case proto::PAYMENTWORKFLOWTYPE_ERROR:
        default: {

            return StorageBox::UNKNOWN;
        }
    }
}

std::string BalanceItem::Text() const
{
    sLock lock(shared_lock_);

    return text_;
}

std::chrono::system_clock::time_point BalanceItem::Timestamp() const
{
    sLock lock(shared_lock_);

    return time_;
}

BalanceItem::~BalanceItem()
{
    if (startup_ && startup_->joinable()) {
        startup_->join();
        startup_.reset();
    }
}

opentxs::Amount ChequeBalanceItem::Amount() const { return effective_amount(); }

std::string ChequeBalanceItem::DisplayAmount() const
{
    sLock lock(shared_lock_);

    if (cheque_ && get_contract()) {
        const auto amount = effective_amount();
        std::string output{};
        const auto formatted =
            contract_->FormatAmountLocale(amount, output, ",", ".");

        if (formatted) { return output; }

        return std::to_string(amount);
    }

    return std::to_string(0);
}

opentxs::Amount ChequeBalanceItem::effective_amount() const
{
    sLock lock(shared_lock_);
    auto amount{0};
    opentxs::Amount sign{0};

    if (cheque_) { amount = cheque_->GetAmount(); }

    switch (type_) {
        case StorageBox::OUTGOINGCHEQUE: {
            sign = -1;
        } break;
        case StorageBox::INCOMINGCHEQUE: {
            sign = 1;
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
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    return amount * sign;
}

std::string ChequeBalanceItem::get_contact_name(const Identifier& nymID) const
{
    if (nymID.empty()) { return {}; }

    std::string output{nymID.str()};
    const auto contactID = contact_.ContactID(nymID);

    if (false == contactID->empty()) {
        output = contact_.ContactName(contactID);
    }

    return output;
}

bool ChequeBalanceItem::get_contract() const
{
    if (contract_) { return true; }

    eLock lock(shared_lock_);
    const auto& contractID = cheque_->GetInstrumentDefinitionID();
    contract_ = wallet_.UnitDefinition(contractID);

    if (contract_) { return true; }

    sync_.ScheduleDownloadContract(
        nym_id_, sync_.IntroductionServer(), contractID);

    return false;
}

std::string ChequeBalanceItem::Memo() const
{
    sLock lock(shared_lock_);

    if (cheque_) { return cheque_->GetMemo().Get(); }

    return {};
}

void ChequeBalanceItem::startup(
    const proto::PaymentWorkflow workflow,
    const proto::PaymentEvent event)
{
    eLock lock(shared_lock_);
    cheque_ = api::client::Workflow::InstantiateCheque(workflow).second;

    OT_ASSERT(cheque_)

    lock.unlock();
    get_contract();
    std::string name{""};
    std::string text{""};
    auto number = std::to_string(cheque_->GetTransactionNum());
    auto otherNymID = Identifier::Factory();

    switch (type_) {
        case StorageBox::INCOMINGCHEQUE: {
            otherNymID = Identifier::Factory(cheque_->GetSenderNymID());

            if (otherNymID->empty()) { otherNymID = nym_id_; }

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CONVEY: {
                    text = "Received cheque #" + number + " from " +
                           get_contact_name(otherNymID);
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CREATE:
                case proto::PAYMENTEVENTTYPE_ACCEPT:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Invalid event state" << std::endl;

                    OT_FAIL
                }
            }
        } break;
        case StorageBox::OUTGOINGCHEQUE: {
            otherNymID = Identifier::Factory(cheque_->GetRecipientNymID());

            switch (event.type()) {
                case proto::PAYMENTEVENTTYPE_CREATE: {
                    text = "Wrote cheque #" + number;

                    if (false == otherNymID->empty()) {
                        text += " for " + get_contact_name(otherNymID);
                    }
                } break;
                case proto::PAYMENTEVENTTYPE_ACCEPT: {
                    text = "Cheque #" + number + " cleared";
                } break;
                case proto::PAYMENTEVENTTYPE_ERROR:
                case proto::PAYMENTEVENTTYPE_CONVEY:
                case proto::PAYMENTEVENTTYPE_CANCEL:
                case proto::PAYMENTEVENTTYPE_COMPLETE:
                default: {
                    otErr << OT_METHOD << __FUNCTION__
                          << ": Invalid event state" << std::endl;

                    OT_FAIL
                }
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
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Invalid item type"
                  << std::endl;

            OT_FAIL
        }
    }

    lock.lock();
    text_ = text;
    time_ = std::chrono::system_clock::from_time_t(event.time());
}

void ChequeBalanceItem::Update(
    const proto::PaymentWorkflow& workflow,
    const proto::PaymentEvent& event)
{
    startup(workflow, event);
}
}  // namespace opentxs::ui::implementation
