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

#include "stdafx.hpp"

#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ContactItem.hpp"

#include "ContactSubsectionParent.hpp"
#include "Row.hpp"

#include "ContactItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;

namespace opentxs
{
ui::ContactItem* Factory::ContactItemWidget(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const ui::implementation::ContactSubsectionParent& parent,
    const ContactItem& item)
{
    return new ui::implementation::ContactItem(
        zmq, publisher, contact, parent, item);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactItem::ContactItem(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const ContactSubsectionParent& parent,
    const opentxs::ContactItem& item)
    : ContactItemType(
          parent,
          zmq,
          publisher,
          contact,
          Identifier::Factory(item.ID()),
          true)
    , active_(item.isActive())
    , primary_(item.isPrimary())
    , value_(item.Value())
{
}
}  // namespace opentxs::ui::implementation
