// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ProfileItemBlank final : public ProfileSubsectionRowInternal
{
public:
    std::string ClaimID() const override { return {}; }
    bool Delete() const override { return false; }
    bool IsActive() const override { return false; }
    bool IsPrimary() const override { return false; }
    std::string Value() const override { return {}; }
    bool Last() const override { return true; }
    bool SetActive(const bool& active) const override { return false; }
    void SetCallback(ui::Widget::Callback) const override {}
    bool SetPrimary(const bool& primary) const override { return false; }
    bool SetValue(const std::string& value) const override { return false; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ProfileSubsectionSortKey& key, const CustomData& custom)
        override
    {
    }

    ProfileItemBlank() = default;
    ~ProfileItemBlank() = default;

private:
    ProfileItemBlank(const ProfileItemBlank&) = delete;
    ProfileItemBlank(ProfileItemBlank&&) = delete;
    ProfileItemBlank& operator=(const ProfileItemBlank&) = delete;
    ProfileItemBlank& operator=(ProfileItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
