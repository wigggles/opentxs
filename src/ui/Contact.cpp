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
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
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
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactSection.hpp"

#include "ContactParent.hpp"
#include "ContactSectionBlank.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "Contact.hpp"

#define OT_METHOD "opentxs::ui::implementation::Contact::"

namespace opentxs
{
ui::Contact* Factory::ContactWidget(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& contactID)
{
    return new ui::implementation::Contact(zmq, publisher, contact, contactID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const std::set<proto::ContactSectionName> Contact::allowed_types_{
    proto::CONTACTSECTION_COMMUNICATION,
    proto::CONTACTSECTION_PROFILE};

const std::map<proto::ContactSectionName, int> Contact::sort_keys_{
    {proto::CONTACTSECTION_COMMUNICATION, 0},
    {proto::CONTACTSECTION_PROFILE, 1}};

Contact::Contact(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const Identifier& contactID)
    : ContactType(
          zmq,
          publisher,
          contact,
          proto::CONTACTSECTION_ERROR,
          contactID,
          new ContactSectionBlank)
    , name_(contact.ContactName(contactID))
    , payment_code_()
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
{
    // NOTE nym_id_ is actually the contact id

    init();
    const auto& endpoint = network::zeromq::Socket::ContactUpdateEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = contact_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&Contact::startup, this));

    OT_ASSERT(startup_)
}

bool Contact::check_type(const proto::ContactSectionName type)
{
    return 1 == allowed_types_.count(type);
}

void Contact::construct_item(
    const ContactIDType& id,
    const ContactSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactSectionWidget(
            zmq_, publisher_, contact_manager_, *this, recover(custom[0])));
}

std::string Contact::ContactID() const { return nym_id_->str(); }

std::string Contact::DisplayName() const
{
    Lock lock(lock_);

    return name_;
}

std::string Contact::PaymentCode() const
{
    Lock lock(lock_);

    return payment_code_;
}

void Contact::process_contact(const opentxs::Contact& contact)
{
    Lock lock(lock_);
    name_ = contact.Label();
    payment_code_ = contact.PaymentCode();
    lock.unlock();
    UpdateNotify();
    std::set<ContactIDType> active{};
    const auto data = contact.Data();

    if (data) {
        for (const auto& section : *data) {
            auto& type = section.first;

            if (check_type(type)) {
                add_item(type, sort_key(type), {section.second.get()});
                active.emplace(type);
            }
        }
    } else {
        items_.clear();
    }

    delete_inactive(active);
    UpdateNotify();
}

void Contact::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    if (contactID != nym_id_) { return; }

    const auto contact = contact_manager_.Contact(contactID);

    OT_ASSERT(contact)

    process_contact(*contact);
}

const opentxs::ContactSection& Contact::recover(const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const opentxs::ContactSection*>(input);
}

int Contact::sort_key(const proto::ContactSectionName type)
{
    return sort_keys_.at(type);
}

void Contact::startup()
{
    otErr << OT_METHOD << __FUNCTION__ << ": Loading contact " << nym_id_->str()
          << std::endl;
    const auto contact = contact_manager_.Contact(nym_id_);

    OT_ASSERT(contact)

    process_contact(*contact);
    startup_complete_->On();
}

void Contact::update(ContactPimpl& row, const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    row.Update(recover(custom[0]));
}
}  // namespace opentxs::ui::implementation
