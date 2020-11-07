// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainselection/BlockchainSelectionItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/api/client/Client.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainSelectionItem::"

namespace opentxs::factory
{
auto BlockchainSelectionItem(
    const ui::implementation::BlockchainSelectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const ui::implementation::BlockchainSelectionRowID& rowID,
    const ui::implementation::BlockchainSelectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSelectionRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSelectionItem;

    return std::make_shared<ReturnType>(
        parent, api, blockchain, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSelectionItem::BlockchainSelectionItem(
    const BlockchainSelectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const BlockchainSelectionRowID& rowID,
    const BlockchainSelectionSortKey& sortKey,
    CustomData& custom) noexcept
    : BlockchainSelectionItemRow(parent, api, rowID, true)
    , blockchain_(blockchain)
    , testnet_(sortKey.first)
    , name_(sortKey.second)
    , enabled_(blockchain_.IsEnabled(row_id_))  // TODO race condition
    , registration_(blockchain_.RegisterForUpdates(
          row_id_,
          [this](const bool value) -> auto {
              enabled_ = value;
              UpdateNotify();

              return true;
          }))
{
}

#if OT_QT
auto BlockchainSelectionItem::qt_data(const int column, int role) const noexcept
    -> QVariant
{
    switch (role) {
        case BlockchainSelectionQt::TypeRole: {
            return static_cast<int>(static_cast<std::uint32_t>(Type()));
        }
        case Qt::DisplayRole: {
            switch (column) {
                case BlockchainSelectionQt::NameColumn: {
                    return Name().c_str();
                }
                case BlockchainSelectionQt::EnabledColumn: {
                    return IsEnabled();
                }
                case BlockchainSelectionQt::TestnetColumn: {
                    return IsTestnet();
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

BlockchainSelectionItem::~BlockchainSelectionItem()
{
    blockchain_.UnregisterForUpdates(row_id_, registration_);
}
}  // namespace opentxs::ui::implementation
