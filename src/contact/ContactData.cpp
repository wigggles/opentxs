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

#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Log.hpp"

#include <sstream>

namespace opentxs
{
ContactData::ContactData(
    const std::string& nym,
    const SectionMap& sections,
    const std::uint32_t version)
    : version_(check_version(version))
    , nym_(nym)
    , sections_(sections)
{
}

ContactData::ContactData(
    const std::string& nym,
    const proto::ContactData& serialized)
    : ContactData(nym, extract_sections(nym, serialized), serialized.version())
{
}

ContactData ContactData::operator+(const ContactData& rhs) const
{
    auto map = sections_;

    for (auto& it : rhs.sections_) {
        auto& rhsID = it.first;
        auto& rhsSection = it.second;

        OT_ASSERT(rhsSection);

        auto lhs = map.find(rhsID);
        const bool exists = (map.end() != lhs);

        if (exists) {
            auto& section = lhs->second;

            OT_ASSERT(section);

            section.reset(new ContactSection(*section + *rhsSection));
        } else {
            map[rhsID] = rhsSection;
        }
    }

    return ContactData(nym_, map, version_);
}

ContactData::operator std::string() const
{
    return PrintContactData(Serialize(false));
}

ContactData ContactData::AddItem(const ClaimTuple& claim) const
{
    auto item = std::make_shared<ContactItem>(nym_, claim);

    return AddItem(item);
}

ContactData ContactData::AddItem(const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    const auto& sectionID = item->Section();
    auto map = sections_;
    auto it = map.find(sectionID);

    if (map.end() == it) {
        auto& section = map[sectionID];
        section.reset(new ContactSection(nym_, sectionID, item, version_));

        OT_ASSERT(section);
    } else {
        auto& section = it->second;

        OT_ASSERT(section);

        section.reset(new ContactSection(section->AddItem(item)));

        OT_ASSERT(section);
    }

    return ContactData(nym_, map, version_);
}

ContactData ContactData::AddPreferredOTServer(
    const Identifier& id,
    const bool primary) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{
        proto::CONTACTSECTION_COMMUNICATION};
    const proto::ContactItemType type{proto::CITEMTYPE_OPENTXS};
    auto group = Group(section, type);

    if (group) {
        needPrimary = group->Primary().empty();
    }

    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE};

    if (primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_PRIMARY);
    }

    auto item = std::make_shared<ContactItem>(
        nym_, section, type, String(id).Get(), attrib);

    OT_ASSERT(item);

    return AddItem(item);
}

std::uint32_t ContactData::check_version(const std::uint32_t in)
{
    // Upgrade version
    if (CONTACT_DATA_VERSION > in) {

        return CONTACT_DATA_VERSION;
    }

    return in;
}

std::shared_ptr<ContactItem> ContactData::Claim(Identifier& item) const
{
    for (const auto& it : sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        auto claim = section->Claim(item);

        if (claim) {

            return claim;
        }
    }

    return {};
}

ContactData ContactData::Delete(const Identifier& id) const
{
    bool deleted{false};
    auto map = sections_;

    for (auto& it : map) {
        auto& section = it.second;

        OT_ASSERT(section);

        if (section->HaveClaim(id)) {
            section.reset(new ContactSection(section->Delete(id)));

            OT_ASSERT(section);

            deleted = true;

            if (0 == section->Size()) {
                map.erase(it.first);
            }

            break;
        }
    }

    if (false == deleted) {

        return *this;
    }

    return ContactData(nym_, map, version_);
}

ContactData::SectionMap ContactData::extract_sections(
    const std::string& nym,
    const proto::ContactData& serialized)
{
    SectionMap sectionMap{};

    for (const auto it : serialized.section()) {
        sectionMap[it.name()].reset(new ContactSection(nym, it));
    }

    return sectionMap;
}

std::shared_ptr<ContactGroup> ContactData::Group(
    const proto::ContactSectionName& section,
    const proto::ContactItemType& type) const
{
    const auto it = sections_.find(section);

    if (sections_.end() == it) {

        return {};
    }

    OT_ASSERT(it->second);

    return it->second->Group(type);
}

bool ContactData::HaveClaim(const Identifier& item) const
{
    for (const auto& section : sections_) {
        OT_ASSERT(section.second);

        if (section.second->HaveClaim(item)) {

            return true;
        }
    }

    return false;
}

