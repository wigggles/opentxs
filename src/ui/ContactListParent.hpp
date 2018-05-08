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

#ifndef OPENTXS_UI_CONTACT_LIST_PARENT_HPP
#define OPENTXS_UI_CONTACT_LIST_PARENT_HPP

#include "opentxs/Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ContactListParent
{
public:
    using ContactListID = OTIdentifier;
    using ContactListSortKey = std::string;

    virtual const Identifier& ID() const = 0;
    virtual bool last(const ContactListID& id) const = 0;
    virtual void reindex_item(
        const ContactListID& id,
        const ContactListSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ContactListParent() = default;

protected:
    ContactListParent() = default;
    ContactListParent(const ContactListParent&) = delete;
    ContactListParent(ContactListParent&&) = delete;
    ContactListParent& operator=(const ContactListParent&) = delete;
    ContactListParent& operator=(ContactListParent&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_LIST_PARENT_HPP
