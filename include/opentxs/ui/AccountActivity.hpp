// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTACTIVITY_HPP
#define OPENTXS_UI_ACCOUNTACTIVITY_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIAccountActivity) opentxs::ui::AccountActivity;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation

class AccountActivity : virtual public List
{
public:
    EXPORT virtual Amount Balance() const = 0;
    EXPORT virtual int BalancePolarity() const = 0;
    EXPORT virtual std::string DisplayBalance() const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem> First()
        const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem> Next()
        const = 0;

    EXPORT virtual ~AccountActivity() = default;

protected:
    AccountActivity() = default;

private:
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    AccountActivity& operator=(const AccountActivity&) = delete;
    AccountActivity& operator=(AccountActivity&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::AccountActivityQt : public QAbstractItemModel
{
    Q_OBJECT
    
public:
    using ConstructorCallback = std::function<implementation::AccountActivity*(
        RowCallbacks insert,
        RowCallbacks remove)>;

    enum Roles {
        IDRole = Qt::UserRole + 1,
        AmountPolarityRole = Qt::UserRole + 2,
        ContactsRole = Qt::UserRole + 3,
        DisplayAmountRole = Qt::UserRole + 4,
        MemoRole = Qt::UserRole + 5,
        WorkflowRole = Qt::UserRole + 6,
        TextRole = Qt::UserRole + 7,
        TimestampRole = Qt::UserRole + 8,
        TypeRole = Qt::UserRole + 9,
    };

    int balancePolarity() const;
    QString displayBalance() const;

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

    const AccountActivity& operator*() const;

    AccountActivityQt(ConstructorCallback cb);
    ~AccountActivityQt() override;

signals:
    void updated() const;

private:
    Q_PROPERTY(int balancePolarity READ balancePolarity NOTIFY updated)
    Q_PROPERTY(QString displayBalance READ displayBalance NOTIFY updated)

    std::unique_ptr<implementation::AccountActivity> parent_;

    void notify() const;
    void finish_row_add();
    void finish_row_delete();
    void start_row_add(const QModelIndex& parent, int first, int last);
    void start_row_delete(const QModelIndex& parent, int first, int last);

    AccountActivityQt() = delete;
    AccountActivityQt(const AccountActivityQt&) = delete;
    AccountActivityQt(AccountActivityQt&&) = delete;
    AccountActivityQt& operator=(const AccountActivityQt&) = delete;
    AccountActivityQt& operator=(AccountActivityQt&&) = delete;
};
#endif
#endif
