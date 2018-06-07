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

#ifndef OPENTXS_UI_PROFILE_SUBSECTION_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_SUBSECTION_BLANK_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/ui/ProfileSubsection.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ProfileSubsectionBlank : virtual public ui::ProfileSubsection,
                               virtual public opentxs::ui::Widget
{
public:
    bool AddItem(const std::string&, const bool, const bool) const override
    {
        return false;
    }
    bool Delete(const std::string& claimID) const override { return false; }
    OTUIProfileItem First() const override
    {
        const std::shared_ptr<const ui::ProfileItem> empty;

        return OTUIProfileItem{empty};
    }
    bool Last() const override { return true; }
    std::string Name(const std::string& lang) const override { return {}; }
    OTUIProfileItem Next() const override
    {
        const std::shared_ptr<const ui::ProfileItem> empty;

        return OTUIProfileItem{empty};
    }
    proto::ContactItemType Type() const override { return {}; }
    bool SetActive(const std::string&, const bool) const override
    {
        return false;
    }
    bool SetPrimary(const std::string&, const bool) const override
    {
        return false;
    }
    bool SetValue(const std::string&, const std::string&) const override
    {
        return false;
    }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void Update(const opentxs::ContactGroup& group) override {}

    ~ProfileSubsectionBlank() = default;

protected:
    ProfileSubsectionBlank() = default;

private:
    friend opentxs::ui::implementation::ProfileSection;

    ProfileSubsectionBlank(const ProfileSubsectionBlank&) = delete;
    ProfileSubsectionBlank(ProfileSubsectionBlank&&) = delete;
    ProfileSubsectionBlank& operator=(const ProfileSubsectionBlank&) = delete;
    ProfileSubsectionBlank& operator=(ProfileSubsectionBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SUBSECTION_BLANK_IMPLEMENTATION_HPP
