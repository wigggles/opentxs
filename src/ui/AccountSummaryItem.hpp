// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/AccountSummaryItem.cpp"

#pragma once

#include <atomic>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/AccountSummaryItem.hpp"
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
class AccountSummaryItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInternal, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem final : public AccountSummaryItemRow
{
public:
    std::string AccountID() const noexcept final { return account_id_.str(); }
    Amount Balance() const noexcept final { return balance_.load(); }
    std::string DisplayBalance() const noexcept final;
    std::string Name() const noexcept final;

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept override;
#endif

    void reindex(
        const IssuerItemSortKey& key,
        const CustomData& custom) noexcept final;

    AccountSummaryItem(
        const IssuerItemInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const IssuerItemRowID& rowID,
        const IssuerItemSortKey& sortKey,
        const CustomData& custom) noexcept;

    ~AccountSummaryItem() final = default;

private:
    const Identifier& account_id_;
    const proto::ContactItemType& currency_;
    mutable std::atomic<Amount> balance_;
    IssuerItemSortKey name_;
    mutable OTUnitDefinition contract_;

    static OTUnitDefinition load_unit(
        const api::Core& api,
        const Identifier& id);

    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    AccountSummaryItem& operator=(const AccountSummaryItem&) = delete;
    AccountSummaryItem& operator=(AccountSummaryItem&&) = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;
