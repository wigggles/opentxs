// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_LIST_HPP
#define OPENTXS_UI_LIST_HPP

#ifndef Q_MOC_RUN
#include "opentxs/Forward.hpp"

#include "opentxs/ui/Widget.hpp"
#include "opentxs/Types.hpp"

#ifdef SWIG
// clang-format off
%rename(UIList) opentxs::ui::List;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class List : virtual public Widget
{
public:
    OPENTXS_EXPORT ~List() override = default;

protected:
    List() noexcept = default;

private:
    List(const List&) = delete;
    List(List&&) = delete;
    List& operator=(const List&) = delete;
    List& operator=(List&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
namespace opentxs
{
namespace ui
{
struct BlankModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    OPENTXS_EXPORT int columnCount(const QModelIndex& = QModelIndex()) const
        noexcept final
    {
        return columns_;
    }
    OPENTXS_EXPORT QVariant
    data(const QModelIndex&, int = Qt::DisplayRole) const noexcept final
    {
        return {};
    }
    OPENTXS_EXPORT QModelIndex
    index(int, int, const QModelIndex& = QModelIndex()) const noexcept final
    {
        return {};
    }
    OPENTXS_EXPORT QModelIndex parent(const QModelIndex&) const noexcept final
    {
        return {};
    }
    OPENTXS_EXPORT int rowCount(const QModelIndex& = QModelIndex()) const
        noexcept final
    {
        return 0;
    }

    BlankModel(const std::size_t columns)
        : columns_(columns)
    {
    }

private:
    const std::size_t columns_;

    BlankModel() = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
#endif
