// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_LIST_PARENT_HPP
#define OPENTXS_UI_CONTACT_LIST_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ContactListParent
{
public:
    using ContactListRowID = OTIdentifier;
    using ContactListSortKey = std::string;

    virtual const Identifier& ID() const = 0;
    virtual bool last(const ContactListRowID& id) const = 0;
    virtual void reindex_item(
        const ContactListRowID& id,
        const ContactListSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ContactListParent() = default;

protected:
    ContactListParent() = default;
    ContactListParent(const ContactListParent&) = delete;
    ContactListParent(ContactListParent&&) = delete;
    ContactListParent& operator=(const ContactListParent&) = delete;
    ContactListParent& operator=(ContactListParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_LIST_PARENT_HPP
