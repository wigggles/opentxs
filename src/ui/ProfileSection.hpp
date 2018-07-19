// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_PROFILE_SECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_SECTION_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileSectionList = List<
    ProfileSectionExternalInterface,
    ProfileSectionInternalInterface,
    ProfileSectionRowID,
    ProfileSectionRowInterface,
    ProfileSectionRowBlank,
    ProfileSectionSortKey>;
using ProfileSectionRow =
    RowType<ProfileRowInterface, ProfileInternalInterface, ProfileRowID>;

class ProfileSection : public ProfileSectionList, public ProfileSectionRow
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

    static int sort_key(const ProfileSectionRowID type);
    static bool check_type(const ProfileSectionRowID type);
    static const opentxs::ContactGroup& recover(const void* input);

    const api::client::Wallet& wallet_;

    void construct_row(
        const ProfileSectionRowID& id,
        const ProfileSectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileSectionRowID& id) const override
    {
        return ProfileSectionList::last(id);
    }
    std::set<ProfileSectionRowID> process_section(
        const opentxs::ContactSection& section);
    void startup(const opentxs::ContactSection section);
    void update(ProfileSectionRowInterface& row, const CustomData& custom)
        const override;

    ProfileSection(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
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
