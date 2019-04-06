// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class ContactItemBlank final : public ContactSubsectionRowInternal
{
public:
    std::string ClaimID() const override { return {}; }
    bool IsActive() const override { return false; }
    bool IsPrimary() const override { return false; }
    std::string Value() const override { return {}; }
    bool Last() const override { return true; }
    void SetCallback(ui::Widget::Callback) const override {}
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ContactSectionSortKey& key, const CustomData& custom)
        override
    {
    }

    ContactItemBlank() = default;
    ~ContactItemBlank() = default;

private:
    ContactItemBlank(const ContactItemBlank&) = delete;
    ContactItemBlank(ContactItemBlank&&) = delete;
    ContactItemBlank& operator=(const ContactItemBlank&) = delete;
    ContactItemBlank& operator=(ContactItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
