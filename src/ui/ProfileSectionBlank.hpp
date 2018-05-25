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

#ifndef OPENTXS_UI_PROFILE_SECTION_BLANK_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_SECTION_BLANK_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/ui/ProfileSection.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs::ui::implementation
{
class ProfileSectionBlank : virtual public ui::ProfileSection,
                            virtual public opentxs::ui::Widget
{
public:
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override
    {
        return false;
    }
    bool Delete(const int, const std::string&) const override { return false; }
    std::shared_ptr<const opentxs::ui::ProfileSubsection> First() const override
    {
        return nullptr;
    }
    ItemTypeList Items(const std::string&) const override { return {}; }
    bool Last() const override { return true; }
    std::string Name(const std::string& lang) const override { return {}; }
    std::shared_ptr<const opentxs::ui::ProfileSubsection> Next() const override
    {
        return nullptr;
    }
    bool SetActive(const int, const std::string&, const bool) const override
    {
        return false;
    }
    bool SetPrimary(const int, const std::string&, const bool) const override
    {
        return false;
    }
    bool SetValue(const int, const std::string&, const std::string&)
        const override
    {
        return false;
    }
    proto::ContactSectionName Type() const override { return {}; }
    bool Valid() const override { return false; }
    OTIdentifier WidgetID() const override { return Identifier::Factory(); }

    void Update(const opentxs::ContactSection& section) override {}

    ~ProfileSectionBlank() = default;

protected:
    ProfileSectionBlank() = default;

private:
    friend opentxs::ui::implementation::Profile;

    ProfileSectionBlank(const ProfileSectionBlank&) = delete;
    ProfileSectionBlank(ProfileSectionBlank&&) = delete;
    ProfileSectionBlank& operator=(const ProfileSectionBlank&) = delete;
    ProfileSectionBlank& operator=(ProfileSectionBlank&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SECTION_BLANK_IMPLEMENTATION_HPP
