// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class ChequeBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const noexcept final { return effective_amount(); }
    std::string Memo() const noexcept final;
    std::string UUID() const noexcept final;
    std::string Workflow() const noexcept final { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept final;

    ~ChequeBalanceItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<const opentxs::Cheque> cheque_;

    opentxs::Amount effective_amount() const noexcept final;
    bool get_contract() const noexcept final;

    void startup(const CustomData& custom) noexcept;

    ChequeBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID) noexcept;

    ChequeBalanceItem() = delete;
    ChequeBalanceItem(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem(ChequeBalanceItem&&) = delete;
    ChequeBalanceItem& operator=(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem& operator=(ChequeBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
