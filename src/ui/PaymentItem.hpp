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

#ifndef OPENTXS_UI_PAYMENTITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_PAYMENTITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class PaymentItem : virtual public ActivityThreadItem
{
public:
    opentxs::Amount Amount() const override;
    std::string DisplayAmount() const override;
    std::string Memo() const override;

    ~PaymentItem();

private:
    friend Factory;

    std::string display_amount_{};
    std::string memo_{};
    opentxs::Amount amount_{0};
    std::unique_ptr<std::thread> load_{nullptr};

    void load();

    PaymentItem(
        const ActivityThreadParent& parent,
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const ActivityThreadID& id,
        const Identifier& nymID,
        const api::Activity& activity,
        const std::chrono::system_clock::time_point& time);
    PaymentItem() = delete;
    PaymentItem(const PaymentItem&) = delete;
    PaymentItem(PaymentItem&&) = delete;
    PaymentItem& operator=(const PaymentItem&) = delete;
    PaymentItem& operator=(PaymentItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PAYMENTITEM_IMPLEMENTATION_HPP