bool ContactData::HaveClaim(
    const proto::ContactSectionName& section,
    const proto::ContactItemType& type,
    const std::string& value) const
{
    auto group = Group(section, type);

    if (false == bool(group)) {

        return false;
    }

    for (const auto it : *group) {
        OT_ASSERT(it.second);

        const auto& claim = *it.second;

        if (value == claim.Value()) {

            return true;
        }
    }

    return false;
}

std::string ContactData::Name() const
{
    auto group = scope().second;

    if (false == bool(group)) {

        return {};
    }

    auto claim = group->Best();

    if (false == bool(claim)) {

        return {};
    }

    return claim->Value();
}

Identifier ContactData::PreferredOTServer() const
{
    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_OPENTXS);

    if (false == bool(group)) {

        return {};
    }

    auto claim = group->Best();

    if (false == bool(claim)) {

        return {};
    }

    return Identifier(claim->Value());
}

std::string ContactData::PrintContactData(const proto::ContactData& data)
{
    std::stringstream output;
    output << "Version " << data.version() << " contact data" << std::endl;
    output << "Sections found: " << data.section().size() << std::endl;

    for (const auto& section : data.section()) {
        output << "- Section: " << proto::TranslateSectionName(section.name())
               << ", version: " << section.version() << " containing "
               << section.item().size() << " item(s)." << std::endl;

        for (const auto& item : section.item()) {
            output << "-- Item type: \""
                   << proto::TranslateItemType(item.type()) << "\", value: \""
                   << item.value() << "\", start: " << item.start()
                   << ", end: " << item.end() << ", version: " << item.version()
                   << std::endl
                   << "--- Attributes: ";

            for (const auto& attribute : item.attribute()) {
                output << proto::TranslateItemAttributes(
                              static_cast<proto::ContactItemAttribute>(
                                  attribute))
                       << " ";
            }

            output << std::endl;
        }
    }

    return output.str();
}

std::shared_ptr<ContactSection> ContactData::Section(
    const proto::ContactSectionName& section) const
{
    const auto it = sections_.find(section);

    if (sections_.end() == it) {

        return {};
    }

    return it->second;
}

ContactData ContactData::SetCommonName(const std::string& name) const
{
    const proto::ContactSectionName section{proto::CONTACTSECTION_IDENTIFIER};
    const proto::ContactItemType type{proto::CITEMTYPE_COMMONNAME};
    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE,
                                                 proto::CITEMATTR_PRIMARY};

    auto item =
        std::make_shared<ContactItem>(nym_, section, type, name, attrib);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::SetScope(
    const proto::ContactItemType type,
    const std::string& name,
    const bool primary) const
{
    const proto::ContactSectionName section{proto::CONTACTSECTION_SCOPE};

    if (type != scope().first) {
        auto mapCopy = sections_;
        mapCopy.erase(section);
        std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE,
                                                     proto::CITEMATTR_PRIMARY};
        auto item =
            std::make_shared<ContactItem>(nym_, section, type, name, attrib);

        OT_ASSERT(item);

        auto newSection =
            std::make_shared<ContactSection>(nym_, section, item, version_);

        OT_ASSERT(newSection);

        mapCopy[section] = newSection;

        return ContactData(nym_, mapCopy, version_);
    }

    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE};

    if (primary) {
        attrib.emplace(proto::CITEMATTR_PRIMARY);
    }

    auto item =
        std::make_shared<ContactItem>(nym_, section, type, name, attrib);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData::Scope ContactData::scope() const
{
    const auto it = sections_.find(proto::CONTACTSECTION_SCOPE);

    if (sections_.end() == it) {

        return {proto::CITEMTYPE_ERROR, nullptr};
    }

    OT_ASSERT(it->second);

    const auto& section = *it->second;

    if (1 != section.Size()) {

        return {proto::CITEMTYPE_ERROR, nullptr};
    }

    return *section.begin();
}

proto::ContactData ContactData::Serialize(const bool withID) const
{
    proto::ContactData output;
    output.set_version(version_);

    for (const auto& it : sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        section->SerializeTo(output, withID);
    }

    return output;
}

proto::ContactItemType ContactData::Type() const { return scope().first; }
}  // namespace opentxs
