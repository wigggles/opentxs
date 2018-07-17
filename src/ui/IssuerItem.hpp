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

#ifndef OPENTXS_UI_ISSUERITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_ISSUERITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
template <>
struct make_blank<IssuerItemRowID> {
    static IssuerItemRowID value()
    {
        return {Identifier::Factory(), proto::CITEMTYPE_ERROR};
    }
};

using IssuerItemList = List<
    IssuerItemExternalInterface,
    IssuerItemInternalInterface,
    IssuerItemRowID,
    IssuerItemRowInterface,
    IssuerItemRowBlank,
    IssuerItemSortKey>;
using IssuerItemRow = RowType<
    AccountSummaryRowInterface,
    AccountSummaryInternalInterface,
    AccountSummaryRowID>;

class IssuerItem : public IssuerItemList, public IssuerItemRow
{
public:
    bool ConnectionState() const override;
    std::string Debug() const override;
    std::string Name() const override;
    bool Trusted() const override;

    ~IssuerItem() = default;

private:
    friend Factory;

    static const ListenerDefinitions listeners_;

    const api::client::Wallet& wallet_;
    const api::storage::Storage& storage_;
    std::atomic<bool> connection_{false};
    const std::shared_ptr<const api::client::Issuer> issuer_{nullptr};
    const proto::ContactItemType currency_;

    void construct_row(
        const IssuerItemRowID& id,
        const IssuerItemSortKey& index,
        const CustomData& custom) const override;

    void process_connection(const network::zeromq::Message& message);
    void process_issuer(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void process_server(const network::zeromq::Message& message);
    void refresh_accounts();
    void startup();

    IssuerItem(
        const AccountSummaryInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const api::storage::Storage& storage,
        const proto::ContactItemType currency,
        const Identifier& id);
    IssuerItem() = delete;
    IssuerItem(const IssuerItem&) = delete;
    IssuerItem(IssuerItem&&) = delete;
    IssuerItem& operator=(const IssuerItem&) = delete;
    IssuerItem& operator=(IssuerItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ISSUERITEM_IMPLEMENTATION_HPP
