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

#include "ContactListItem.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/Types.hpp"

#include "ContactList.hpp"

#include <locale>

namespace opentxs::ui::implementation
{
ContactListItem::ContactListItem(
    const ContactList& parent,
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& id,
    const std::string& name)
    : ContactListItemType(parent, zmq, contact, id)
    , name_(name)
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
{
    const auto listening = contact_subscriber_->Start(
        network::zeromq::Socket::ContactUpdateEndpoint);

    OT_ASSERT(listening)
}

std::string ContactListItem::ContactID() const { return String(id_).Get(); }

std::string ContactListItem::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string ContactListItem::ImageURI() const
{
    // TODO

    return {};
}

void ContactListItem::process_contact(const network::zeromq::Message& message)
{
    const Identifier contactID{std::string(message)};

    if (id_ != contactID) {

        return;
    }

    Lock lock(lock_);
    name_ = contact_.ContactName(id_);
}

std::string ContactListItem::Section() const
{
    Lock lock(lock_);

    if (id_ == parent_.owner_contact_id_) {

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
