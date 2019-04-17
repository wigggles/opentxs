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
class AccountList : virtual public List
{
#if OT_QT
    Q_OBJECT

public:
    enum AccountListRoles {
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
#endif

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
}  // namespace ui
}  // namespace opentxs
#endif
