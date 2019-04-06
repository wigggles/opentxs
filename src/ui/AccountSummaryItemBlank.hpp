// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/AccountSummaryItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class AccountSummaryItemBlank final : public IssuerItemRowInternal
{
public:
    // AccountSummaryItem
    std::string AccountID() const override { return {}; }
    Amount Balance() const override { return {}; };
    std::string DisplayBalance() const override { return {}; };
    std::string Name() const override { return {}; };

    void reindex(
        const implementation::IssuerItemSortKey& key,
        const implementation::CustomData& custom) override
    {
    }

    // ListRow
    bool Last() const override { return {}; }
    bool Valid() const override { return {}; }

    // Widget
    void SetCallback(ui::Widget::Callback) const override {}
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    AccountSummaryItemBlank() = default;
    ~AccountSummaryItemBlank() = default;

private:
    AccountSummaryItemBlank(const AccountSummaryItemBlank&) = delete;
    AccountSummaryItemBlank(AccountSummaryItemBlank&&) = delete;
    AccountSummaryItemBlank& operator=(const AccountSummaryItemBlank&) = delete;
    AccountSummaryItemBlank& operator=(AccountSummaryItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
