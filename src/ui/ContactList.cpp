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

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"

#include "ContactListParent.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "ContactList.hpp"

#define OT_METHOD "opentxs::ui::implementation::ContactList::"

namespace opentxs
{
ui::ContactList* Factory::ContactList(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& nymID)
{
    return new ui::implementation::ContactList(zmq, publisher, contact, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactList::ContactList(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& nymID)
    : ContactListType(
          zmq,
          publisher,
          contact,
          contact.ContactID(nymID),
          nymID,
          nullptr)
    , owner_contact_id_(Identifier::Factory(last_id_))
    , owner_p_(Factory::ContactListItem(
          *this,
          zmq,
          publisher_,
          contact,
          owner_contact_id_,
          "Owner"))
    , owner_(*owner_p_)
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
{
    OT_ASSERT(!last_id_->empty())
    OT_ASSERT(owner_p_)

    // WARNING do not attempt to use blank_ in this class
    init();
    const auto& endpoint = network::zeromq::Socket::ContactUpdateEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = contact_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ContactList::startup, this));

    OT_ASSERT(startup_)
}

void ContactList::add_item(
    const ContactListID& id,
    const ContactListSortKey& index,
    const CustomData& custom)
{
    if (owner_contact_id_ == id) {
        owner_.SetName(index);

        return;
    }

    insert_outer(id, index, custom);
}

ContactListID ContactList::blank_id() const { return Identifier::Factory(); }

void ContactList::construct_item(
    const ContactListID& id,
    const ContactListSortKey& index,
    const CustomData&) const
{
    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactListItem(
            *this, zmq_, publisher_, contact_manager_, id, index));
}

/** Returns owner contact. Sets up iterators for next row */
std::shared_ptr<const opentxs::ui::ContactListItem> ContactList::first(
    const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    have_items_->Set(first_valid_item(lock));
    start_->Set(!have_items_.get());
    last_id_ = owner_contact_id_;

    return owner_p_;
}

const Identifier& ContactList::ID() const { return owner_contact_id_; }

ContactListOuter::const_iterator ContactList::outer_first() const
{
    return items_.begin();
}

ContactListOuter::const_iterator ContactList::outer_end() const
{
    return items_.end();
}

void ContactList::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());
    const auto contactID(Identifier::Factory());

    const std::string id(*message.Body().begin());

    OT_ASSERT(false == contactID->empty())

    const auto name = contact_manager_.ContactName(contactID);
    add_item(contactID, name, {});
}

void ContactList::startup()
{
    const auto contacts = contact_manager_.ContactList();
    otErr << OT_METHOD << __FUNCTION__ << ": Loading " << contacts.size()
          << " contacts." << std::endl;

    for (const auto& [id, alias] : contacts) {
        add_item(Identifier::Factory(id), alias, {});
    }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
