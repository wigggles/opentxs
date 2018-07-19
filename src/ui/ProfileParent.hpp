// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILE_PARENT_HPP
#define OPENTXS_UI_PROFILE_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ProfileParent
{
public:
    using ContactRowID = proto::ContactSectionName;
    using ContactSortKey = int;

    EXPORT virtual bool AddClaim(
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const = 0;
    virtual const Identifier& NymID() const = 0;
    virtual bool last(const ContactRowID& id) const = 0;
    virtual void reindex_item(
        const ContactRowID& id,
        const ContactSortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ProfileParent() = default;

protected:
    ProfileParent() = default;
    ProfileParent(const ProfileParent&) = delete;
    ProfileParent(ProfileParent&&) = delete;
    ProfileParent& operator=(const ProfileParent&) = delete;
    ProfileParent& operator=(ProfileParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_PARENT_HPP
