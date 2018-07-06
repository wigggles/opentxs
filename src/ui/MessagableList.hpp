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

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using MessagableListList = List<
    MessagableExternalInterface,
    MessagableInternalInterface,
    MessagableListRowID,
    MessagableListRowInterface,
    MessagableListRowBlank,
    MessagableListSortKey>;

class MessagableList : virtual public MessagableListList
{
public:
    const Identifier& ID() const override;

    ~MessagableList() = default;

private:
    friend Factory;

    static const ListenerDefinitions listeners_;

    const api::client::Sync& sync_;
    const OTIdentifier owner_contact_id_;

    void construct_row(
        const MessagableListRowID& id,
        const MessagableListSortKey& index,
        const CustomData& custom) const override;
    bool last(const MessagableListRowID& id) const override
    {
        return MessagableListList::last(id);
    }

    void process_contact(
        const MessagableListRowID& id,
        const MessagableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    MessagableList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const Identifier& nymID);
    MessagableList() = delete;
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    MessagableList& operator=(const MessagableList&) = delete;
    MessagableList& operator=(MessagableList&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_MESSAGABLELIST_IMPLEMENTATION_HPP
