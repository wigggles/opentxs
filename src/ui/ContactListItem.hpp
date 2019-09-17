// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListItemRow =
    Row<ContactListRowInternal, ContactListInternalInterface, ContactListRowID>;

class ContactListItem : public ContactListItemRow
{
public:
    std::string ContactID() const noexcept final;
    std::string DisplayName() const noexcept final;
    std::string ImageURI() const noexcept final;
    std::string Section() const noexcept final;

    void reindex(
        const ContactListSortKey&,
        const CustomData&) noexcept override;

    ~ContactListItem() override = default;

protected:
    ContactListSortKey key_{""};

    ContactListItem(
        const ContactListInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const ContactListRowID& rowID,
        const ContactListSortKey& key) noexcept;

private:
    friend opentxs::Factory;

    ContactListItem() = delete;
    ContactListItem(const ContactListItem&) = delete;
    ContactListItem(ContactListItem&&) = delete;
    ContactListItem& operator=(const ContactListItem&) = delete;
    ContactListItem& operator=(ContactListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
