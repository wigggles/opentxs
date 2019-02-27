// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ContactSubsection.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class ContactSubsectionBlank final : public ContactSectionRowInternal
{
public:
    std::string Name(const std::string& lang) const override { return {}; }
    OTUIContactItem First() const override
    {
        const std::shared_ptr<const ui::ContactItem> empty;

        return OTUIContactItem{empty};
    }
    bool Last() const override { return true; }
    OTUIContactItem Next() const override
    {
        const std::shared_ptr<const ui::ContactItem> empty;

        return OTUIContactItem{empty};
    }
    proto::ContactItemType Type() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ContactSectionSortKey&, const CustomData&) override {}
    bool last(const ContactSubsectionRowID&) const override { return false; }

    ContactSubsectionBlank() = default;
    ~ContactSubsectionBlank() = default;

private:
    ContactSubsectionBlank(const ContactSubsectionBlank&) = delete;
    ContactSubsectionBlank(ContactSubsectionBlank&&) = delete;
    ContactSubsectionBlank& operator=(const ContactSubsectionBlank&) = delete;
    ContactSubsectionBlank& operator=(ContactSubsectionBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
