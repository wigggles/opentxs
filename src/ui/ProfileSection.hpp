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

#ifndef OPENTXS_UI_PROFILE_SECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_SECTION_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileSectionPimpl = std::unique_ptr<opentxs::ui::ProfileSubsection>;
using ProfileSectionIDType =
    std::pair<proto::ContactSectionName, proto::ContactItemType>;
using ProfileSectionSortKey = int;
using ProfileSectionInner = std::map<ProfileSectionIDType, ProfileSectionPimpl>;
using ProfileSectionOuter =
    std::map<ProfileSectionSortKey, ProfileSectionInner>;
using ProfileSectionReverse =
    std::map<ProfileSectionIDType, ProfileSectionSortKey>;
using ProfileSectionType = List<
    opentxs::ui::ProfileSection,
    ProfileSectionParent,
    opentxs::ui::ProfileSubsection,
    ProfileSectionIDType,
    ProfileSectionPimpl,
    ProfileSectionInner,
    ProfileSectionSortKey,
    ProfileSectionOuter,
    ProfileSectionOuter::const_iterator,
    ProfileSectionReverse>;
using ProfileSectionRowType = RowType<
    opentxs::ui::ProfileSection,
    ProfileParent,
    proto::ContactSectionName>;

class ProfileSection : public ProfileSectionType, public ProfileSectionRowType
{
public:
    bool AddClaim(
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override;
    bool Delete(const int type, const std::string& claimID) const override;
    ItemTypeList Items(const std::string& lang) const override;
    std::string Name(const std::string& lang) const override;
    const Identifier& NymID() const override;
    bool SetActive(
        const int type,
        const std::string& claimID,
        const bool active) const override;
    bool SetPrimary(
        const int type,
        const std::string& claimID,
        const bool primary) const override;
    bool SetValue(
        const int type,
        const std::string& claimID,
        const std::string& value) const override;
    proto::ContactSectionName Type() const override { return id_; }

    void Update(const opentxs::ContactSection& section) override;

    ~ProfileSection() = default;

private:
    friend Factory;

    static int sort_key(const ProfileSectionIDType type);
    static bool check_type(const ProfileSectionIDType type);
    static const opentxs::ContactGroup& recover(const void* input);

    const api::client::Wallet& wallet_;

    ProfileSectionIDType blank_id() const override
    {
        return {proto::CONTACTSECTION_ERROR, proto::CITEMTYPE_ERROR};
    }
    void construct_item(
        const ProfileSectionIDType& id,
        const ProfileSectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileSectionIDType& id) const override
    {
        return ProfileSectionType::last(id);
    }
    ProfileSectionOuter::const_iterator outer_first() const override
    {
        return items_.begin();
    }
    ProfileSectionOuter::const_iterator outer_end() const override
    {
        return items_.end();
    }
    std::set<ProfileSectionIDType> process_section(
        const opentxs::ContactSection& section);
    void startup(const opentxs::ContactSection section);
    void update(ProfileSectionPimpl& row, const CustomData& custom)
        const override;

    ProfileSection(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ProfileParent& parent,
        const opentxs::ContactSection& section);
    ProfileSection() = delete;
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    ProfileSection& operator=(const ProfileSection&) = delete;
    ProfileSection& operator=(ProfileSection&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SECTION_IMPLEMENTATION_HPP
