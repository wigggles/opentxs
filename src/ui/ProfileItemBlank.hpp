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

#ifndef OPENTXS_UI_PROFILE_ITEM_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_ITEM_BLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ProfileItemBlank : virtual public ui::ProfileItem
{
public:
    std::string ClaimID() const override { return {}; }
    bool Delete() const override{ return false; }
    bool IsActive() const override { return false; }
    bool IsPrimary() const override { return false; }
    std::string Value() const override { return {}; }
    bool Last() const override { return true; }
    bool SetActive(const bool& active) const override { return false; }
    bool SetPrimary(const bool& primary) const override { return false; }
    bool SetValue(const std::string& value) const override { return false; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    ~ProfileItemBlank() = default;

protected:
    ProfileItemBlank() = default;

private:
    friend opentxs::ui::implementation::ProfileSubsection;

    ProfileItemBlank(const ProfileItemBlank&) = delete;
    ProfileItemBlank(ProfileItemBlank&&) = delete;
    ProfileItemBlank& operator=(const ProfileItemBlank&) = delete;
    ProfileItemBlank& operator=(ProfileItemBlank&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_ITEM_BLANK_IMPLEMENTATION_HPP
