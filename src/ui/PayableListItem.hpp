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

#ifndef OPENTXS_UI_PAYABLELISTITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_PAYABLELISTITEM_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "ContactListItem.hpp"

namespace opentxs::ui::implementation
{
class PayableListItem : virtual public opentxs::ui::PayableListItem,
                        public ContactListItem
{
public:
    std::string PaymentCode() const override;

    ~PayableListItem() = default;

private:
    friend Factory;

    using ot_super = ContactListItem;

    std::string payment_code_{""};
    const proto::ContactItemType currency_;

    void process_contact(const network::zeromq::Message& message) override;

    PayableListItem(
        const ContactListParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const Identifier& id,
        const std::string& name,
        const std::string& paymentcode,
        const proto::ContactItemType& currency);
    PayableListItem() = delete;
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    PayableListItem& operator=(const PayableListItem&) = delete;
    PayableListItem& operator=(PayableListItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PAYABLELISTITEM_IMPLEMENTATION_HPP
