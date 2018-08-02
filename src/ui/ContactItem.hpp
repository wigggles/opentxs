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
    Row<ContactSubsectionRowInternal,
        ContactSubsectionInternalInterface,
        ContactSubsectionRowID>;

class ContactItem final : public ContactItemRow
{
public:
    std::string ClaimID() const override
    {
        sLock lock(shared_lock_);

        return row_id_->str();
    }
    bool IsActive() const override
    {
        sLock lock(shared_lock_);

        return item_->isActive();
    }
    bool IsPrimary() const override
    {
        sLock lock(shared_lock_);

        return item_->isPrimary();
    }
    std::string Value() const override
    {
        sLock lock(shared_lock_);

        return item_->Value();
    }

    void reindex(const ContactSubsectionSortKey& key, const CustomData& custom)
        override;

    ~ContactItem() = default;

private:
    friend Factory;

    std::unique_ptr<opentxs::ContactItem> item_{nullptr};

    ContactItem(
        const ContactSubsectionInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Contacts& contact,
        const ContactSubsectionRowID& rowID,
        const ContactSubsectionSortKey& sortKey,
        const CustomData& custom);
    ContactItem() = delete;
    ContactItem(const ContactItem&) = delete;
    ContactItem(ContactItem&&) = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_ITEM_IMPLEMENTATION_HPP
