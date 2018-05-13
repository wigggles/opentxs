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
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"

#include "ActivityThreadParent.hpp"
#include "Row.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
ActivityThreadItem::ActivityThreadItem(
    const ActivityThreadParent& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const ActivityThreadID& id,
    const Identifier& nymID,
    const api::Activity& activity,
    const std::chrono::system_clock::time_point& time,
    const std::string& text,
    const bool loading,
    const bool pending)
    : ActivityThreadItemType(parent, zmq, contact, id, true)
    , nym_id_(nymID)
    , activity_(activity)
    , time_(time)
    , item_id_(std::get<0>(id_))
    , box_(std::get<1>(id_))
    , account_id_(std::get<2>(id_))
    , text_(text)
    , loading_(Flag::Factory(loading))
    , pending_(Flag::Factory(pending))
{
}

bool ActivityThreadItem::Loading() const { return loading_.get(); }

bool ActivityThreadItem::MarkRead() const
{
    return activity_.MarkRead(
        nym_id_, Identifier(parent_.ThreadID()), item_id_);
}

bool ActivityThreadItem::Pending() const { return pending_.get(); }

std::string ActivityThreadItem::Text() const
{
    sLock lock(shared_lock_);

    return text_;
}

std::chrono::system_clock::time_point ActivityThreadItem::Timestamp() const
{
    return time_;
}

StorageBox ActivityThreadItem::Type() const { return box_; }
}  // namespace opentxs::ui::implementation
