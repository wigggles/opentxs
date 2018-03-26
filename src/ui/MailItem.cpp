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

#include "opentxs/api/Activity.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"

#include "MailItem.hpp"

namespace opentxs::ui::implementation
{
MailItem::MailItem(
    const ActivityThread& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const ActivityThreadID& id,
    const Identifier& nymID,
    const api::Activity& activity,
    const std::chrono::system_clock::time_point& time,
    const std::string& text,
    const bool loading,
    const bool pending)
    : ActivityThreadItem(
          parent,
          zmq,
          contact,
          id,
          nymID,
          activity,
          time,
          text,
          loading,
          pending)
    , load_(nullptr){OT_ASSERT(false == nym_id_.empty())
                         OT_ASSERT(false == item_id_.empty())}

    MailItem::MailItem(
        const ActivityThread& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time)
    : MailItem(parent, zmq, contact, id, nymID, activity, time, "", true, false)
{
    switch (box_) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            load_.reset(new std::thread(&MailItem::load, this));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    OT_ASSERT(load_)
}

void MailItem::load()
{
    std::shared_ptr<const std::string> text{nullptr};

    switch (box_) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            text = activity_.MailText(nym_id_, item_id_, box_);
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }

    OT_ASSERT(text)

    eLock lock(shared_lock_);
    text_ = *text;
    loading_->Off();
    pending_->Off();
    UpdateNotify();
}

MailItem::~MailItem()
{
    if (load_ && load_->joinable()) {
        load_->join();
    }
}
}  // namespace opentxs::ui::implementation
