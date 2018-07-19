// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_PARENT_HPP
#define OPENTXS_UI_CONTACT_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ContactParent
{
public:
    using ContactRowID = proto::ContactSectionName;
    using ContactSortKey = int;

    virtual std::string ContactID() const = 0;
    virtual bool last(const ContactRowID& id) const = 0;
    virtual void reindex_item(
        const ContactRowID& id,
        const ContactSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ContactParent() = default;

protected:
    ContactParent() = default;
    ContactParent(const ContactParent&) = delete;
    ContactParent(ContactParent&&) = delete;
    ContactParent& operator=(const ContactParent&) = delete;
    ContactParent& operator=(ContactParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_PARENT_HPP
