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

#ifndef OPENTXS_UI_ACCOUNTSUMMARYITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_ACCOUNTSUMMARYITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInterface, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem : public AccountSummaryItemRow
{
public:
    std::string AccountID() const override { return account_id_.str(); }
    Amount Balance() const override { return balance_.load(); }
    std::string DisplayBalance() const override;
    std::string Name() const override;

    ~AccountSummaryItem() = default;

private:
    friend Factory;

    static const Widget::ListenerDefinitions listeners_;

    const api::client::Wallet& wallet_;
    const api::storage::Storage& storage_;
    const Identifier& account_id_;
    const proto::ContactItemType& currency_;
    mutable std::atomic<Amount> balance_{0};
    std::string name_{""};
    std::shared_ptr<const UnitDefinition> contract_{nullptr};

    void process_account(const network::zeromq::Message& message);
    void update();

    AccountSummaryItem(
        const IssuerItemParent& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage,
        const api::ContactManager& contact,
        const IssuerItemRowID& id);
    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    AccountSummaryItem& operator=(const AccountSummaryItem&) = delete;
    AccountSummaryItem& operator=(AccountSummaryItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ACCOUNTSUMMARYITEM_IMPLEMENTATION_HPP
