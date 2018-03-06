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

#ifndef OPENTXS_UI_ACTIVITYSUMMARY_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACTIVITYSUMMARY_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"

#include <map>
#include <string>
#include <thread>
#include <tuple>

namespace opentxs::ui::implementation
{
class ActivitySummary : virtual public ui::ActivitySummary, Lockable
{
public:
    const opentxs::ui::ActivitySummaryItem& First() const override;
    const opentxs::ui::ActivitySummaryItem& Next() const override;

    ~ActivitySummary();

private:
    friend api::implementation::UI;
    friend ActivitySummaryItem;
    /** Thread ID, Thread */
    using Item = std::map<Identifier, OTUIActivitySummaryItem>;
    /** Last Activity, Display name */
    using ItemIndex =
        std::pair<std::chrono::system_clock::time_point, std::string>;
    using ItemMap = std::map<ItemIndex, Item>;
    /** ThreadID, display name*/
    using NameMap = std::map<Identifier, ItemIndex>;

    const network::zeromq::Context& zmq_;
    const api::Activity& activity_;
    const api::ContactManager& contact_manager_;
    const Flag& running_;
    const Identifier owner_nym_id_;
    mutable Identifier last_thread_id_;
    const std::unique_ptr<opentxs::ui::ActivitySummaryItem> blank_p_;
    const opentxs::ui::ActivitySummaryItem& blank_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    mutable OTFlag startup_complete_;
    mutable ItemMap items_;
    mutable NameMap names_;
    mutable ItemMap::const_reverse_iterator outer_;
    mutable Item::const_iterator inner_;
    std::unique_ptr<std::thread> startup_{nullptr};
    OTZMQListenCallback activity_subscriber_callback_;
    OTZMQSubscribeSocket activity_subscriber_;

    const opentxs::ui::ActivitySummaryItem& current(const Lock& lock) const;
    const opentxs::ui::ActivitySummaryItem& first(const Lock& lock) const;
    bool first_valid_item(const Lock& lock) const;
    void increment_inner(const Lock& lock) const;
    bool increment_outer(const Lock& lock) const;
    // Only used by ActivitySummaryItem
    bool last(const Identifier& id) const;
    const opentxs::ui::ActivitySummaryItem& next(const Lock& lock) const;
    // Only used by ActivitySummaryItem
    void reindex_item(const Identifier& threadID, const ItemIndex& newIndex)
        const;
    void reindex_item(
        const Lock& lock,
        const Identifier& threadID,
        const ItemIndex& oldIndex,
        const ItemIndex& newIndex) const;

    void add_item(const std::string& threadID);
    void add_item(
        const Identifier& threadID,
        const std::string& name,
        const std::chrono::system_clock::time_point& time);
    void process_thread(const network::zeromq::Message& message);
    void startup();

    ActivitySummary(
        const network::zeromq::Context& zmq,
        const api::Activity& activity,
        const api::ContactManager& contact,
        const Flag& running,
        const Identifier& nymID);
    ActivitySummary() = delete;
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    ActivitySummary& operator=(const ActivitySummary&) = delete;
    ActivitySummary& operator=(ActivitySummary&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITYSUMMARY_IMPLEMENTATION_HPP
