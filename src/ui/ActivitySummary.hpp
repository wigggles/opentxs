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
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"

#include "List.hpp"

#include <map>
#include <string>
#include <tuple>

namespace opentxs::ui::implementation
{
using ActivitySummaryPimpl = OTUIActivitySummaryItem;
using ActivitySummaryID = OTIdentifier;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;
using ActivitySummaryInner = std::map<ActivitySummaryID, ActivitySummaryPimpl>;
using ActivitySummaryOuter =
    std::map<ActivitySummarySortKey, ActivitySummaryInner>;
using ActivitySummaryReverse =
    std::map<ActivitySummaryID, ActivitySummarySortKey>;
using ActivitySummaryType = List<
    opentxs::ui::ActivitySummary,
    opentxs::ui::ActivitySummaryItem,
    ActivitySummaryID,
    ActivitySummaryPimpl,
    ActivitySummaryInner,
    ActivitySummarySortKey,
    ActivitySummaryOuter,
    ActivitySummaryOuter::const_reverse_iterator,
    ActivitySummaryReverse>;

class ActivitySummary : virtual public ActivitySummaryType
{
public:
    ~ActivitySummary() = default;

private:
    friend api::implementation::UI;

    const api::Activity& activity_;
    const Flag& running_;
    OTZMQListenCallback activity_subscriber_callback_;
    OTZMQSubscribeSocket activity_subscriber_;

    ActivitySummaryID blank_id() const override;
    ActivitySummary* clone() const override { return nullptr; }
    void construct_item(
        const ActivitySummaryID& id,
        const ActivitySummarySortKey& index,
        void* custom = nullptr) const override;
    ActivitySummaryOuter::const_reverse_iterator outer_first() const override;
    ActivitySummaryOuter::const_reverse_iterator outer_end() const override;

    void process_thread(const std::string& threadID);
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
