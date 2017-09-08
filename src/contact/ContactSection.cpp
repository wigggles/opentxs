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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/contact/ContactSection.hpp"

#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Log.hpp"

//#define OT_METHOD "opentxs::ContactSection::"

namespace opentxs
{
ContactSection::ContactSection(
    const std::string& nym,
    const proto::ContactSectionName section,
    const GroupMap& groups,
    const std::uint32_t version)
    : version_(version)
    , nym_(nym)
    , section_(section)
    , groups_(groups)
{
}

ContactSection::ContactSection(
    const std::string& nym,
    const proto::ContactSectionName section,
    const std::shared_ptr<ContactItem>& item,
    const std::uint32_t version)
    : ContactSection(nym, section, create_group(nym, section, item), version)
{
}

ContactSection::ContactSection(
    const std::string& nym,
    const proto::ContactSection& serialized)
    : ContactSection(
          nym,
          serialized.name(),
          extract_groups(nym, serialized),
          serialized.version())
{
}

ContactSection ContactSection::operator+(const ContactSection& rhs) const
{
    auto map = groups_;

    for (auto& it : rhs.groups_) {
        auto& rhsID = it.first;
        auto& rhsGroup = it.second;

        OT_ASSERT(rhsGroup);

        auto lhs = map.find(rhsID);
        const bool exists = (map.end() != lhs);

        if (exists) {
            auto& group = lhs->second;

            OT_ASSERT(group);

            group.reset(new ContactGroup(*group + *rhsGroup));
        } else {
            map[rhsID] = rhsGroup;
        }
    }

    return ContactSection(nym_, section_, map, version_);
}

ContactSection ContactSection::add_scope(
    const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    auto scope = item;

    if (false == scope->isPrimary()) {
        scope.reset(new ContactItem(scope->SetPrimary(true)));
    }

    if (false == scope->isActive()) {
        scope.reset(new ContactItem(scope->SetActive(true)));
    }

    GroupMap groups{};
    const auto& groupID = scope->Type();
    const auto& claimID = scope->ID();
    groups[groupID].reset(new ContactGroup(nym_, section_, claimID, scope));

    return ContactSection(nym_, section_, groups);
}

ContactSection ContactSection::AddItem(
    const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    const bool specialCaseScope = (proto::CONTACTSECTION_SCOPE == section_);

    if (specialCaseScope) {

        return add_scope(item);
    }

    const auto& groupID = item->Type();
    const bool groupExists = groups_.count(groupID);
    auto map = groups_;

    if (groupExists) {
        auto& existing = map.at(groupID);

        OT_ASSERT(existing);

        existing.reset(new ContactGroup(existing->AddItem(item)));
    } else {
        const auto& id = item->ID();
        Identifier primary{};

        if (item->isPrimary()) {
            primary = id;
        }

        map[groupID].reset(new ContactGroup(nym_, section_, primary, item));
    }

    return ContactSection(nym_, section_, map);
}

ContactSection::GroupMap::const_iterator ContactSection::begin() const
{
    return groups_.cbegin();
}

std::shared_ptr<ContactItem> ContactSection::Claim(Identifier& item) const
{
    for (const auto& group : groups_) {
        OT_ASSERT(group.second);

        auto claim = group.second->Claim(item);

        if (claim) {

            return claim;
        }
    }

    return {};
}

ContactSection::GroupMap ContactSection::create_group(
    const std::string& nym,
    const proto::ContactSectionName section,
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(item);

    GroupMap output{};
    const auto& itemType = item->Type();
    Identifier primary{};

    if (item->isPrimary()) {
        primary = item->ID();
    }

    output[itemType].reset(new ContactGroup(nym, section, primary, item));

    return output;
}

ContactSection ContactSection::Delete(const Identifier& id)
{
    bool deleted{false};
    auto map = groups_;

    for (auto& it : map) {
        auto& group = it.second;

        OT_ASSERT(group);

        if (group->HaveClaim(id)) {
            group.reset(new ContactGroup(group->Delete(id)));
            deleted = true;

            if (0 == group->Size()) {
                map.erase(it.first);
            }

            break;
        }
    }

    if (false == deleted) {

        return *this;
    }

    return ContactSection(nym_, section_, map, version_);
}

ContactSection::GroupMap::const_iterator ContactSection::end() const
{
    return groups_.cend();
}

ContactSection::GroupMap ContactSection::extract_groups(
    const std::string& nym,
    const proto::ContactSection& serialized)
{
    GroupMap groupMap{};
    std::map<proto::ContactItemType, Identifier> primaryMap{};
    std::map<proto::ContactItemType, ContactGroup::ItemMap> itemMaps{};
    const auto& section = serialized.name();

    for (const auto& item : serialized.item()) {
        const auto& itemType = item.type();
        auto instantiated = std::make_shared<ContactItem>(nym, section, item);

        OT_ASSERT(instantiated);

        const auto& itemID = instantiated->ID();
        auto& itemMap = itemMaps[itemType];
        itemMap[itemID] = instantiated;

        if (instantiated->isPrimary()) {
            primaryMap[itemType] = itemID;
        }
    }

    for (const auto& itemMap : itemMaps) {
        const auto& type = itemMap.first;
        const auto& map = itemMap.second;
        const auto& primary = primaryMap[type];
        auto& group = groupMap[type];
        group.reset(new ContactGroup(nym, section, type, primary, map));
    }

    return groupMap;
}

std::shared_ptr<ContactGroup> ContactSection::Group(
    const proto::ContactItemType& type) const
{
    const auto it = groups_.find(type);

    if (groups_.end() == it) {

        return {};
    }

    return it->second;
}

bool ContactSection::HaveClaim(const Identifier& item) const
{
    for (const auto& group : groups_) {
        OT_ASSERT(group.second);

        if (group.second->HaveClaim(item)) {

            return true;
        }
    }

    return false;
}

bool ContactSection::SerializeTo(
    proto::ContactData& section,
    const bool withIDs) const
{
    bool output = true;
    auto& serialized = *section.add_section();
    serialized.set_version(version_);
    serialized.set_name(section_);

    for (const auto& it : groups_) {
        const auto& group = it.second;

        OT_ASSERT(group);

        output &= group->SerializeTo(serialized, withIDs);
    }

    return output;
}

std::size_t ContactSection::Size() const { return groups_.size(); }
}  // namespace opentxs
