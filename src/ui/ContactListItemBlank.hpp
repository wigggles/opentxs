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

#ifndef OPENTXS_UI_CONTACTLISTITEMBLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLISTITEMBLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ContactListItemBlank : virtual public ui::ContactListItem,
                             virtual public opentxs::ui::Widget
{
public:
    std::string ContactID() const override { return {}; }
    std::string DisplayName() const override { return {}; }
    std::string ImageURI() const override { return {}; }
    bool Last() const override { return true; }
    std::string Section() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }
    std::string WidgetName() const override { return {}; }

    ~ContactListItemBlank() = default;

private:
    friend MessagableList;
    friend PayableList;

    ContactListItemBlank() = default;
    ContactListItemBlank(const ContactListItemBlank&) = delete;
    ContactListItemBlank(ContactListItemBlank&&) = delete;
    ContactListItemBlank& operator=(const ContactListItemBlank&) = delete;
    ContactListItemBlank& operator=(ContactListItemBlank&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLISTITEMBLANK_IMPLEMENTATION_HPP
