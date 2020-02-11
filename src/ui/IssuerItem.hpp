// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using IssuerItemList = List<
    IssuerItemExternalInterface,
    IssuerItemInternalInterface,
    IssuerItemRowID,
    IssuerItemRowInterface,
    IssuerItemRowInternal,
    IssuerItemRowBlank,
    IssuerItemSortKey,
    IssuerItemPrimaryID>;
using IssuerItemRow = RowType<
    AccountSummaryRowInternal,
    AccountSummaryInternalInterface,
    AccountSummaryRowID>;

class IssuerItem final
    : public Combined<IssuerItemList, IssuerItemRow, AccountSummarySortKey>
{
public:
    bool ConnectionState() const noexcept final { return connection_.load(); }
    std::string Debug() const noexcept final;
    std::string Name() const noexcept final;
    bool Trusted() const noexcept final { return issuer_->Paired(); }

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept final;
#endif

    void reindex(
        const AccountSummarySortKey& key,
        const CustomData& custom) noexcept final;

    ~IssuerItem();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const std::string& name_;
    std::atomic<bool> connection_;
    const std::shared_ptr<const api::client::Issuer> issuer_;
    const proto::ContactItemType currency_;

    void* construct_row(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& index,
        const CustomData& custom) const noexcept final;

    void process_account(const Identifier& accountID) noexcept;
    void process_account(const network::zeromq::Message& message) noexcept;
    void refresh_accounts() noexcept;
    void startup() noexcept;

    IssuerItem(
        const AccountSummaryInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountSummaryRowID& rowID,
        const AccountSummarySortKey& sortKey,
        const CustomData& custom,
        const proto::ContactItemType currency
#if OT_QT
        ,
        const bool qt
#endif
        ) noexcept;
    IssuerItem() = delete;
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    IssuerItem& operator=(const IssuerItem&) = delete;
    IssuerItem& operator=(IssuerItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
