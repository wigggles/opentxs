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

#ifndef OPENTXS_UI_ACTIVITYTHREADITEMBLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACTIVITYTHREADITEMBLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class ActivityThreadItemBlank : virtual public ui::ActivityThreadItem
{
public:
    bool Last() const override { return true; }
    bool Loading() const override { return false; }
    bool MarkRead() const override { return false; }
    bool Pending() const override { return false; }
    std::string Text() const override { return {}; }
    std::int64_t Time() const override { return {}; }
    std::chrono::system_clock::time_point Timestamp() const override
    {
        return {};
    }
    StorageBox Type() const override { return StorageBox::UNKNOWN; }
    bool Valid() const override { return false; }

    ~ActivityThreadItemBlank() = default;

private:
    friend ActivityThread;

    ActivityThreadItemBlank() = default;
    ActivityThreadItemBlank(const ActivityThreadItemBlank&) = delete;
    ActivityThreadItemBlank(ActivityThreadItemBlank&&) = delete;
    ActivityThreadItemBlank& operator=(const ActivityThreadItemBlank&) = delete;
    ActivityThreadItemBlank& operator=(ActivityThreadItemBlank&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITYTHREADITEMBLANK_IMPLEMENTATION_HPP
