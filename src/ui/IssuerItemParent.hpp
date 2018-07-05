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

#ifndef OPENTXS_UI_ISSUER_ITEM_PARENT_HPP
#define OPENTXS_UI_ISSUER_ITEM_PARENT_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class IssuerItemParent
{
public:
    virtual bool last(const IssuerItemRowID& id) const = 0;
    virtual void reindex_item(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~IssuerItemParent() = default;

protected:
    IssuerItemParent() = default;
    IssuerItemParent(const IssuerItemParent&) = delete;
    IssuerItemParent(IssuerItemParent&&) = delete;
    IssuerItemParent& operator=(const IssuerItemParent&) = delete;
    IssuerItemParent& operator=(IssuerItemParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ISSUER_ITEM_PARENT_HPP
