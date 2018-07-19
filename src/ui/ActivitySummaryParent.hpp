// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP
#define OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP

#include "Internal.hpp"

#include <chrono>
#include <tuple>
#include <string>

namespace opentxs::ui::implementation
{
class ActivitySummaryParent
{
public:
    virtual bool last(const ActivitySummaryRowID& id) const = 0;
    virtual void reindex_item(
        const ActivitySummaryRowID& id,
        const ActivitySummarySortKey& newIndex) const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ActivitySummaryParent() = default;

protected:
    ActivitySummaryParent() = default;
    ActivitySummaryParent(const ActivitySummaryParent&) = delete;
    ActivitySummaryParent(ActivitySummaryParent&&) = delete;
    ActivitySummaryParent& operator=(const ActivitySummaryParent&) = delete;
    ActivitySummaryParent& operator=(ActivitySummaryParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ACTIVITY_SUMMARY_PARENT_HPP
