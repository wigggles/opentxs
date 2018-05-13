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

#ifndef OPENTXS_UI_ACTIVITYTHREADITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACTIVITYTHREADITEM_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivityThreadItemType =
    Row<opentxs::ui::ActivityThreadItem,
        ActivityThreadParent,
        ActivityThreadID>;

class ActivityThreadItem : public ActivityThreadItemType
{
public:
    opentxs::Amount Amount() const override { return 0; }
    std::string DisplayAmount() const override { return {}; }
    bool Loading() const override;
    bool MarkRead() const override;
    std::string Memo() const override { return {}; }
    bool Pending() const override;
    std::string Text() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override;

    virtual ~ActivityThreadItem() = default;

protected:
    const Identifier& nym_id_;
    const api::Activity& activity_;
    const std::chrono::system_clock::time_point time_;
    const Identifier& item_id_;
    const StorageBox& box_;
    const Identifier& account_id_;
    std::string text_;
    OTFlag loading_;
    OTFlag pending_;

    ActivityThreadItem(
        const ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time,
        const std::string& text,
        const bool loading,
        const bool pending);

private:
    ActivityThreadItem() = delete;
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    ActivityThreadItem& operator=(const ActivityThreadItem&) = delete;
    ActivityThreadItem& operator=(ActivityThreadItem&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITYTHREADITEM_IMPLEMENTATION_HPP
