// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_SECTION_PARENT_HPP
#define OPENTXS_UI_CONTACT_SECTION_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ContactSectionParent
{
public:
    using ContactSectionRowID =
        std::pair<proto::ContactSectionName, proto::ContactItemType>;
    using ContactSectionSortKey = int;

    virtual std::string ContactID() const = 0;
    virtual bool last(const ContactSectionRowID& id) const = 0;
    virtual void reindex_item(
        const ContactSectionRowID& id,
        const ContactSectionSortKey& newIndex) const = 0;
    virtual proto::ContactSectionName Type() const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ContactSectionParent() = default;

protected:
    ContactSectionParent() = default;
    ContactSectionParent(const ContactSectionParent&) = delete;
    ContactSectionParent(ContactSectionParent&&) = delete;
    ContactSectionParent& operator=(const ContactSectionParent&) = delete;
    ContactSectionParent& operator=(ContactSectionParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_SECTION_PARENT_HPP
