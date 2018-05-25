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

#ifndef OPENTXS_UI_ACTIVITY_THREAD_PARENT_HPP
#define OPENTXS_UI_ACTIVITY_THREAD_PARENT_HPP

#include "Internal.hpp"

#include <chrono>
#include <cstdint>
#include <tuple>

namespace opentxs::ui::implementation
{
using ActivityThreadSortKey =
    std::pair<std::chrono::system_clock::time_point, std::uint64_t>;

class ActivityThreadParent
{
public:
    virtual bool last(const ActivityThreadID& id) const = 0;
    virtual void reindex_item(
        const ActivityThreadID& id,
        const ActivityThreadSortKey& newIndex) const = 0;
    virtual std::string ThreadID() const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ActivityThreadParent() = default;

protected:
    ActivityThreadParent() = default;
    ActivityThreadParent(const ActivityThreadParent&) = delete;
    ActivityThreadParent(ActivityThreadParent&&) = delete;
    ActivityThreadParent& operator=(const ActivityThreadParent&) = delete;
    ActivityThreadParent& operator=(ActivityThreadParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITY_THREAD_PARENT_HPP
