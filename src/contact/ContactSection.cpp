// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "opentxs/contact/ContactSection.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <utility>

#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::ContactSection::"

namespace opentxs
{
ContactSection::ContactSection(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const proto::ContactSectionName section,
    const GroupMap& groups)
    : api_(api)
    , version_(check_version(version, parentVersion))
    , nym_(nym)
    , section_(section)
    , groups_(groups)
{
}

ContactSection::ContactSection(const ContactSection& rhs)
    : api_(rhs.api_)
    , version_(rhs.version_)
    , nym_(rhs.nym_)
    , section_(rhs.section_)
    , groups_(rhs.groups_)
{
}

ContactSection::ContactSection(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const proto::ContactSectionName section,
    const std::shared_ptr<ContactItem>& item)
    : ContactSection(
          api,
          nym,
          version,
          parentVersion,
          section,
          create_group(nym, section, item))
{
    if (0 == version) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: malformed version. "
                                           "Setting to ")(parentVersion)(".")
            .Flush();
    }
}

ContactSection::ContactSection(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber parentVersion,
    const proto::ContactSection& serialized)
    : ContactSection(
          api,
          nym,
          serialized.version(),
          parentVersion,
          serialized.name(),
          extract_groups(api, nym, parentVersion, serialized))
{
}

ContactSection ContactSection::operator+(const ContactSection& rhs) const
{
    auto map{groups_};

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

            OT_ASSERT(group);
        } else {
            [[maybe_unused]] const auto [it, inserted] =
                map.emplace(rhsID, rhsGroup);

            OT_ASSERT(inserted);
        }
    }

    const auto version = std::max(version_, rhs.Version());

    return ContactSection(api_, nym_, version, version, section_, map);
}

ContactSection ContactSection::add_scope(
    const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    auto scope = item;

    bool needsPrimary{true};

    const auto& groupID = scope->Type();
    GroupMap groups = groups_;
    const auto& group = groups[groupID];

    if (group) { needsPrimary = (1 > group->Size()); }

    if (needsPrimary && false == scope->isPrimary()) {
        scope.reset(new ContactItem(scope->SetPrimary(true)));
    }

    if (false == scope->isActive()) {
        scope.reset(new ContactItem(scope->SetActive(true)));
    }

    groups[groupID].reset(new ContactGroup(nym_, section_, scope));

    auto version = proto::RequiredVersion(section_, item->Type(), version_);

    return ContactSection(api_, nym_, version, version, section_, groups);
}

ContactSection ContactSection::AddItem(
    const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    const bool specialCaseScope = (proto::CONTACTSECTION_SCOPE == section_);

    if (specialCaseScope) { return add_scope(item); }

    const auto& groupID = item->Type();
    const bool groupExists = groups_.count(groupID);
    auto map = groups_;

    if (groupExists) {
        auto& existing = map.at(groupID);

        OT_ASSERT(existing);

        existing.reset(new ContactGroup(existing->AddItem(item)));
    } else {
        map[groupID].reset(new ContactGroup(nym_, section_, item));
    }

    auto version = proto::RequiredVersion(section_, item->Type(), version_);

    return ContactSection(api_, nym_, version, version, section_, map);
}

ContactSection::GroupMap::const_iterator ContactSection::begin() const
{
    return groups_.cbegin();
}

VersionNumber ContactSection::check_version(
    const VersionNumber in,
    const VersionNumber targetVersion)
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

std::shared_ptr<ContactItem> ContactSection::Claim(const Identifier& item) const
{
    for (const auto& group : groups_) {
        OT_ASSERT(group.second);

        auto claim = group.second->Claim(item);

        if (claim) { return claim; }
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

    output[itemType].reset(new ContactGroup(nym, section, item));

    return output;
}

ContactSection ContactSection::Delete(const Identifier& id) const
{
    bool deleted{false};
    auto map = groups_;

    for (auto& it : map) {
        auto& group = it.second;

        OT_ASSERT(group);

        if (group->HaveClaim(id)) {
            group.reset(new ContactGroup(group->Delete(id)));
            deleted = true;

            if (0 == group->Size()) { map.erase(it.first); }

            break;
        }
    }

    if (false == deleted) { return *this; }

    return ContactSection(api_, nym_, version_, version_, section_, map);
}

ContactSection::GroupMap::const_iterator ContactSection::end() const
{
    return groups_.cend();
}

ContactSection::GroupMap ContactSection::extract_groups(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber parentVersion,
    const proto::ContactSection& serialized)
{
    GroupMap groupMap{};
    std::map<proto::ContactItemType, ContactGroup::ItemMap> itemMaps{};
    const auto& section = serialized.name();

    for (const auto& item : serialized.item()) {
        const auto& itemType = item.type();
        auto instantiated = std::make_shared<ContactItem>(
            api,
            nym,
            check_version(serialized.version(), parentVersion),
            section,
            item);

        OT_ASSERT(instantiated);

        const auto& itemID = instantiated->ID();
        auto& itemMap = itemMaps[itemType];
        itemMap.emplace(itemID, instantiated);
    }

    for (const auto& itemMap : itemMaps) {
        const auto& type = itemMap.first;
        const auto& map = itemMap.second;
        auto& group = groupMap[type];
        group.reset(new ContactGroup(nym, section, type, map));
    }

    return groupMap;
}

std::shared_ptr<ContactGroup> ContactSection::Group(
    const proto::ContactItemType& type) const
{
    const auto it = groups_.find(type);

    if (groups_.end() == it) { return {}; }

    return it->second;
}

bool ContactSection::HaveClaim(const Identifier& item) const
{
    for (const auto& group : groups_) {
        OT_ASSERT(group.second);

        if (group.second->HaveClaim(item)) { return true; }
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

const proto::ContactSectionName& ContactSection::Type() const
{
    return section_;
}

VersionNumber ContactSection::Version() const { return version_; }
}  // namespace opentxs
