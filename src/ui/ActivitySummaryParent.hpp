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

#ifndef OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP
#define OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP

#include "Internal.hpp"

#include <chrono>
#include <tuple>
#include <string>

namespace opentxs::ui::implementation
{
using ActivitySummaryID = OTIdentifier;
using ActivitySummarySortKey =
    std::pair<std::chrono::system_clock::time_point, std::string>;

class ActivitySummaryParent
{
public:
    virtual bool last(const ActivitySummaryID& id) const = 0;
    virtual void reindex_item(
        const ActivitySummaryID& id,
        const ActivitySummarySortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ActivitySummaryParent() = default;

protected:
    ActivitySummaryParent() = default;
    ActivitySummaryParent(const ActivitySummaryParent&) = delete;
    ActivitySummaryParent(ActivitySummaryParent&&) = delete;
    ActivitySummaryParent& operator=(const ActivitySummaryParent&) = delete;
    ActivitySummaryParent& operator=(ActivitySummaryParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP
