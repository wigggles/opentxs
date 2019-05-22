// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACTITEM_HPP
#define OPENTXS_CONTACT_CONTACTITEM_HPP

#define NULL_START 0
#define NULL_END 0

#include "opentxs/Forward.hpp"

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <chrono>
#include <cstdint>
#include <set>

namespace opentxs
{

class ContactItem
{
public:
    ContactItem(
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const proto::ContactSectionName section,
        const proto::ContactItemType& type,
        const std::string& value,
        const std::set<proto::ContactItemAttribute>& attributes,
        const std::time_t start,
        const std::time_t end);
    ContactItem(
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const Claim& claim);
    ContactItem(
        const std::string& nym,
        const VersionNumber parentVersion,
        const proto::ContactSectionName section,
        const proto::ContactItem& serialized);
    ContactItem(const ContactItem&) = default;
    ContactItem(ContactItem&&) = default;

    bool operator==(const ContactItem& rhs) const;

    // Includes IDs
    operator proto::ContactItem() const;

    const std::time_t& End() const;
    const Identifier& ID() const;
    bool isActive() const;
    bool isLocal() const;
    bool isPrimary() const;
    const proto::ContactSectionName& Section() const;
    proto::ContactItem Serialize(const bool withID = false) const;
    ContactItem SetActive(const bool active) const;
    ContactItem SetEnd(const std::time_t end) const;
    ContactItem SetLocal(const bool local) const;
    ContactItem SetPrimary(const bool primary) const;
    ContactItem SetStart(const std::time_t start) const;
    ContactItem SetValue(const std::string& value) const;
    const std::time_t& Start() const;
    const proto::ContactItemType& Type() const;
    const std::string& Value() const;
    VersionNumber Version() const;

    ~ContactItem() = default;

private:
    const VersionNumber version_{0};
    const std::string nym_{};
    const proto::ContactSectionName section_{proto::CONTACTSECTION_ERROR};
    const proto::ContactItemType type_{proto::CITEMTYPE_ERROR};
    const std::string value_;
    const std::time_t start_{0};
    const std::time_t end_{0};
    const std::set<proto::ContactItemAttribute> attributes_{};
    const OTIdentifier id_;

    static VersionNumber check_version(
        const VersionNumber in,
        const VersionNumber targetVersion);
    static std::set<proto::ContactItemAttribute> extract_attributes(
        const proto::ContactItem& serialized);
    static std::set<proto::ContactItemAttribute> extract_attributes(
        const Claim& claim);

    ContactItem set_attribute(
        const proto::ContactItemAttribute& attribute,
        const bool value) const;

    ContactItem() = delete;
    ContactItem& operator=(const ContactItem&) = delete;
    ContactItem& operator=(ContactItem&&) = delete;
};
}  // namespace opentxs

#endif
