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

#ifndef OPENTXS_UI_ACTIVITYTHREAD_HPP
#define OPENTXS_UI_ACTIVITYTHREAD_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/Widget.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(UIActivityThread) opentxs::ui::ActivityThread;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ActivityThread : virtual public Widget
{
public:
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual const ActivityThreadItem& First() const = 0;
    EXPORT virtual const ActivityThreadItem& Next() const = 0;
    EXPORT virtual std::string GetDraft() const = 0;
    EXPORT virtual std::string Participants() const = 0;
    EXPORT virtual bool SendDraft() const = 0;
    EXPORT virtual bool SetDraft(const std::string& draft) const = 0;
    EXPORT virtual std::string ThreadID() const = 0;

    EXPORT virtual ~ActivityThread() = default;

protected:
    ActivityThread() = default;

private:
    ActivityThread(const ActivityThread&) = delete;
    ActivityThread(ActivityThread&&) = delete;
    ActivityThread& operator=(const ActivityThread&) = delete;
    ActivityThread& operator=(ActivityThread&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_ACTIVITYTHREAD_HPP
