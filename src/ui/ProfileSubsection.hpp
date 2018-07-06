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

#ifndef OPENTXS_UI_PROFILE_SUBSECTION_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_SUBSECTION_IMPLEMENTATION_HPP

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfileSubsectionList = List<
    ProfileSubsectionExternalInterface,
    ProfileSubsectionInternalInterface,
    ProfileSubsectionRowID,
    ProfileSubsectionRowInterface,
    ProfileSubsectionRowBlank,
    ProfileSubsectionSortKey>;
using ProfileSubsectionRow = RowType<
    ProfileSectionRowInterface,
    ProfileSectionInternalInterface,
    ProfileSectionRowID>;

class ProfileSubsection : public ProfileSubsectionList,
                          public ProfileSubsectionRow
{
public:
    bool AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const override;
    bool Delete(const std::string& claimID) const override;
    std::string Name(const std::string& lang) const override;
    const Identifier& NymID() const override { return nym_id_; }
    proto::ContactSectionName Section() const override { return id_.first; }
    bool SetActive(const std::string& claimID, const bool active)
        const override;
    bool SetPrimary(const std::string& claimID, const bool primary)
        const override;
    bool SetValue(const std::string& claimID, const std::string& value)
        const override;
    proto::ContactItemType Type() const override { return id_.second; }

    void Update(const opentxs::ContactGroup& group) override;

    ~ProfileSubsection() = default;

private:
    friend Factory;

    static bool check_type(const ProfileSubsectionRowID type);
    static const opentxs::ContactItem& recover(const void* input);

    const api::client::Wallet& wallet_;

    void construct_row(
        const ProfileSubsectionRowID& id,
        const ProfileSubsectionSortKey& index,
        const CustomData& custom) const override;

    bool last(const ProfileSubsectionRowID& id) const override
    {
        return ProfileSubsectionList::last(id);
    }
    void process_group(const opentxs::ContactGroup& group);
    int sort_key(const ProfileSubsectionRowID type) const;
    void startup(const opentxs::ContactGroup group);

    ProfileSubsection(
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const ProfileSectionParent& parent,
        const opentxs::ContactGroup& group);
    ProfileSubsection() = delete;
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    ProfileSubsection& operator=(const ProfileSubsection&) = delete;
    ProfileSubsection& operator=(ProfileSubsection&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_SUBSECTION_IMPLEMENTATION_HPP
