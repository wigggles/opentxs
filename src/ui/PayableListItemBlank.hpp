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

#ifndef OPENTXS_UI_PAYABLELISTITEMBLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_PAYABLELISTITEMBLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/PayableListItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "ContactListItemBlank.hpp"

namespace opentxs::ui::implementation
{
class PayableListItemBlank : virtual public ui::PayableListItem,
                             virtual public ContactListItemBlank
{
public:
    std::string PaymentCode() const override { return {}; }

    ~PayableListItemBlank() = default;

private:
    friend PayableList;

    PayableListItemBlank() = default;
    PayableListItemBlank(const PayableListItemBlank&) = delete;
    PayableListItemBlank(PayableListItemBlank&&) = delete;
    PayableListItemBlank& operator=(const PayableListItemBlank&) = delete;
    PayableListItemBlank& operator=(PayableListItemBlank&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_PAYABLELISTITEMBLANK_IMPLEMENTATION_HPP
