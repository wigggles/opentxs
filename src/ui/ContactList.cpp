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

#include "ContactList.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/Types.hpp"

#define STARTUP_WAIT_MILLISECONDS 100
#define VALID_ITERATORS()                                                      \
    {                                                                          \
        OT_ASSERT(items_.end() != name_)                                       \
                                                                               \
        const auto& validName = name_->second;                                 \
                                                                               \
        OT_ASSERT(validName.end() != contact_)                                 \
    }

//#define OT_METHOD "opentxs::ui::implementation::ContactList::"

namespace opentxs::ui::implementation
{
ContactList::ContactList(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const Identifier& nymID)
    : zmq_(zmq)
    , contact_manager_(contact)
    , owner_contact_id_(contact_manager_.ContactID(nymID))
    , last_id_(owner_contact_id_)
    , owner_(*this, zmq, contact, owner_contact_id_, "Owner")
    , items_()
    , names_()
    , have_items_(Flag::Factory(false))
    , start_(Flag::Factory(true))
    , startup_complete_(Flag::Factory(false))
    , name_(items_.begin())
    , contact_(items_.begin()->second.begin())
    , startup_(nullptr)
    , contact_subscriber_(zmq_.SubscribeSocket())
{
    network::zeromq::Socket::ReceiveCallback callback =
        [this](const network::zeromq::Message& message) -> void {
        this->process_contact(message);
    };
    contact_subscriber_->RegisterCallback(callback);
    const auto listening = contact_subscriber_->Start(
        network::zeromq::Socket::ContactUpdateEndpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ContactList::startup, this));

    OT_ASSERT(startup_)
}

void ContactList::add_contact(const std::string& id, const std::string& alias)
{
    const Identifier contactID(id);

    if (owner_contact_id_ == contactID) {
        Lock lock(owner_.lock_);
        owner_.name_ = alias;

        return;
    }

    Lock lock(lock_);

    if (0 == names_.count(contactID)) {
        names_.emplace(id, alias);
        items_[alias].emplace(
            contactID,
            new ContactListItem(
                *this, zmq_, contact_manager_, contactID, alias));

        return;
    }

    const auto& oldName = names_.at(contactID);

    if (oldName == alias) {

        return;
    }

    rename_contact(lock, contactID, oldName, alias);
}

/** Returns owner contact. Sets up iterators for next row */
const opentxs::ui::ContactListItem& ContactList::first(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    have_items_->Set(first_valid_item(lock));
    start_->Set(!have_items_.get());
    last_id_ = owner_contact_id_;

    return owner_;
}

const opentxs::ui::ContactListItem& ContactList::First() const
{
    Lock lock(lock_);

    return first(lock);
}

/** Searches for the first name with at least one contact and sets iterators
 *  to match
 *
 *  If this function returns false, then no valid names are present and
 *  the value of contact_ is undefined.
 */
bool ContactList::first_valid_item(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    if (0 == items_.size()) {

        return false;
    }

    name_ = items_.begin();

    while (items_.end() != name_) {
        const auto& currentName = name_->second;

        if (0 < currentName.size()) {
            contact_ = currentName.begin();
            VALID_ITERATORS()

            return true;
        }

        ++name_;
    }

    return false;
}

/** Increment iterators to the next valid contact, or loop back to owner */
void ContactList::increment_contact(const Lock& lock) const
{
    VALID_ITERATORS()

    const auto& currentName = name_->second;

    ++contact_;

    if (currentName.end() != contact_) {
        VALID_ITERATORS()

        return;
    }

    // The previous position was the last contact for this name.
    increment_name(lock);
}

/** Move to the next valid name, or loop back to owner
 *
 *  contact_ is an invalid iterator at this point
 */
bool ContactList::increment_name(const Lock& lock) const
{
    OT_ASSERT(items_.end() != name_)

    bool searching{true};

    while (searching) {
        ++name_;

        if (items_.end() == name_) {
            // Both iterators are invalid at this point
            start_->On();
            have_items_->Set(first_valid_item(lock));

            if (have_items_.get()) {
                VALID_ITERATORS()
            }

            return false;
        }

        const auto& currentName = name_->second;

        if (0 < currentName.size()) {
            searching = false;
            contact_ = currentName.begin();
        }
    }

    VALID_ITERATORS()

    return true;
}

/** Returns the next non-owner contact, or loops around to the owner contact */
const opentxs::ui::ContactListItem& ContactList::next(const Lock& lock) const
{
    VALID_ITERATORS()

    auto& output = contact_->second.get();
    last_id_ = Identifier(output.ContactID());
    increment_contact(lock);

    return output;
}

const opentxs::ui::ContactListItem& ContactList::Next() const
{
    Lock lock(lock_);

    if (start_.get()) {

        return first(lock);
    }

    return next(lock);
}

void ContactList::process_contact(const network::zeromq::Message& message)
{
    while (false == startup_complete_.get()) {
        Log::Sleep(std::chrono::milliseconds(STARTUP_WAIT_MILLISECONDS));
    }

    const std::string id(message);
    const Identifier contactID(id);

    OT_ASSERT(false == contactID.empty())

    const auto name = contact_manager_.ContactName(contactID);
    add_contact(id, name);
}

void ContactList::rename_contact(
    const Lock& lock,
    const Identifier& contactID,
    const std::string& oldName,
    const std::string& newName)
{
    OT_ASSERT(verify_lock(lock));

    names_[contactID] = newName;
    auto name = items_.find(oldName);

    OT_ASSERT(items_.end() != name);

    auto& contactMap = name->second;
    auto contact = contactMap.find(contactID);

    OT_ASSERT(contactMap.end() != contact);

    // I'm about to delete this row. Make sure iterators are not pointing to it
    if (contact_ == contact) {
        increment_contact(lock);
    }

    OTUIContactListItem item = std::move(contact->second);
    const auto deleted = contactMap.erase(contactID);

    OT_ASSERT(1 == deleted)

    items_[newName].emplace(contactID, std::move(item));
}

void ContactList::startup()
{
    const auto contacts = contact_manager_.ContactList();

    for (const auto & [ id, alias ] : contacts) {
        add_contact(id, alias);
    }

    startup_complete_->On();
}

ContactList::~ContactList()
{
    if (startup_ && startup_->joinable()) {
        startup_->join();
        startup_.reset();
    }
}
}  // namespace opentxs::ui::implementation
