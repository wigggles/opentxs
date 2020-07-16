// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "ui/AccountActivity.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"

// #define OT_METHOD "opentxs::ui::implementation::AccountActivity::"

namespace opentxs::factory
{
#if OT_QT
auto AccountActivityQtModel(
    ui::implementation::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>
{
    using ReturnType = ui::AccountActivityQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(AccountActivityQt, implementation::AccountActivity)

auto AccountActivityQt::accountID() const noexcept -> QString
{
    return parent_.AccountID().c_str();
}
auto AccountActivityQt::balancePolarity() const noexcept -> int
{
    return parent_.BalancePolarity();
}
#if OT_BLOCKCHAIN
auto AccountActivityQt::depositChains() const noexcept -> QList<int>
{
    const auto input = parent_.DepositChains();
    auto output = QList<int>{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(output), [
        ](const auto& in) -> auto { return static_cast<int>(in); });

    return output;
}
#endif  // OT_BLOCKCHAIN
auto AccountActivityQt::displayBalance() const noexcept -> QString
{
    return parent_.DisplayBalance().c_str();
}
#if OT_BLOCKCHAIN
auto AccountActivityQt::getDepositAddress(const int chain) const noexcept
    -> QString
{
    return parent_.DepositAddress(static_cast<blockchain::Type>(chain)).c_str();
}
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
AccountActivity::AccountActivity(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const AccountType type,
    const SimpleCallback& cb,
#if OT_QT
    const bool qt,
#endif  // OT_QT
    ListenerDefinitions&& listeners) noexcept
    : AccountActivityList(
          api,
          nymID,
          cb
#if OT_QT
          ,
          qt,
          Roles{
              {AccountActivityQt::PolarityRole, "polarity"},
              {AccountActivityQt::ContactsRole, "contacts"},
              {AccountActivityQt::WorkflowRole, "workflow"},
              {AccountActivityQt::TypeRole, "type"},
          },
          5
#endif  // OT_QT
          )
    , listeners_(std::move(listeners))
    , balance_(0)
    , account_id_(accountID)
    , type_(type)
{
    init();
    setup_listeners(listeners_);
}

auto AccountActivity::construct_row(
    const AccountActivityRowID& id,
    const AccountActivitySortKey& index,
    CustomData& custom) const noexcept -> void*
{
    OT_ASSERT(2 <= custom.size())

    names_.emplace(id, index);
    const auto [it, added] = items_[index].emplace(
        id,
        factory::BalanceItem(
            *this,
            api_,

            id,
            index,
            custom,
            primary_id_,
            account_id_));

    return it->second.get();
}

AccountActivity::~AccountActivity()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
