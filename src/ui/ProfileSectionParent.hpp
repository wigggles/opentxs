// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILE_SECTION_PARENT_HPP
#define OPENTXS_UI_PROFILE_SECTION_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ProfileSectionParent
{
public:
    using ProfileSectionRowIDType =
        std::pair<proto::ContactSectionName, proto::ContactItemType>;
    using ProfileSectionSortKey = int;

    EXPORT virtual bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const = 0;
    virtual bool last(const ProfileSectionRowIDType& id) const = 0;
    virtual const Identifier& NymID() const = 0;
    virtual void reindex_item(
        const ProfileSectionRowIDType& id,
        const ProfileSectionSortKey& newIndex) const = 0;
    virtual proto::ContactSectionName Type() const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ProfileSectionParent() = default;

protected:
    ProfileSectionParent() = default;
    ProfileSectionParent(const ProfileSectionParent&) = delete;
    ProfileSectionParent(ProfileSectionParent&&) = delete;
    ProfileSectionParent& operator=(const ProfileSectionParent&) = delete;
    ProfileSectionParent& operator=(ProfileSectionParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SECTION_PARENT_HPP
