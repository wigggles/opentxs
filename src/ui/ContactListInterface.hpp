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

#ifndef OPENTXS_UI_CONTACTLISTINTERFACE_HPP
#define OPENTXS_UI_CONTACTLISTINTERFACE_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ContactListInterface : virtual public opentxs::ui::Widget
{
public:
    virtual const Identifier& ID() const = 0;
    virtual bool last(const Identifier& id) const = 0;

protected:
    ContactListInterface() = default;

    virtual ~ContactListInterface() = default;

private:
    ContactListInterface(const ContactListInterface&) = delete;
    ContactListInterface(ContactListInterface&&) = delete;
    ContactListInterface& operator=(const ContactListInterface&) = delete;
    ContactListInterface& operator=(ContactListInterface&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLISTINTERFACE_HPP
