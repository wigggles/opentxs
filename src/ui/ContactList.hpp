// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ContactList.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowInternal,
    ContactListRowBlank,
    ContactListSortKey,
    ContactListPrimaryID>;

class ContactList final : public ContactListList
{
public:
    std::string AddContact(
        const std::string& label,
        const std::string& paymentCode,
        const std::string& nymID) const override;
#if OT_QT
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const override;
#endif
    const Identifier& ID() const override { return owner_contact_id_; }
#if OT_QT
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const override;
#endif

    ~ContactList();

private:
    friend api::client::implementation::UI;

    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    std::shared_ptr<ContactListRowInternal> owner_;

    void construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        const CustomData& custom) const override;
    std::shared_ptr<const ContactListRowInternal> first(
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
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
    );
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // namespace opentxs::ui::implementation
