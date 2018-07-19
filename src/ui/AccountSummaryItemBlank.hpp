// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTSUMMARYITEMBLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACCOUNTSUMMARYITEMBLANK_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/ui/AccountSummaryItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class AccountSummaryItemBlank : virtual public ui::AccountSummaryItem,
                                virtual public opentxs::ui::Widget
{
public:
    // AccountSummaryItem
    std::string AccountID() const override { return {}; }
    Amount Balance() const override { return {}; };
    std::string DisplayBalance() const override { return {}; };
    std::string Name() const override { return {}; };

    // ListRow
    bool Last() const override { return {}; }
    bool Valid() const override { return {}; }

    // Widget
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
#endif  // OPENTXS_UI_ACCOUNTSUMMARYITEMBLANK_IMPLEMENTATION_HPP
