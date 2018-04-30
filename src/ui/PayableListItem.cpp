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
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/Types.hpp"

#include "PayableListItem.hpp"

#include "PayableList.hpp"

#include <locale>

namespace opentxs::ui::implementation
{
PayableListItem::PayableListItem(
    const ContactListInterface& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name,
    const std::string& paymentcode,
    const proto::ContactItemType& currency)
    : PayableListItemType(parent, zmq, contact, id, true)
    , name_(name)
    , paymentCode_(paymentcode)
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
    , currency_(currency)
{
    const auto listening = contact_subscriber_->Start(
        network::zeromq::Socket::ContactUpdateEndpoint);

    OT_ASSERT(listening)
}

std::string PayableListItem::ContactID() const { return id_.str(); }

std::string PayableListItem::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string PayableListItem::ImageURI() const
{
    // TODO

    return {};
}

std::string PayableListItem::PaymentCode() const
{
    Lock lock(lock_);

    return paymentCode_;
}

void PayableListItem::process_contact(const network::zeromq::Message& message)
{
    const Identifier contactID{std::string(message)};

    if (id_ != contactID) {

        return;
    }

    Lock lock(lock_);

    name_ = contact_.ContactName(id_);

    const auto contact = contact_.Contact(id_);

    OT_ASSERT(contact);

    paymentCode_ = contact->PaymentCode();
}

std::string PayableListItem::Section() const
{
    Lock lock(lock_);

    if (id_ == parent_.ID()) {

        return {"ME"};
    }

    if (name_.empty()) {

        return {" "};
    }

    std::locale loc;
    std::string output{" "};
    output[0] = std::toupper(name_[0], loc);

    return output;
}
}  // namespace opentxs::ui::implementation
