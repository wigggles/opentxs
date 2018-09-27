// Copyright (c) 2018 The Open-Transactions developers
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
    static const proto::PaymentEvent& recover_event(const CustomData& custom);
    static const proto::PaymentWorkflow& recover_workflow(
        const CustomData& custom);

    std::vector<std::string> Contacts() const override { return contacts_; }
    std::string DisplayAmount() const override;
    std::string Text() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override { return type_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) override;

    virtual ~BalanceItem() override;

protected:
    const OTIdentifier nym_id_;
    const std::string workflow_{""};
    const StorageBox type_{StorageBox::UNKNOWN};
    std::string text_{""};
    std::chrono::system_clock::time_point time_;
    mutable std::shared_ptr<const UnitDefinition> contract_{nullptr};
    std::unique_ptr<std::thread> startup_{nullptr};

    static StorageBox extract_type(const proto::PaymentWorkflow& workflow);

    std::string get_contact_name(const Identifier& nymID) const;

    BalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const Identifier& nymID,
        const Identifier& accountID);

private:
    const OTIdentifier account_id_;
    const std::vector<std::string> contacts_;

    static std::vector<std::string> extract_contacts(
        const proto::PaymentWorkflow& workflow);

    virtual opentxs::Amount effective_amount() const = 0;
    virtual bool get_contract() const = 0;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    BalanceItem& operator=(const BalanceItem&) = delete;
    BalanceItem& operator=(BalanceItem&&) = delete;
};

class ChequeBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const override { return effective_amount(); }
    std::string Memo() const override;
    std::string Workflow() const override { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) override;

    ~ChequeBalanceItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<const opentxs::Cheque> cheque_{nullptr};

    opentxs::Amount effective_amount() const override;
    bool get_contract() const override;

    void startup(const CustomData& custom);

    ChequeBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const Identifier& nymID,
        const Identifier& accountID);

    ChequeBalanceItem() = delete;
    ChequeBalanceItem(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem(ChequeBalanceItem&&) = delete;
    ChequeBalanceItem& operator=(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem& operator=(ChequeBalanceItem&&) = delete;
};

class TransferBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const override { return effective_amount(); }
    std::string Memo() const override;
    std::string Workflow() const override { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) override;

    ~TransferBalanceItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<const opentxs::Item> transfer_{nullptr};

    opentxs::Amount effective_amount() const override;
    bool get_contract() const override;

    void startup(const CustomData& custom);

    TransferBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const Identifier& nymID,
        const Identifier& accountID);

    TransferBalanceItem() = delete;
    TransferBalanceItem(const TransferBalanceItem&) = delete;
    TransferBalanceItem(TransferBalanceItem&&) = delete;
    TransferBalanceItem& operator=(const TransferBalanceItem&) = delete;
    TransferBalanceItem& operator=(TransferBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
