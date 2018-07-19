// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP
#define OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ProfileSubsectionParent
{
public:
    using ProfileSubsectionRowIDType = OTIdentifier;
    using ProfileSubsectionSortKey = int;

    virtual bool last(const ProfileSubsectionRowIDType& id) const = 0;
    virtual const Identifier& NymID() const = 0;
    virtual void reindex_item(
        const ProfileSubsectionRowIDType& id,
        const ProfileSubsectionSortKey& newIndex) const = 0;
    virtual proto::ContactSectionName Section() const = 0;
    virtual proto::ContactItemType Type() const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ProfileSubsectionParent() = default;

protected:
    ProfileSubsectionParent() = default;
    ProfileSubsectionParent(const ProfileSubsectionParent&) = delete;
    ProfileSubsectionParent(ProfileSubsectionParent&&) = delete;
    ProfileSubsectionParent& operator=(const ProfileSubsectionParent&) = delete;
    ProfileSubsectionParent& operator=(ProfileSubsectionParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP
