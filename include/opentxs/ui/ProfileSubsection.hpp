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

#ifndef OPENTXS_UI_PROFILE_SUBSECTION_HPP
#define OPENTXS_UI_PROFILE_SUBSECTION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/ui/ListRow.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%extend opentxs::ui::ProfileSubsection {
    int Type() const
    {
        return static_cast<int>($self->Type());
    }
}
%ignore opentxs::ui::ProfileSubsection::Type;
%ignore opentxs::ui::ProfileSubsection::Update;
%template(OTUIProfileSubsection) opentxs::SharedPimpl<opentxs::ui::ProfileSubsection>;
%rename(UIProfileSubsection) opentxs::ui::ProfileSubsection;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class ProfileSubsection : virtual public ListRow
{
public:
    EXPORT virtual bool AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const = 0;
    EXPORT virtual bool Delete(const std::string& claimID) const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ProfileItem> First()
        const = 0;
    EXPORT virtual std::string Name(const std::string& lang) const = 0;
    EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ProfileItem> Next()
        const = 0;
    EXPORT virtual bool SetActive(const std::string& claimID, const bool active)
        const = 0;
    EXPORT virtual bool SetPrimary(
        const std::string& claimID,
        const bool primary) const = 0;
    EXPORT virtual bool SetValue(
        const std::string& claimID,
        const std::string& value) const = 0;
    EXPORT virtual proto::ContactItemType Type() const = 0;

    virtual void Update(const opentxs::ContactGroup& group) = 0;

    EXPORT virtual ~ProfileSubsection() = default;

protected:
    ProfileSubsection() = default;

private:
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    ProfileSubsection& operator=(const ProfileSubsection&) = delete;
    ProfileSubsection& operator=(ProfileSubsection&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif  // OPENTXS_UI_PROFILE_SUBSECTION_HPP
