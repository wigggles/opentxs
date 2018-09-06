// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "InternalUI.hpp"

namespace opentxs::ui::implementation
{
class BalanceItemBlank final : public AccountActivityRowInternal
{
public:
    opentxs::Amount Amount() const override { return {}; }
    std::vector<std::string> Contacts() const override { return {}; }
    std::string DisplayAmount() const override { return {}; }
    bool Last() const override { return true; }
    std::string Memo() const override { return {}; }
    TransactionNumber Number() const override { return {}; }
    std::string Text() const override { return {}; }
    std::chrono::system_clock::time_point Timestamp() const override
    {
        return {};
    }
    StorageBox Type() const override { return StorageBox::UNKNOWN; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(
        const implementation::AccountActivitySortKey&,
        const implementation::CustomData&) override
    {
    }

    BalanceItemBlank() = default;
    ~BalanceItemBlank() = default;

private:
    BalanceItemBlank(const BalanceItemBlank&) = delete;
    BalanceItemBlank(BalanceItemBlank&&) = delete;
    BalanceItemBlank& operator=(const BalanceItemBlank&) = delete;
    BalanceItemBlank& operator=(BalanceItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
