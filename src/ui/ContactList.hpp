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

#ifndef OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "ContactListItem.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"

#include <map>
#include <string>
#include <thread>

namespace opentxs::ui::implementation
{
class ContactList : virtual public ui::ContactList, Lockable
{
public:
    const opentxs::ui::ContactListItem& First() const override;
    const opentxs::ui::ContactListItem& Next() const override;

    ~ContactList();

private:
    friend api::implementation::UI;
    friend ContactListItem;
    /** Contact ID, Contact */
    using ItemIndex = std::map<Identifier, OTUIContactListItem>;
    /** Display name, contacts*/
    using ItemMap = std::map<std::string, ItemIndex>;
    /** ContactID, display name*/
    using NameMap = std::map<Identifier, std::string>;

    const network::zeromq::Context& zmq_;
    const api::ContactManager& contact_manager_;
    const Identifier owner_contact_id_;
    mutable Identifier last_id_;
    ContactListItem owner_;
    ItemMap items_;
    NameMap names_;
    mutable OTFlag have_items_;
    mutable OTFlag start_;
    mutable OTFlag startup_complete_;
    mutable ItemMap::const_iterator name_;
    mutable ItemIndex::const_iterator contact_;
    std::unique_ptr<std::thread> startup_{nullptr};
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;

    const opentxs::ui::ContactListItem& first(const Lock& lock) const;
    bool first_valid_item(const Lock& lock) const;
    void increment_contact(const Lock& lock) const;
    bool increment_name(const Lock& lock) const;
    const opentxs::ui::ContactListItem& next(const Lock& lock) const;

    void add_contact(const std::string& id, const std::string& alias);
    void process_contact(const network::zeromq::Message& message);
    void rename_contact(
        const Lock& lock,
        const Identifier& contactID,
        const std::string& oldName,
        const std::string& newName);
    void startup();

    ContactList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& nymID);
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
