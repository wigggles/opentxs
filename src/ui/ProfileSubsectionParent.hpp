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

#ifndef OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP
#define OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP

#include "Internal.hpp"

#include <string>

namespace opentxs::ui::implementation
{
class ProfileSubsectionParent
{
public:
    using ProfileSubsectionRowIDType = OTIdentifier;
    using ProfileSubsectionSortKey = int;

    virtual bool last(const ProfileSubsectionRowIDType& id) const = 0;
    virtual const Identifier& NymID() const = 0;
    virtual void reindex_item(
        const ProfileSubsectionRowIDType& id,
        const ProfileSubsectionSortKey& newIndex) const = 0;
    virtual proto::ContactSectionName Section() const = 0;
    virtual proto::ContactItemType Type() const = 0;
    virtual OTIdentifier WidgetID() const = 0;

    virtual ~ProfileSubsectionParent() = default;

protected:
    ProfileSubsectionParent() = default;
    ProfileSubsectionParent(const ProfileSubsectionParent&) = delete;
    ProfileSubsectionParent(ProfileSubsectionParent&&) = delete;
    ProfileSubsectionParent& operator=(const ProfileSubsectionParent&) = delete;
    ProfileSubsectionParent& operator=(ProfileSubsectionParent&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SUBSECTION_PARENT_HPP
