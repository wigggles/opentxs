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

#ifndef OPENTXS_UI_CONTACTLIST_HPP
#define OPENTXS_UI_CONTACTLIST_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/Widget.hpp"

#ifdef SWIG
// clang-format off
%rename(UIContactList) opentxs::ui::ContactList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ContactList : virtual public Widget
{
public:
    EXPORT virtual const ContactListItem& First() const = 0;
    EXPORT virtual const ContactListItem& Next() const = 0;

    EXPORT virtual ~ContactList() = default;

protected:
    ContactList() = default;

private:
    friend OTUIContactList;

    /** WARNING: not implemented */
    virtual ContactList* clone() const = 0;

    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_CONTACTLIST_HPP
