// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYSUMMARY_HPP
#define OPENTXS_UI_ACTIVITYSUMMARY_HPP

#include "opentxs/Forward.hpp"

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

class ActivitySummary : virtual public List
{
public:
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem>
    First() const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem> Next()
        const = 0;

    EXPORT virtual ~ActivitySummary() = default;

protected:
    ActivitySummary() = default;

private:
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    ActivitySummary& operator=(const ActivitySummary&) = delete;
    ActivitySummary& operator=(ActivitySummary&&) = delete;
};

#if OT_QT
class ActivitySummaryQt : public QAbstractItemModel
{
public:
    using ConstructorCallback = std::function<implementation::ActivitySummary*(
        RowCallbacks insert,
        RowCallbacks remove)>;

    enum Roles {
        IDRole = Qt::UserRole + 1,
        NameRole = Qt::UserRole + 2,
        ImageRole = Qt::UserRole + 3,
        TextRole = Qt::UserRole + 4,
        TimestampRole = Qt::UserRole + 5,
        TypeRole = Qt::UserRole + 6,
    };

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const override;
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    const ActivitySummary& operator*() const;

    ActivitySummaryQt(ConstructorCallback cb);
    ~ActivitySummaryQt() override;

signals:
    void updated() const;

private:
    Q_OBJECT

    std::unique_ptr<implementation::ActivitySummary> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    ActivitySummaryQt() = delete;
    ActivitySummaryQt(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt(ActivitySummaryQt&&) = delete;
    ActivitySummaryQt& operator=(const ActivitySummaryQt&) = delete;
    ActivitySummaryQt& operator=(ActivitySummaryQt&&) = delete;
};
#endif
}  // namespace ui
}  // namespace opentxs
#endif
