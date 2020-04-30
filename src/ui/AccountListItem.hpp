// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/AccountListItem.cpp"

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "ui/Row.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class AccountListItem;
}  // namespace ui

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountListItemRow =
    Row<AccountListRowInternal, AccountListInternalInterface, AccountListRowID>;

class AccountListItem final : public AccountListItemRow
{
public:
    std::string AccountID() const noexcept final { return row_id_->str(); }
    Amount Balance() const noexcept final { return balance_; }
    std::string ContractID() const noexcept final
    {
        return contract_->ID()->str();
    }
    std::string DisplayBalance() const noexcept final;
    std::string DisplayUnit() const noexcept final { return contract_->TLA(); }
    std::string Name() const noexcept final { return name_; }
    std::string NotaryID() const noexcept final { return notary_->ID()->str(); }
    std::string NotaryName() const noexcept final
    {
        return notary_->EffectiveName();
    }
    void reindex(
        const AccountListSortKey& key,
        const CustomData& custom) noexcept final
    {
    }
    AccountType Type() const noexcept final { return type_; }
    proto::ContactItemType Unit() const noexcept final { return unit_; }

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept final;
#endif

    AccountListItem(
        const AccountListInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        const CustomData& custom) noexcept;

    ~AccountListItem() = default;

private:
    friend opentxs::Factory;

    const AccountType type_;
    const proto::ContactItemType unit_;
    const Amount balance_;
    const OTUnitDefinition contract_;
    const OTServerContract notary_;
    const std::string name_;

    static OTServerContract load_server(
        const api::Core& api,
        const identifier::Server& id);
    static OTUnitDefinition load_unit(
        const api::Core& api,
        const identifier::UnitDefinition& id);

    AccountListItem() = delete;
    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    AccountListItem& operator=(const AccountListItem&) = delete;
    AccountListItem& operator=(AccountListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountListItem>;
