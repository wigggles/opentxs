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

#ifndef OPENTXS_UI_PROFILE_ITEM_HPP
#define OPENTXS_UI_PROFILE_ITEM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/Types.hpp"

#include <string>

#include "ListRow.hpp"

#ifdef SWIG
// clang-format off
%rename(UIProfileItem) opentxs::ui::ProfileItem;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ProfileItem : virtual public ListRow
{
public:
    EXPORT virtual std::string ClaimID() const = 0;
    EXPORT virtual bool Delete() const = 0;
    EXPORT virtual bool IsActive() const = 0;
    EXPORT virtual bool IsPrimary() const = 0;
    EXPORT virtual bool SetActive(const bool& active) const = 0;
    EXPORT virtual bool SetPrimary(const bool& primary) const = 0;
    EXPORT virtual bool SetValue(const std::string& value) const = 0;
    EXPORT virtual std::string Value() const = 0;

    EXPORT virtual ~ProfileItem() = default;

protected:
    ProfileItem() = default;

private:
    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    ProfileItem& operator=(const ProfileItem&) = delete;
    ProfileItem& operator=(ProfileItem&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_PROFILE_ITEM_HPP
