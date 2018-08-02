// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListItemRow =
    Row<ContactListRowInternal, ContactListInternalInterface, ContactListRowID>;

class ContactListItem : public ContactListItemRow
{
public:
    std::string ContactID() const override;
    std::string DisplayName() const override;
    std::string ImageURI() const override;
    std::string Section() const override;

    void reindex(const ContactListSortKey&, const CustomData&) override;

    virtual ~ContactListItem() = default;

protected:
    ContactListSortKey key_{""};

    ContactListItem(
        const ContactListInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ContactListRowID& rowID,
        const ContactListSortKey& key);

private:
    friend Factory;

    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    ContactListItem& operator=(const ContactListItem&) = delete;
    ContactListItem& operator=(ContactListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACTLISTITEM_IMPLEMENTATION_HPP
