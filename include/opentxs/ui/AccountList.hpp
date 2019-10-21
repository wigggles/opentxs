// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLIST_HPP
#define OPENTXS_UI_ACCOUNTLIST_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIAccountList) opentxs::ui::AccountList;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class AccountList;
}  // namespace implementation

class AccountList : virtual public List
{
public:
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem> First()
        const noexcept = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem> Next()
        const noexcept = 0;

    EXPORT ~AccountList() override = default;

protected:
    AccountList() noexcept = default;

private:
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    AccountList& operator=(const AccountList&) = delete;
    AccountList& operator=(AccountList&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::AccountListQt final : public QAbstractItemModel
{
    Q_OBJECT

public:
    using ConstructorCallback = std::function<
        implementation::AccountList*(RowCallbacks insert, RowCallbacks remove)>;

    int columnCount(const QModelIndex& parent = QModelIndex()) const
        noexcept final;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        noexcept final;
    QModelIndex index(
        int row,
        int column,
        const QModelIndex& parent = QModelIndex()) const noexcept final;
    QModelIndex parent(const QModelIndex& index) const noexcept final;
    QHash<int, QByteArray> roleNames() const noexcept final;
    int rowCount(const QModelIndex& parent = QModelIndex()) const
        noexcept final;

    const AccountList& operator*() const noexcept;

    // Throws std::runtime_error if callback returns invalid pointer
    AccountListQt(ConstructorCallback cb) noexcept(false);
    ~AccountListQt() final;

signals:
    void updated() const;

private:
    std::unique_ptr<implementation::AccountList> parent_;

    void notify() const noexcept;
    void finish_row_add() noexcept;
    void finish_row_delete() noexcept;
    void start_row_add(const QModelIndex& parent, int first, int last) noexcept;
    void start_row_delete(
        const QModelIndex& parent,
        int first,
        int last) noexcept;

    AccountListQt() = delete;
    AccountListQt(const AccountListQt&) = delete;
    AccountListQt(AccountListQt&&) = delete;
    AccountListQt& operator=(const AccountListQt&) = delete;
    AccountListQt& operator=(AccountListQt&&) = delete;
};
#endif
#endif
