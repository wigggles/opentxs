// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTACTIVITY_HPP
#define OPENTXS_UI_ACCOUNTACTIVITY_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
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

class AccountActivity;
class BalanceItem;

#if OT_QT
class AccountActivityQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class AccountActivity : virtual public List
{
public:
    OPENTXS_EXPORT virtual std::string AccountID() const noexcept = 0;
    OPENTXS_EXPORT virtual Amount Balance() const noexcept = 0;
    OPENTXS_EXPORT virtual int BalancePolarity() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ContractID() const noexcept = 0;
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT virtual std::string DepositAddress(
        const blockchain::Type chain =
            blockchain::Type::Unknown) const noexcept = 0;
    OPENTXS_EXPORT virtual std::vector<blockchain::Type> DepositChains()
        const noexcept = 0;
#endif  // OT_BLOCKCHAIN
    OPENTXS_EXPORT virtual std::string DisplayBalance() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string DisplayUnit() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Name() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem> Next()
        const noexcept = 0;
    OPENTXS_EXPORT virtual std::string NotaryID() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string NotaryName() const noexcept = 0;
    OPENTXS_EXPORT virtual AccountType Type() const noexcept = 0;
    OPENTXS_EXPORT virtual proto::ContactItemType Unit() const noexcept = 0;

    OPENTXS_EXPORT ~AccountActivity() override = default;

protected:
    AccountActivity() noexcept = default;

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
class opentxs::ui::AccountActivityQt final : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int balancePolarity READ balancePolarity NOTIFY updated)
#if OT_BLOCKCHAIN
    Q_PROPERTY(QList<int> depositChains READ depositChains NOTIFY updated)
#endif  // OT_BLOCKCHAIN
    Q_PROPERTY(QString displayBalance READ displayBalance NOTIFY updated)

signals:
    void updated() const;

public:
    // Table layout
    enum Roles {
        PolarityRole = Qt::UserRole + 0,
        ContactsRole = Qt::UserRole + 1,
        WorkflowRole = Qt::UserRole + 2,
        TypeRole = Qt::UserRole + 3,
    };
    enum Columns {
        AmountColumn = 0,
        TextColumn = 1,
        MemoColumn = 2,
        TimeColumn = 3,
        UUIDColumn = 4,
    };

    OPENTXS_EXPORT int balancePolarity() const noexcept;
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT QList<int> depositChains() const noexcept;
#endif  // OT_BLOCKCHAIN
    OPENTXS_EXPORT QString displayBalance() const noexcept;
#if OT_BLOCKCHAIN
    OPENTXS_EXPORT Q_INVOKABLE QString
    getDepositAddress(const int chain = 0) const noexcept;
#endif  // OT_BLOCKCHAIN

    AccountActivityQt(implementation::AccountActivity& parent) noexcept;

    ~AccountActivityQt() final = default;

private:
    friend opentxs::Factory;

    implementation::AccountActivity& parent_;

    void notify() const noexcept;

    AccountActivityQt() = delete;
    AccountActivityQt(const AccountActivityQt&) = delete;
    AccountActivityQt(AccountActivityQt&&) = delete;
    AccountActivityQt& operator=(const AccountActivityQt&) = delete;
    AccountActivityQt& operator=(AccountActivityQt&&) = delete;
};
#endif
#endif
