// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class ActivitySummaryItemBlank final : public ActivitySummaryRowInternal
{
public:
    std::string DisplayName() const override { return {}; }
    std::string ImageURI() const override { return {}; }
    bool Last() const override { return true; }
    std::string Text() const override { return {}; }
    std::string ThreadID() const override { return {}; }
    std::chrono::system_clock::time_point Timestamp() const override
    {
        return {};
    }
    StorageBox Type() const override { return StorageBox::UNKNOWN; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const ActivitySummarySortKey&, const CustomData&) override {}

    ActivitySummaryItemBlank() = default;
    ~ActivitySummaryItemBlank() = default;

private:
    ActivitySummaryItemBlank(const ActivitySummaryItemBlank&) = delete;
    ActivitySummaryItemBlank(ActivitySummaryItemBlank&&) = delete;
    ActivitySummaryItemBlank& operator=(const ActivitySummaryItemBlank&) =
        delete;
    ActivitySummaryItemBlank& operator=(ActivitySummaryItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
