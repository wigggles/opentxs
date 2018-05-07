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

#ifndef OPENTXS_UI_PAYABLELISTITEM_HPP
#define OPENTXS_UI_PAYABLELISTITEM_HPP

#include "opentxs/Forward.hpp"

#include <string>

#include "ContactListItem.hpp"

#ifdef SWIG
// clang-format off
%rename(UIPayableListItem) opentxs::ui::PayableListItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class PayableListItem : virtual public ContactListItem
{
public:
    EXPORT virtual std::string PaymentCode() const = 0;

    EXPORT virtual ~PayableListItem() = default;

protected:
    PayableListItem() = default;

private:
    friend OTUIPayableListItem;

    /** WARNING: not implemented */
    virtual PayableListItem* clone() const = 0;

    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    PayableListItem& operator=(const PayableListItem&) = delete;
    PayableListItem& operator=(PayableListItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_PAYABLELISTITEM_HPP
