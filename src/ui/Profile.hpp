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

#ifndef OPENTXS_UI_PROFILE_IMPLEMENTATION_HPP
#define OPENTXS_UI_PROFILE_IMPLEMENTATION_HPP

#include "opentxs/Internal.hpp"

namespace opentxs::ui::implementation
{
using ProfilePimpl = std::unique_ptr<opentxs::ui::ProfileSection>;
using ProfileIDType = proto::ContactSectionName;
using ProfileSortKey = int;
using ProfileInner = std::map<ProfileIDType, ProfilePimpl>;
using ProfileOuter = std::map<ProfileSortKey, ProfileInner>;
using ProfileReverse = std::map<ProfileIDType, ProfileSortKey>;
using ProfileType = List<
    opentxs::ui::Profile,
    ProfileParent,
    opentxs::ui::ProfileSection,
    ProfileIDType,
    ProfilePimpl,
    ProfileInner,
    ProfileSortKey,
    ProfileOuter,
    ProfileOuter::const_iterator,
    ProfileReverse>;

class Profile : virtual public ProfileType
{
public:
    bool AddClaim(
        const proto::ContactSectionName section,
        const proto::ContactItemType type,
        const std::string& value,
        const bool primary,
        const bool active) const override;
    ItemTypeList AllowedItems(
        const proto::ContactSectionName section,
        const std::string& lang) const override;
    SectionTypeList AllowedSections(const std::string& lang) const override;
    bool Delete(const int section, const int type, const std::string& claimID)
        const override;
    std::string DisplayName() const override;
    const Identifier& NymID() const override;
    std::string ID() const override { return nym_id_->str(); }
    std::string PaymentCode() const override;
    bool SetActive(
        const int section,
        const int type,
        const std::string& claimID,
        const bool active) const override;
    bool SetPrimary(
        const int section,
        const int type,
        const std::string& claimID,
        const bool primary) const override;
    bool SetValue(
        const int section,
        const int type,
        const std::string& claimID,
        const std::string& value) const override;

    ~Profile() = default;

private:
    friend Factory;

    const api::client::Wallet& wallet_;
    std::string name_;
    std::string payment_code_;
    OTZMQListenCallback nym_subscriber_callback_;
    OTZMQSubscribeSocket nym_subscriber_;

    static const std::set<proto::ContactSectionName> allowed_types_;
    static const std::map<proto::ContactSectionName, int> sort_keys_;

    static int sort_key(const proto::ContactSectionName type);
    static bool check_type(const proto::ContactSectionName type);
    static const opentxs::ContactSection& recover(const void* input);
    static std::string nym_name(
        const api::client::Wallet& wallet,
        const Identifier& nymID);

    ProfileIDType blank_id() const override
    {
        return proto::CONTACTSECTION_ERROR;
    }
    void construct_item(
        const ProfileIDType& id,
        const ProfileSortKey& index,
        void* custom = nullptr) const override;

    bool last(const ProfileIDType& id) const override
    {
        return ProfileType::last(id);
    }
    ProfileOuter::const_iterator outer_first() const override
    {
        return items_.begin();
    }
    ProfileOuter::const_iterator outer_end() const override
    {
        return items_.end();
    }
    void update(ProfilePimpl& row, const void* custom) const override;

    void process_nym(const Nym& nym);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    Profile(
        const network::zeromq::Context& zmq,
        const api::ContactManager& contact,
        const api::client::Wallet& wallet,
        const Identifier& nymID);
    Profile() = delete;
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    Profile& operator=(const Profile&) = delete;
    Profile& operator=(Profile&&) = delete;
};
}  // opentxs::ui::implementation
#endif  // OPENTXS_UI_PROFILE_IMPLEMENTATION_HPP
