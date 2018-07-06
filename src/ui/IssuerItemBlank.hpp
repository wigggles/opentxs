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

#ifndef OPENTXS_UI_ISSUERITEMBLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_ISSUERITEMBLANK_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/ui/Widget.hpp"

#include "AccountSummaryItemBlank.hpp"

namespace opentxs::ui::implementation
{
class IssuerItemBlank : virtual public ui::IssuerItem,
                        virtual public opentxs::ui::Widget
{
public:
    // IssuerItem
    bool ConnectionState() const override { return {}; }
    std::string Debug() const override { return {}; }
    OTUIAccountSummaryItem First() const override
    {
        return OTUIAccountSummaryItem{
            std::make_shared<AccountSummaryItemBlank>()};
    }
    std::string Name() const override { return {}; }
    OTUIAccountSummaryItem Next() const override
    {
        return OTUIAccountSummaryItem{
            std::make_shared<AccountSummaryItemBlank>()};
    }
    bool Trusted() const override { return {}; }

    // ListRow
    bool Last() const override { return {}; }
    bool Valid() const override { return {}; }

    // Widget
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    IssuerItemBlank() = default;
    ~IssuerItemBlank() = default;

private:
    IssuerItemBlank(const IssuerItemBlank&) = delete;
    IssuerItemBlank(IssuerItemBlank&&) = delete;
    IssuerItemBlank& operator=(const IssuerItemBlank&) = delete;
    IssuerItemBlank& operator=(IssuerItemBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_ISSUERITEMBLANK_IMPLEMENTATION_HPP
