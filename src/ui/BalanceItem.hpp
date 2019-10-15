// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using BalanceItemRow =
    Row<AccountActivityRowInternal,
        AccountActivityInternalInterface,
        AccountActivityRowID>;

class BalanceItem : public BalanceItemRow
{
public:
    static const proto::PaymentEvent& recover_event(
        const CustomData& custom) noexcept;
    static const proto::PaymentWorkflow& recover_workflow(
        const CustomData& custom) noexcept;

    std::vector<std::string> Contacts() const noexcept final
    {
        return contacts_;
    }
    std::string DisplayAmount() const noexcept final;
    std::string Text() const noexcept final;
    std::chrono::system_clock::time_point Timestamp() const noexcept final;
    StorageBox Type() const noexcept final { return type_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) noexcept override;

    virtual ~BalanceItem() override;

protected:
    const OTNymID nym_id_;
    const std::string workflow_{""};
    const StorageBox type_{StorageBox::UNKNOWN};
    std::string text_{""};
    std::chrono::system_clock::time_point time_;
    mutable std::shared_ptr<const UnitDefinition> contract_{nullptr};
    std::unique_ptr<std::thread> startup_{nullptr};

    static StorageBox extract_type(
        const proto::PaymentWorkflow& workflow) noexcept;

    std::string get_contact_name(const identifier::Nym& nymID) const noexcept;

    BalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID) noexcept;

private:
    const OTIdentifier account_id_;
    const std::vector<std::string> contacts_;

    static std::vector<std::string> extract_contacts(
        const api::client::internal::Manager& api,
        const proto::PaymentWorkflow& workflow) noexcept;

    virtual opentxs::Amount effective_amount() const noexcept = 0;
    virtual bool get_contract(const PasswordPrompt& reason) const noexcept = 0;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    BalanceItem& operator=(const BalanceItem&) = delete;
    BalanceItem& operator=(BalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
