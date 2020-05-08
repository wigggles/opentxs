// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/TransferBalanceItem.cpp"

#pragma once

#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Item.hpp"
#include "ui/BalanceItem.hpp"

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
}  // namespace api

namespace identifier
{
class Nym;
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

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class TransferBalanceItem final : public BalanceItem
{
public:
    opentxs::Amount Amount() const noexcept final { return effective_amount(); }
    std::string Memo() const noexcept final;
    std::string UUID() const noexcept final;
    std::string Workflow() const noexcept final { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept final;

    TransferBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID) noexcept;
    ~TransferBalanceItem() = default;

private:
    std::unique_ptr<const opentxs::Item> transfer_;

    opentxs::Amount effective_amount() const noexcept final;
    bool get_contract() const noexcept final;

    void startup(const CustomData& custom) noexcept;

    TransferBalanceItem() = delete;
    TransferBalanceItem(const TransferBalanceItem&) = delete;
    TransferBalanceItem(TransferBalanceItem&&) = delete;
    TransferBalanceItem& operator=(const TransferBalanceItem&) = delete;
    TransferBalanceItem& operator=(TransferBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
