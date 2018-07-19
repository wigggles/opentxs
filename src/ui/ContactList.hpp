// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowBlank,
    ContactListSortKey>;

class ContactList : virtual public ContactListList
{
public:
    const Identifier& ID() const override;

    ~ContactList() = default;

private:
    friend Factory;

    static const ListenerDefinitions listeners_;

    const OTIdentifier owner_contact_id_;
    std::shared_ptr<opentxs::ui::ContactListItem> owner_p_;
    opentxs::ui::ContactListItem& owner_;

    void construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        const CustomData& custom) const override;
    std::shared_ptr<const opentxs::ui::ContactListItem> first(
        const Lock& lock) const override;
    bool last(const ContactListRowID& id) const override
    {
        return ContactListList::last(id);
    }

    void add_item(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        const CustomData& custom) override;
    void process_contact(const network::zeromq::Message& message);
    void startup();

    ContactList(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID);
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLIST_IMPLEMENTATION_HPP
