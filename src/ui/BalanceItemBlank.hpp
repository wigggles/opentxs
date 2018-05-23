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

#ifndef OPENTXS_UI_BALANCE_ITEM_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_BALANCE_ITEM_BLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class BalanceItemBlank : virtual public ui::BalanceItem,
                         virtual public opentxs::ui::Widget
{
public:
    opentxs::Amount Amount() const override { return {}; }
    std::string DisplayAmount() const override { return {}; }
    bool Last() const override { return true; }
    std::string Memo() const override { return {}; }
    std::string Text() const override { return {}; }
    std::chrono::system_clock::time_point Timestamp() const override
    {
        return {};
    }
    StorageBox Type() const override { return StorageBox::UNKNOWN; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void Update(const proto::PaymentWorkflow&, const proto::PaymentEvent&)
        override
    {
    }

    ~BalanceItemBlank() = default;

private:
    friend opentxs::ui::implementation::AccountActivity;

    BalanceItemBlank() = default;
    BalanceItemBlank(const BalanceItemBlank&) = delete;
    BalanceItemBlank(BalanceItemBlank&&) = delete;
    BalanceItemBlank& operator=(const BalanceItemBlank&) = delete;
    BalanceItemBlank& operator=(BalanceItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_BALANCE_ITEM_BLANK_IMPLEMENTATION_HPP
