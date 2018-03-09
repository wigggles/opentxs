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

#include "UI.hpp"

#include "ui/ActivitySummary.hpp"
#include "ui/ContactList.hpp"
#include "ui/MessagableList.hpp"

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs::api::implementation
{
UI::UI(
    const opentxs::network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const Flag& running)
    : zmq_(zmq)
    , activity_(activity)
    , contact_(contact)
    , sync_(sync)
    , running_(running)
    , activity_summaries_()
    , contact_lists_()
    , messagable_lists_()
{
}

const ui::ActivitySummary& UI::ActivitySummary(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = activity_summaries_[nymID];

    if (false == bool(output)) {
        output.reset(new ui::implementation::ActivitySummary(
            zmq_, activity_, contact_, running_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::ContactList& UI::ContactList(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = contact_lists_[nymID];

    if (false == bool(output)) {
        output.reset(
            new ui::implementation::ContactList(zmq_, contact_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::MessagableList& UI::MessagableList(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = messagable_lists_[nymID];

    if (false == bool(output)) {
        output.reset(new ui::implementation::MessagableList(
            zmq_, contact_, sync_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

UI::~UI() {}
}  // namespace opentxs::api::implementation
