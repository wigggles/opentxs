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

#ifndef OPENTXS_UI_ACCOUNT_ACTIVITY_PARENT_HPP
#define OPENTXS_UI_ACCOUNT_ACTIVITY_PARENT_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class AccountSummaryParent
{
public:
    virtual proto::ContactItemType Currency() const = 0;
    virtual const Identifier& NymID() const = 0;
    virtual bool last(const AccountSummaryRowID& id) const = 0;
    virtual void reindex_item(
        const AccountSummaryRowID& id,
        const AccountSummarySortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~AccountSummaryParent() = default;

protected:
    AccountSummaryParent() = default;
    AccountSummaryParent(const AccountSummaryParent&) = delete;
    AccountSummaryParent(AccountSummaryParent&&) = delete;
    AccountSummaryParent& operator=(const AccountSummaryParent&) = delete;
    AccountSummaryParent& operator=(AccountSummaryParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ACCOUNT_ACTIVITY_PARENT_HPP
