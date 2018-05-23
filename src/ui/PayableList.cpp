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

#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/MultipartMessage.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/PayableList.hpp"

#include "PayableListItemBlank.hpp"
#include "ContactListParent.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "PayableList.hpp"

#define OT_METHOD "opentxs::ui::implementation::PayableList::"

namespace opentxs
{
ui::PayableList* Factory::PayableList(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const Identifier& nymID,
    const proto::ContactItemType& currency)
{
    return new ui::implementation::PayableList(
        zmq, contact, sync, nymID, currency);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PayableList::PayableList(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const Identifier& nymID,
    const proto::ContactItemType& currency)
    : PayableListType(
          zmq,
          contact,
          contact.ContactID(nymID),
          nymID,
          new PayableListItemBlank)
    , sync_(sync)
    , owner_contact_id_(Identifier::Factory(last_id_))
    , contact_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::MultipartMessage& message) -> void {
              this->process_contact(message);
          }))
    , contact_subscriber_(
          zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
    , nym_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::MultipartMessage& message) -> void {
              this->process_nym(message);
          }))
    , nym_subscriber_(zmq_.SubscribeSocket(contact_subscriber_callback_.get()))
    , currency_(currency)
{
    OT_ASSERT(blank_p_)

    init();
    const auto& contactEndpoint =
        network::zeromq::Socket::ContactUpdateEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << contactEndpoint
           << std::endl;
    const auto contactListening = contact_subscriber_->Start(contactEndpoint);

    OT_ASSERT(contactListening)

    const auto& nymEndpoint = network::zeromq::Socket::NymDownloadEndpoint;
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << nymEndpoint
           << std::endl;
    const auto nymListening = nym_subscriber_->Start(nymEndpoint);

    OT_ASSERT(nymListening)

    startup_.reset(new std::thread(&PayableList::startup, this));

    OT_ASSERT(startup_)
}

PayableListID PayableList::blank_id() const { return Identifier::Factory(); }

void PayableList::construct_item(
    const PayableListID& id,
    const PayableListSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    std::unique_ptr<const std::string> paymentCode;
    paymentCode.reset(static_cast<const std::string*>(custom[0]));

    OT_ASSERT(paymentCode);
    OT_ASSERT(false == paymentCode->empty());

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::PayableListItem(
            *this, zmq_, contact_manager_, id, index, *paymentCode, currency_));
}

const Identifier& PayableList::ID() const { return owner_contact_id_; }

PayableListOuter::const_iterator PayableList::outer_first() const
{
    return items_.begin();
}

PayableListOuter::const_iterator PayableList::outer_end() const
{
    return items_.end();
}

void PayableList::process_contact(
    const PayableListID& id,
    const PayableListSortKey& key)
{
    if (owner_contact_id_ == id) { return; }

    const auto contact = contact_manager_.Contact(id);

    if (false == bool(contact)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: Contact " << id->str()
              << " can not be loaded." << std::endl;

        return;
    }

    OT_ASSERT(contact);

    auto paymentCode =
        std::make_unique<std::string>(contact->PaymentCode(currency_));

    OT_ASSERT(paymentCode);

    if (!paymentCode->empty()) {

        add_item(id, key, {paymentCode.release()});
    } else {
        otWarn << OT_METHOD << __FUNCTION__ << ": Skipping unpayable contact "
               << id->str() << std::endl;
    }
}

void PayableList::process_contact(
    const network::zeromq::MultipartMessage& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const Identifier contactID(id);

    OT_ASSERT(false == contactID.empty())

    const auto name = contact_manager_.ContactName(contactID);
    process_contact(contactID, name);
}

void PayableList::process_nym(const network::zeromq::MultipartMessage& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const Identifier nymID(id);

    OT_ASSERT(false == nymID.empty())

    const auto contactID = contact_manager_.ContactID(nymID);
    const auto name = contact_manager_.ContactName(contactID);
    process_contact(contactID, name);
}

void PayableList::startup()
{
    const auto contacts = contact_manager_.ContactList();
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << contacts.size()
           << " contacts." << std::endl;

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier(id), alias);
    }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
