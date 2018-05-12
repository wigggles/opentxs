/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_UI_BALANCE_ITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_BALANCE_ITEM_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using BalanceItemType =
    Row<opentxs::ui::BalanceItem, AccountActivityParent, AccountActivityID>;

class BalanceItem : public BalanceItemType
{
public:
    std::string Text() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override { return type_; }

    virtual ~BalanceItem() override;

protected:
    const api::client::Sync& sync_;
    const api::client::Wallet& wallet_;
    const OTIdentifier nym_id_;
    const StorageBox type_{StorageBox::UNKNOWN};
    std::string text_{""};
    std::chrono::system_clock::time_point time_;
    std::unique_ptr<std::thread> startup_{nullptr};

    static StorageBox extract_type(const proto::PaymentWorkflow& workflow);

    virtual void startup(
        const proto::PaymentWorkflow workflow,
        const proto::PaymentEvent event) = 0;

    BalanceItem(
        const AccountActivityParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const proto::PaymentWorkflow& workflow,
        const proto::PaymentEvent& event,
        const Identifier& nymID,
        const Identifier& accountID);

private:
    const OTIdentifier account_id_;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    BalanceItem& operator=(const BalanceItem&) = delete;
    BalanceItem& operator=(BalanceItem&&) = delete;
};

class ChequeBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const override;
    std::string DisplayAmount() const override;
    std::string Memo() const override;

    void Update(
        const proto::PaymentWorkflow& workflow,
        const proto::PaymentEvent& event) override;

    ~ChequeBalanceItem() override = default;

private:
    friend Factory;
    std::unique_ptr<const opentxs::Cheque> cheque_{nullptr};
    mutable std::shared_ptr<const UnitDefinition> contract_{nullptr};

    opentxs::Amount effective_amount() const;
    bool get_contract() const;
    std::string get_contact_name(const Identifier& nymID) const;

    void startup(
        const proto::PaymentWorkflow workflow,
        const proto::PaymentEvent event) override;

    ChequeBalanceItem(
        const AccountActivityParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Sync& sync,
        const api::client::Wallet& wallet,
        const proto::PaymentWorkflow& workflow,
        const proto::PaymentEvent& event,
        const Identifier& nymID,
        const Identifier& accountID);

    ChequeBalanceItem() = delete;
    ChequeBalanceItem(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem(ChequeBalanceItem&&) = delete;
    ChequeBalanceItem& operator=(const ChequeBalanceItem&) = delete;
    ChequeBalanceItem& operator=(ChequeBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_BALANCE_ITEM_IMPLEMENTATION_HPP
