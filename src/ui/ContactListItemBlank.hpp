// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class ContactListItemBlank : public ContactListRowInternal
{
public:
    std::string ContactID() const override { return {}; }
    std::string DisplayName() const override { return {}; }
    std::string ImageURI() const override { return {}; }
    bool Last() const override { return true; }
    std::string Section() const override { return {}; }
    void SetCallback(ui::Widget::Callback) const override {}
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ContactListSortKey&, const CustomData&) override {}

    ContactListItemBlank() = default;
    ~ContactListItemBlank() = default;

private:
    ContactListItemBlank(const ContactListItemBlank&) = delete;
    ContactListItemBlank(ContactListItemBlank&&) = delete;
    ContactListItemBlank& operator=(const ContactListItemBlank&) = delete;
    ContactListItemBlank& operator=(ContactListItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
