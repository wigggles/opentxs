// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class ActivityThreadItemBlank final : public ActivityThreadRowInternal
{
public:
    opentxs::Amount Amount() const override { return 0; }
    bool Deposit() const override { return false; }
    std::string DisplayAmount() const override { return {}; }
    bool Last() const override { return true; }
    bool Loading() const override { return false; }
    bool MarkRead() const override { return false; }
    std::string Memo() const override { return {}; }
    bool Pending() const override { return false; }
    void SetCallback(ui::Widget::Callback) const override {}
    std::string Text() const override { return {}; }
    std::chrono::system_clock::time_point Timestamp() const override
    {
        return {};
    }
    StorageBox Type() const override { return StorageBox::UNKNOWN; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ActivityThreadSortKey&, const CustomData&) override {}

    ActivityThreadItemBlank() = default;
    ~ActivityThreadItemBlank() = default;

private:
    ActivityThreadItemBlank(const ActivityThreadItemBlank&) = delete;
    ActivityThreadItemBlank(ActivityThreadItemBlank&&) = delete;
    ActivityThreadItemBlank& operator=(const ActivityThreadItemBlank&) = delete;
    ActivityThreadItemBlank& operator=(ActivityThreadItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
