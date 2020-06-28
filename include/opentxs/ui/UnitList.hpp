// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_UNITLIST_HPP
#define OPENTXS_UI_UNITLIST_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIUnitList) opentxs::ui::UnitList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class UnitList;
}  // namespace implementation

class UnitList;
class UnitListItem;

#if OT_QT
class UnitListQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class UnitList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::UnitListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::UnitListItem>
    Next() const noexcept = 0;

    ~UnitList() override = default;

protected:
    UnitList() noexcept = default;

private:
    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    UnitList& operator=(const UnitList&) = delete;
    UnitList& operator=(UnitList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::UnitListQt final : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    enum Columns {
        UnitNameColumn = 0,
    };
    enum Roles {
        UnitIDRole = Qt::UserRole,
    };

    UnitListQt(implementation::UnitList& parent) noexcept;

    ~UnitListQt() final = default;

private:
    implementation::UnitList& parent_;

    void notify() const noexcept;

    UnitListQt(const UnitListQt&) = delete;
    UnitListQt(UnitListQt&&) = delete;
    UnitListQt& operator=(const UnitListQt&) = delete;
    UnitListQt& operator=(UnitListQt&&) = delete;
};
#endif
#endif
