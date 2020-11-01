// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "ui/BlockchainAccountListItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "internal/api/client/Client.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "ui/Widget.hpp"
#if OT_QT
#include "util/Polarity.hpp"  // IWYU pragma: keep
#endif                        // OT_QT

// #define OT_METHOD "opentxs::ui::implementation::BlockchainAccountListItem::"

namespace opentxs::factory
{
auto BlockchainAccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>
{
    using ReturnType = ui::implementation::BlockchainAccountListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountListItem::BlockchainAccountListItem(
    const AccountListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    CustomData& custom) noexcept
    : AccountListItemRow(parent, api, rowID, true)
    , type_(AccountType::Blockchain)
    , unit_(sortKey.first)
    , balance_(extract_custom<Amount>(custom, 1))
    , chain_(extract_custom<blockchain::Type>(custom, 2))
    , contract_(UnitID(api_, chain_).str())
    , notary_(sortKey.second)
    , name_(extract_custom<std::string>(custom, 3))
{
}

#if OT_QT
auto BlockchainAccountListItem::qt_data(const int column, int role)
    const noexcept -> QVariant
{
    switch (role) {
        case AccountListQt::NotaryIDRole: {
            return NotaryID().c_str();
        }
        case AccountListQt::UnitRole: {
            return static_cast<int>(Unit());
        }
        case AccountListQt::AccountIDRole: {
            return AccountID().c_str();
        }
        case AccountListQt::BalanceRole: {
            return static_cast<unsigned long long>(Balance());
        }
        case AccountListQt::PolarityRole: {
            return polarity(Balance());
        }
        case AccountListQt::AccountTypeRole: {
            return static_cast<int>(Type());
        }
        case AccountListQt::ContractIdRole: {
            return ContractID().c_str();
        }
        case Qt::DisplayRole: {
            switch (column) {
                case AccountListQt::NotaryNameColumn: {
                    return NotaryName().c_str();
                }
                case AccountListQt::DisplayUnitColumn: {
                    return DisplayUnit().c_str();
                }
                case AccountListQt::AccountNameColumn: {
                    return Name().c_str();
                }
                case AccountListQt::DisplayBalanceColumn: {
                    return DisplayBalance().c_str();
                }
                default: {
                }
            }

            [[fallthrough]];
        }
        default: {
        }
    }
    return {};
}
#endif
}  // namespace opentxs::ui::implementation
