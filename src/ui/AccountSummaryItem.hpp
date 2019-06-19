// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInternal, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem final : public AccountSummaryItemRow
{
public:
    std::string AccountID() const override { return account_id_.str(); }
    Amount Balance() const override { return balance_.load(); }
    std::string DisplayBalance() const override;
    std::string Name() const override;

    void reindex(const IssuerItemSortKey& key, const CustomData& custom)
        override;

    ~AccountSummaryItem() = default;

private:
    friend opentxs::Factory;

    const Identifier& account_id_;
    const proto::ContactItemType& currency_;
    mutable std::atomic<Amount> balance_{0};
    IssuerItemSortKey name_{""};
    mutable std::shared_ptr<const UnitDefinition> contract_{nullptr};

    AccountSummaryItem(
        const PasswordPrompt& reason,
        const IssuerItemInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const IssuerItemRowID& rowID,
        const IssuerItemSortKey& sortKey,
        const CustomData& custom);
    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    AccountSummaryItem& operator=(const AccountSummaryItem&) = delete;
    AccountSummaryItem& operator=(AccountSummaryItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
