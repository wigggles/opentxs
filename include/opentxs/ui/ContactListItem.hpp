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

#ifndef OPENTXS_UI_CONTACTLISTITEM_HPP
#define OPENTXS_UI_CONTACTLISTITEM_HPP

#include "opentxs/Forward.hpp"

#include <string>

namespace opentxs
{
namespace ui
{
class ContactListItem
{
public:
    EXPORT virtual std::string ContactID() const = 0;
    EXPORT virtual std::string DisplayName() const = 0;
    EXPORT virtual std::string ImageURI() const = 0;
    EXPORT virtual bool Last() const = 0;
    EXPORT virtual std::string Section() const = 0;

    EXPORT virtual ~ContactListItem() = default;

protected:
    ContactListItem() = default;

private:
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    ContactListItem& operator=(const ContactListItem&) = delete;
    ContactListItem& operator=(ContactListItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_CONTACTLISTITEM_HPP
