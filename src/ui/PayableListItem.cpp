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

#include "opentxs/stdafx.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/PayableListItem.hpp"

#include "ContactListParent.hpp"
#include "Row.hpp"

#include "PayableListItem.hpp"

namespace opentxs
{
ui::PayableListItem* Factory::PayableListItem(
    const ui::implementation::ContactListParent& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name,
    const std::string& paymentcode,
    const proto::ContactItemType& currency)
{
    return new ui::implementation::PayableListItem(
        parent, zmq, contact, id, name, paymentcode, currency);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PayableListItem::PayableListItem(
    const ContactListParent& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name,
    const std::string& paymentcode,
    const proto::ContactItemType& currency)
    : ot_super(parent, zmq, contact, id, name)
    , payment_code_(paymentcode)
    , currency_(currency)
{
}

std::string PayableListItem::PaymentCode() const
{
    Lock lock(lock_);

    return payment_code_;
}

void PayableListItem::process_contact(
    const network::zeromq::MultipartMessage& message)
{
    ot_super::process_contact(message);
    const auto contact = contact_.Contact(id_);

    OT_ASSERT(contact);

    payment_code_ = contact->PaymentCode(currency_);
}
}  // namespace opentxs::ui::implementation
