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

#ifndef OPENTXS_UI_CONTACT_ITEM_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_ITEM_BLANK_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ContactItemBlank : virtual public ui::ContactItem
{
public:
    std::string ClaimID() const override { return {}; }
    bool IsActive() const override { return false; }
    bool IsPrimary() const override { return false; }
    std::string Value() const override { return {}; }
    bool Last() const override { return true; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    ContactItemBlank() = default;
    ~ContactItemBlank() = default;

private:
    ContactItemBlank(const ContactItemBlank&) = delete;
    ContactItemBlank(ContactItemBlank&&) = delete;
    ContactItemBlank& operator=(const ContactItemBlank&) = delete;
    ContactItemBlank& operator=(ContactItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_ITEM_BLANK_IMPLEMENTATION_HPP
