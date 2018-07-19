// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_ITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_CONTACT_ITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactItemRow =
    Row<ContactSubsectionRowInterface,
        ContactSubsectionInternalInterface,
        ContactSubsectionRowID>;

class ContactItem : public ContactItemRow
{
public:
    std::string ClaimID() const override { return id_->str(); }
    bool IsActive() const override { return active_; }
    bool IsPrimary() const override { return primary_; }
    std::string Value() const override { return value_; }

    ~ContactItem() = default;

private:
    friend Factory;

    const bool active_{false};
    const bool primary_{false};
    const std::string value_{""};

    ContactItem(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const ContactSubsectionParent& parent,
        const opentxs::ContactItem& item);
    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_ITEM_IMPLEMENTATION_HPP
