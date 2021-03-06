// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARY_HPP
#define OPENTXS_UI_ACTIVITYSUMMARY_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIActivitySummary) opentxs::ui::ActivitySummary;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivitySummary;
}  // namespace implementation

class ActivitySummary;
class ActivitySummaryItem;

#if OT_QT
class ActivitySummaryQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class ActivitySummary : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::ActivitySummaryItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::ActivitySummaryItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~ActivitySummary() override = default;

protected:
    ActivitySummary() = default;

private:
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    ActivitySummary& operator=(const ActivitySummary&) = delete;
    ActivitySummary& operator=(ActivitySummary&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class OPENTXS_EXPORT opentxs::ui::ActivitySummaryQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    ActivitySummaryQt(implementation::ActivitySummary& parent) noexcept;

    ~ActivitySummaryQt() final = default;

private:
    friend opentxs::Factory;

    implementation::ActivitySummary& parent_;

    void notify() const noexcept;

    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    ActivitySummaryQt& operator=(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt& operator=(ActivitySummaryQt&&) = delete;
};
#endif
#endif
