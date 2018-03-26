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

#ifndef OPENTXS_UI_LISTROW_HPP
#define OPENTXS_UI_LISTROW_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/Widget.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(UIListRow) opentxs::ui::ListRow;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ListRow : virtual public Widget
{
public:
    EXPORT virtual bool Last() const = 0;
    EXPORT virtual bool Valid() const = 0;

    EXPORT virtual ~ListRow() = default;

protected:
    ListRow() = default;

private:
    ListRow(const ListRow&) = delete;
    ListRow(ListRow&&) = delete;
    ListRow& operator=(const ListRow&) = delete;
    ListRow& operator=(ListRow&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_LISTROW_HPP
