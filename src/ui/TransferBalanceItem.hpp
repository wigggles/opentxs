// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class TransferBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const noexcept final { return effective_amount(); }
    std::string Memo() const noexcept final;
    std::string UUID() const noexcept final;
    std::string Workflow() const noexcept final { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept final;

    ~TransferBalanceItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<const opentxs::Item> transfer_;

    opentxs::Amount effective_amount() const noexcept final;
    bool get_contract(const PasswordPrompt& reason) const noexcept final;

    void startup(const CustomData& custom) noexcept;

    TransferBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID) noexcept;

    TransferBalanceItem() = delete;
    TransferBalanceItem(const TransferBalanceItem&) = delete;
    TransferBalanceItem(TransferBalanceItem&&) = delete;
    TransferBalanceItem& operator=(const TransferBalanceItem&) = delete;
    TransferBalanceItem& operator=(TransferBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
