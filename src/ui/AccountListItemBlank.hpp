// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "internal/ui/UI.hpp"

namespace opentxs::ui::implementation
{
class AccountListItemBlank final : public AccountListRowInternal
{
public:
    std::string AccountID() const override { return {}; }
    Amount Balance() const override { return {}; }
    std::string ContractID() const override { return {}; }
    std::string DisplayBalance() const override { return {}; }
    bool Last() const override { return true; }
    std::string Name() const override { return {}; }
    std::string NotaryID() const override { return {}; }
    std::string NotaryName() const override { return {}; }
    AccountType Type() const override { return {}; }
    proto::ContactItemType Unit() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void reindex(const AccountListSortKey&, const CustomData&) override {}

    AccountListItemBlank() = default;
    ~AccountListItemBlank() = default;

private:
    AccountListItemBlank(const AccountListItemBlank&) = delete;
    AccountListItemBlank(AccountListItemBlank&&) = delete;
    AccountListItemBlank& operator=(const AccountListItemBlank&) = delete;
    AccountListItemBlank& operator=(AccountListItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
