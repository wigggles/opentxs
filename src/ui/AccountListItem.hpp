// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountListItemRow =
    Row<AccountListRowInternal, AccountListInternalInterface, AccountListRowID>;

class AccountListItem final : public AccountListItemRow
{
public:
    std::string AccountID() const override { return row_id_->str(); }
    Amount Balance() const override { return balance_; }
    std::string ContractID() const override { return contract_->ID()->str(); }
    std::string DisplayBalance() const override;
    std::string DisplayUnit() const override { return contract_->TLA(); }
    std::string Name() const override { return name_; }
    std::string NotaryID() const override { return notary_->ID()->str(); }
    std::string NotaryName() const override
    {
        auto reason = api_.Factory().PasswordPrompt("Loading notary contract");

        return notary_->EffectiveName(reason);
    }
    void reindex(const AccountListSortKey& key, const CustomData& custom)
        override{};
    AccountType Type() const override { return type_; }
    proto::ContactItemType Unit() const override { return unit_; }

    ~AccountListItem() = default;

private:
    friend opentxs::Factory;

    const AccountType type_;
    const proto::ContactItemType unit_;
    const Amount balance_;
    const std::shared_ptr<const UnitDefinition> contract_;
    const std::shared_ptr<const ServerContract> notary_;
    const std::string name_;

    AccountListItem(
        const PasswordPrompt& reason,
        const AccountListInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountListRowID& rowID,
        const AccountListSortKey& sortKey,
        const CustomData& custom);
    AccountListItem() = delete;
    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    AccountListItem& operator=(const AccountListItem&) = delete;
    AccountListItem& operator=(AccountListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
