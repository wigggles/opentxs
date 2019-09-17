// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILEITEM_HPP
#define OPENTXS_UI_PROFILEITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <string>

#include "ListRow.hpp"

#ifdef SWIG
// clang-format off
%template(OTUIProfileItem) opentxs::SharedPimpl<opentxs::ui::ProfileItem>;
%rename(UIProfileItem) opentxs::ui::ProfileItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ProfileItem : virtual public ListRow
{
public:
    EXPORT virtual std::string ClaimID() const noexcept = 0;
    EXPORT virtual bool Delete() const noexcept = 0;
    EXPORT virtual bool IsActive() const noexcept = 0;
    EXPORT virtual bool IsPrimary() const noexcept = 0;
    EXPORT virtual bool SetActive(const bool& active) const noexcept = 0;
    EXPORT virtual bool SetPrimary(const bool& primary) const noexcept = 0;
    EXPORT virtual bool SetValue(const std::string& value) const noexcept = 0;
    EXPORT virtual std::string Value() const noexcept = 0;

    EXPORT virtual ~ProfileItem() = default;

protected:
    ProfileItem() noexcept = default;

private:
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    ProfileItem& operator=(const ProfileItem&) = delete;
    ProfileItem& operator=(ProfileItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
