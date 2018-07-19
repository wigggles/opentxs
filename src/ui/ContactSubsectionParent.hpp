// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_SUBSECTION_PARENT_HPP
#define OPENTXS_UI_CONTACT_SUBSECTION_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ContactSubsectionParent
{
public:
    using ContactSubsectionRowID = OTIdentifier;
    using ContactSubsectionSortKey = int;

    virtual bool last(const ContactSubsectionRowID& id) const = 0;
    virtual void reindex_item(
        const ContactSubsectionRowID& id,
        const ContactSubsectionSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ContactSubsectionParent() = default;

protected:
    ContactSubsectionParent() = default;
    ContactSubsectionParent(const ContactSubsectionParent&) = delete;
    ContactSubsectionParent(ContactSubsectionParent&&) = delete;
    ContactSubsectionParent& operator=(const ContactSubsectionParent&) = delete;
    ContactSubsectionParent& operator=(ContactSubsectionParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_CONTACT_SUBSECTION_PARENT_HPP
