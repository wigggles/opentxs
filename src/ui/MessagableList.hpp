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

#ifndef OPENTXS_UI_MESSAGABLELIST_IMPLEMENTATION_HPP
#define OPENTXS_UI_MESSAGABLELIST_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/MessagableList.hpp"

#include "ContactListInterface.hpp"
#include "ContactListItem.hpp"
#include "List.hpp"

#include <map>
#include <string>

namespace opentxs::ui::implementation
{
using MessagableListPimpl = OTUIContactListItem;
using MessagableListID = OTIdentifier;
using MessagableListSortKey = std::string;
using MessagableListInner = std::map<MessagableListID, MessagableListPimpl>;
using MessagableListOuter =
    std::map<MessagableListSortKey, MessagableListInner>;
using MessagableListReverse = std::map<MessagableListID, MessagableListSortKey>;
using MessagableListType = List<
    opentxs::ui::MessagableList,
    opentxs::ui::ContactListItem,
    MessagableListID,
    MessagableListPimpl,
    MessagableListInner,
    MessagableListSortKey,
    MessagableListOuter,
    MessagableListOuter::const_iterator,
    MessagableListReverse>;

class MessagableList : virtual public MessagableListType,
                       virtual public ContactListInterface
{
public:
    const Identifier& ID() const override;

    ~MessagableList() = default;

private:
    friend api::implementation::UI;

    const api::client::Sync& sync_;
    const OTIdentifier owner_contact_id_;
    OTZMQListenCallback contact_subscriber_callback_;
    OTZMQSubscribeSocket contact_subscriber_;
    OTZMQListenCallback nym_subscriber_callback_;
    OTZMQSubscribeSocket nym_subscriber_;

    MessagableListID blank_id() const override;
    void construct_item(
        const MessagableListID& id,
        const MessagableListSortKey& index) const override;
    bool last(const MessagableListID& id) const override
    {
        return MessagableListType::last(id);
    }
    MessagableListOuter::const_iterator outer_first() const override;
    MessagableListOuter::const_iterator outer_end() const override;

    void process_contact(
        const MessagableListID& id,
        const MessagableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    MessagableList(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    MessagableList() = delete;
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_MESSAGABLELIST_IMPLEMENTATION_HPP
