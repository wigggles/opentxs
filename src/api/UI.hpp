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
 *  fellowtraveler\opentransactions.org
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

#ifndef OPENTXS_API_UI_IMPLEMENTATION_HPP
#define OPENTXS_API_UI_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/UI.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>
#include <tuple>

namespace opentxs::api::implementation
{
class UI : virtual public opentxs::api::UI, Lockable
{
public:
    const ui::ActivitySummary& ActivitySummary(
        const Identifier& nymID) const override;
    const ui::ActivityThread& ActivityThread(
        const Identifier& nymID,
        const Identifier& threadID) const override;
    const ui::ContactList& ContactList(const Identifier& nymID) const override;
    const ui::MessagableList& MessagableList(
        const Identifier& nymID) const override;

    ~UI();

private:
    friend class implementation::Native;
    using ActivitySummaryMap =
        std::map<Identifier, std::unique_ptr<ui::ActivitySummary>>;
    using ActivityThreadID = std::pair<Identifier, Identifier>;
    using ActivityThreadMap =
        std::map<ActivityThreadID, std::unique_ptr<ui::ActivityThread>>;
    using ContactListMap =
        std::map<Identifier, std::unique_ptr<ui::ContactList>>;
    using MessagableListMap =
        std::map<Identifier, std::unique_ptr<ui::MessagableList>>;

    const opentxs::network::zeromq::Context& zmq_;
    const api::Activity& activity_;
    const api::ContactManager& contact_;
    const api::client::Sync& sync_;
    const Flag& running_;
    mutable ActivitySummaryMap activity_summaries_{};
    mutable ContactListMap contact_lists_{};
    mutable MessagableListMap messagable_lists_{};
    mutable ActivityThreadMap activity_threads_{};

    UI(const opentxs::network::zeromq::Context& zmq,
       const api::Activity& activity,
       const api::ContactManager& contact,
       const api::client::Sync& sync,
       const Flag& running);
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::implementation
#endif  // OPENTXS_API_UI_IMPLEMENTATION_HPP
