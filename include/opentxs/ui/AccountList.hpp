// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLIST_HPP
#define OPENTXS_UI_ACCOUNTLIST_HPP

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
        const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem> Next()
        const = 0;

    EXPORT virtual ~AccountList() = default;

protected:
    AccountList() = default;

private:
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    AccountList& operator=(const AccountList&) = delete;
    AccountList& operator=(AccountList&&) = delete;
};

#if OT_QT
class AccountListQt : public QAbstractItemModel
{
public:
    using ConstructorCallback = std::function<
        implementation::AccountList*(RowCallbacks insert, RowCallbacks remove)>;

    enum Roles {
        IDRole = Qt::UserRole + 1,
        BalancePolarityRole = Qt::UserRole + 2,
        ContractIDRole = Qt::UserRole + 3,
        DisplayBalanceRole = Qt::UserRole + 4,
        DisplayUnit = Qt::UserRole + 5,
        NameRole = Qt::UserRole + 6,
        NotaryIDRole = Qt::UserRole + 7,
        NotaryNameRole = Qt::UserRole + 8,
        TypeRole = Qt::UserRole + 9,
        UnitRole = Qt::UserRole + 10,
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

    const AccountList& operator*() const;

    AccountListQt(ConstructorCallback cb);
    ~AccountListQt() override;

signals:
    void updated() const;

private:
    Q_OBJECT

    std::unique_ptr<implementation::AccountList> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    AccountListQt() = delete;
    AccountListQt(const AccountListQt&) = delete;
    AccountListQt(AccountListQt&&) = delete;
    AccountListQt& operator=(const AccountListQt&) = delete;
    AccountListQt& operator=(AccountListQt&&) = delete;
};
#endif
}  // namespace ui
}  // namespace opentxs
#endif
