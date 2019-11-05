// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/ui/AccountListItem.hpp"

#include "internal/api/client/Client.hpp"
#include "internal/ui/UI.hpp"
#include "Row.hpp"

#include <atomic>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>

#include "AccountListItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::AccountListItem>;

// #define OT_METHOD "opentxs::ui::implementation::AccountListItem::"

namespace opentxs
{
auto Factory::AccountListItem(
    const opentxs::PasswordPrompt& reason,
    const ui::implementation::AccountListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    const ui::implementation::CustomData& custom)
    -> ui::implementation::AccountListRowInternal*
{
    return new ui::implementation::AccountListItem(
        reason, parent, api, publisher, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
AccountListItem::AccountListItem(
    const opentxs::PasswordPrompt& reason,
    const AccountListInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const AccountListRowID& rowID,
    const AccountListSortKey& sortKey,
    const CustomData& custom) noexcept
    : AccountListItemRow(parent, api, publisher, rowID, true)
    , type_(AccountType::Custodial)
    , unit_(sortKey.first)
    , balance_(extract_custom<Amount>(custom, 0))
    , contract_(load_unit(api_, extract_custom<OTUnitID>(custom, 1), reason))
    , notary_(load_server(api_, api.Factory().ServerID(sortKey.second), reason))
    , name_(extract_custom<std::string>(custom, 2))
{
}

auto AccountListItem::DisplayBalance() const noexcept -> std::string
{
    std::string output{};
    const auto formatted =
        contract_->FormatAmountLocale(balance_, output, ",", ".");

    if (formatted) { return output; }

    return std::to_string(balance_);
}

auto AccountListItem::load_server(
    const api::Core& api,
    const identifier::Server& id,
    const PasswordPrompt& reason) -> OTServerContract
{
    try {

        return api.Wallet().Server(id, reason);
    } catch (...) {

        return api.Factory().ServerContract();
    }
}

auto AccountListItem::load_unit(
    const api::Core& api,
    const identifier::UnitDefinition& id,
    const PasswordPrompt& reason) -> OTUnitDefinition
{
    try {

        return api.Wallet().UnitDefinition(id, reason);
    } catch (...) {

        return api.Factory().UnitDefinition();
    }
}

#if OT_QT
auto AccountListItem::qt_data(const int column, int role) const noexcept
    -> QVariant
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
                    [[fallthrough]];
                }
            }
        }
        default: {
        }
    }
    return {};
}
#endif
}  // namespace opentxs::ui::implementation
