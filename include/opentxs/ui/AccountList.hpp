// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLIST_HPP
#define OPENTXS_UI_ACCOUNTLIST_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
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

class AccountList;
class AccountListItem;

#if OT_QT
class AccountListQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class AccountList : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::AccountListItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT ~AccountList() override = default;

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
class OPENTXS_EXPORT opentxs::ui::AccountListQt final
    : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // User roles return the same data for all columns
    //
    // NotaryIDRole: QString
    // UnitRole: int (proto::ContactItemType)
    // AccountIDRole: QString
    // BalanceRole: int
    // PolarityRole: int (-1, 0, or 1)
    // AccountTypeRole: int (opentxs::AccountType)
    // ContractIdRole: QString
    //
    // Qt::DisplayRole, NotaryNameColumn: QString
    // Qt::DisplayRole, DisplayUnitColumn: QString
    // Qt::DisplayRole, AccountNameColumn: QString
    // Qt::DisplayRole, DisplayBalanceColumn: QString

    enum Columns {
        NotaryNameColumn = 0,
        DisplayUnitColumn = 1,
        AccountNameColumn = 2,
        DisplayBalanceColumn = 3,
    };
    enum Roles {
        NotaryIDRole = Qt::UserRole + 0,
        UnitRole = Qt::UserRole + 1,
        AccountIDRole = Qt::UserRole + 2,
        BalanceRole = Qt::UserRole + 3,
        PolarityRole = Qt::UserRole + 4,
        AccountTypeRole = Qt::UserRole + 5,
        ContractIdRole = Qt::UserRole + 6,
    };

    AccountListQt(implementation::AccountList& parent) noexcept;

    ~AccountListQt() final = default;

private:
    implementation::AccountList& parent_;

    void notify() const noexcept;

    AccountListQt(const AccountListQt&) = delete;
    AccountListQt(AccountListQt&&) = delete;
    AccountListQt& operator=(const AccountListQt&) = delete;
    AccountListQt& operator=(AccountListQt&&) = delete;
};
#endif
#endif
