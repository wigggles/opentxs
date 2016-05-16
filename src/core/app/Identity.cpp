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

#include "opentxs/core/app/Identity.hpp"

#include "opentxs/core/Nym.hpp"

namespace opentxs
{
bool Identity::AddClaim(
    Nym& toNym,
    const Claim claim) const
{
    std::unique_ptr<proto::ContactData> revised;
    auto existing = toNym.ContactData();

    if (existing) {
        revised.reset(new proto::ContactData(*existing));
    } else {
        revised = InitializeContactData();
    }

    AddClaimToSection(*revised, claim);

    return toNym.SetContactData(*revised);
}

void Identity::AddClaimToSection(
    proto::ContactData& data,
    const Claim& claim) const
{
    const auto sectionType =
        static_cast<proto::ContactSectionName>(std::get<1>(claim));
    const auto itemType =
        static_cast<proto::ContactItemType>(std::get<2>(claim));
    const auto value = std::get<3>(claim);
    const auto start = std::get<4>(claim);
    const auto end = std::get<5>(claim);

    auto section = GetOrCreateSection(
        data,
        sectionType);
    const bool primary = ClaimIsPrimary(claim);

    if (primary) {
        ResetPrimary(section, itemType);
    }

    auto& item = GetOrCreateClaim(
        section,
        itemType,
        value,
        start,
        end);

    SetAttributesOnClaim(item, claim);
}

bool Identity::ClaimIsPrimary(const Claim& claim) const
{
    bool primary = false;

    for (auto& attribute: std::get<6>(claim)) {
        if (proto::CITEMATTR_PRIMARY == attribute) {
            primary = true;
        }
    }

    return primary;
}

// Because we're building with protobuf-lite, we don't have library
// support for deleting items from protobuf repeated fields.
// Thus we delete by making a copy which excludes the item to be
// deleted.
void Identity::ClearPrimaryAttribute(proto::ContactItem& claim) const
{
    proto::ContactItem revised;
    revised.set_version(claim.version());
    revised.set_type(claim.type());
    revised.set_start(claim.start());
    revised.set_end(claim.end());
    bool changed = false;

    for (auto& attribute : claim.attribute()) {
        if (proto::CITEMATTR_PRIMARY == attribute) {
            changed = true;
        } else {
            revised.add_attribute(
                static_cast<proto::ContactItemAttribute>(attribute));
        }
    }

    if (changed) {
        claim = revised;
    }
}

proto::ContactItem& Identity::GetOrCreateClaim(
    proto::ContactSection& section,
    const proto::ContactItemType type,
    const std::string& value,
    const std::int64_t start,
    const std::int64_t end) const
{
    for (auto& claim : *section.mutable_item()) {
        if ((type == claim.type()) &&
            (value == claim.value()) &&
            (start <= claim.start()) &&
            (end >= claim.end())) {
                claim.set_start(start);
                claim.set_end(end);

                return claim;
        }
    }

    auto& newClaim = *section.add_item();
    InitializeContactItem(newClaim, type, value, start, end);

    return newClaim;
}

proto::ContactSection& Identity::GetOrCreateSection(
    proto::ContactData& data,
    proto::ContactSectionName section,
    const std::uint32_t version) const
{
    for (auto& it : *data.mutable_section()) {
        if (it.name() == section) {

            return it;
        }
    }

    auto& newSection = *data.add_section();
    InitializeContactSection(newSection, section, version);

    return newSection;
}

std::unique_ptr<proto::ContactData> Identity::InitializeContactData(
    const std::uint32_t version) const
{
    std::unique_ptr<proto::ContactData> output(new proto::ContactData);
    output->set_version(version);

    return output;
}

void Identity::InitializeContactItem(
    proto::ContactItem& item,
    const proto::ContactItemType type,
    const std::string& value,
    const std::int64_t start,
    const std::int64_t end) const
{
    item.set_type(type);
    item.set_value(value);
    item.set_start(start);
    item.set_end(end);
}

void Identity::InitializeContactSection(
    proto::ContactSection& section,
    const proto::ContactSectionName name,
    const std::uint32_t version) const
{
    section.set_version(version);
    section.set_name(name);
}

void Identity::ResetPrimary(
    proto::ContactSection& section,
    const proto::ContactItemType& type) const
{
    for (auto& item: *section.mutable_item()) {
        if (type == item.type()) {
            ClearPrimaryAttribute(item);
        }
    }
}

void Identity::SetAttributesOnClaim(
    proto::ContactItem& item,
    const Claim& claim) const
{
    item.clear_attribute();

    for (auto& attribute: std::get<6>(claim)) {
        item.add_attribute(
            static_cast<proto::ContactItemAttribute>(attribute));
    }
}
} // namespace opentxs
