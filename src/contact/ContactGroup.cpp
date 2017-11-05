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

#include "opentxs/stdafx.hpp"

#include "opentxs/contact/ContactGroup.hpp"

#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Log.hpp"

#define OT_METHOD "opentxs::ContactGroup::"

namespace opentxs
{
ContactGroup::ContactGroup(
    const std::string& nym,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const Identifier& primary,
    const ItemMap& items)
    : nym_(nym)
    , section_(section)
    , type_(type)
    , primary_(primary)
    , items_(items)
{
    for (const auto& it : items_) {
        OT_ASSERT(it.second);
    }
}

ContactGroup::ContactGroup(
    const std::string& nym,
    const proto::ContactSectionName section,
    const Identifier& primary,
    const std::shared_ptr<ContactItem>& item)
    : ContactGroup(nym, section, item->Type(), primary, create_item(item))
{
    OT_ASSERT(item);
}

ContactGroup ContactGroup::operator+(const ContactGroup& rhs) const
{
    OT_ASSERT(section_ == rhs.section_);

    Identifier primary{};

    if (primary_.empty()) {
        primary = rhs.primary_;
    } else {
        primary = primary_;
    }

    auto map = items_;

    for (const auto& it : rhs.items_) {
        const auto& item = it.second;

        OT_ASSERT(item);
        const auto& id = item->ID();
        const bool exists = (1 == map.count(id));

        if (exists) {
            continue;
        }

        const bool isPrimary = item->isPrimary();
        const bool designated = (id == primary);

        if (isPrimary && (false == designated)) {
            map[id].reset(new ContactItem(item->SetPrimary(false)));
        } else {
            map[id] = item;
        }
    }

    return ContactGroup(nym_, section_, type_, primary, map);
}

ContactGroup ContactGroup::AddItem(
    const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    if (item->isPrimary()) {

        return AddPrimary(item);
    }

    const auto& id = item->ID();
    const bool alreadyExists =
        (1 == items_.count(id)) && (*item == *items_.at(id));

    if (alreadyExists) {

        return *this;
    }

    auto map = items_;
    map[id] = item;

    if (primary_ == id) {

        return ContactGroup(nym_, section_, type_, {}, map);
    }

    return ContactGroup(nym_, section_, type_, id, map);
}

ContactGroup ContactGroup::AddPrimary(
    const std::shared_ptr<ContactItem>& item) const
{
    if (false == bool(item)) {

        return *this;
    }

    const auto& incomingID = item->ID();
    const bool isExistingPrimary = (primary_ == incomingID);
    const bool haveExistingPrimary =
        ((false == primary_.empty()) && (false == isExistingPrimary));
    auto map = items_;
    auto& newPrimary = map[incomingID];
    newPrimary.reset(new ContactItem(item->SetPrimary(true)));

    OT_ASSERT(newPrimary);

    if (haveExistingPrimary) {
        auto& oldPrimary = map.at(primary_);
        oldPrimary.reset(new ContactItem(item->SetPrimary(false)));

        OT_ASSERT(oldPrimary);
    }

    return ContactGroup(nym_, section_, type_, incomingID, map);
}

ContactGroup::ItemMap::const_iterator ContactGroup::begin() const
{
    return items_.cbegin();
}

std::shared_ptr<ContactItem> ContactGroup::Best() const
{
    if (0 == items_.size()) {

        return {};
    }

    if (false == primary_.empty()) {

        return items_.at(primary_);
    }

    for (const auto& it : items_) {
        const auto& claim = it.second;

        if (claim->isActive()) {

            return claim;
        }
    }

    return items_.begin()->second;
}

std::shared_ptr<ContactItem> ContactGroup::Claim(Identifier& item) const
{
    auto it = items_.find(item);

    if (items_.end() == it) {

        return {};
    }

    return it->second;
}

ContactGroup::ItemMap ContactGroup::create_item(
    const std::shared_ptr<ContactItem>& item)
{
    OT_ASSERT(item);

    ItemMap output{};
    output[item->ID()] = item;

    return output;
}

ContactGroup ContactGroup::Delete(const Identifier& id)
{
    const bool exists = (1 == items_.count(id));
    const bool primary = (primary_ == id);

    if (false == exists) {

        return *this;
    }

    auto map = items_;
    map.erase(id);

    if (primary) {
        return ContactGroup(nym_, section_, type_, primary_, map);
    } else {
        return ContactGroup(nym_, section_, type_, {}, map);
    }
}

ContactGroup::ItemMap::const_iterator ContactGroup::end() const
{
    return items_.cend();
}

bool ContactGroup::HaveClaim(const Identifier& item) const
{
    return (1 == items_.count(item));
}

const Identifier& ContactGroup::Primary() const { return primary_; }

bool ContactGroup::SerializeTo(
    proto::ContactSection& section,
    const bool withIDs) const
{
    if (section.name() != section_) {

        otErr << OT_METHOD << __FUNCTION__
              << ": Trying to serialized to incorrect section." << std::endl;

        return false;
    }

    for (const auto& it : items_) {
        const auto& item = it.second;

        OT_ASSERT(item);

        *section.add_item() = item->Serialize(withIDs);
    }

    return true;
}

std::shared_ptr<ContactItem> ContactGroup::PrimaryClaim() const
{
    if (primary_.empty()) {

        return {};
    }

    return items_.at(primary_);
}

std::size_t ContactGroup::Size() const { return items_.size(); }

const proto::ContactItemType& ContactGroup::Type() const { return type_; }
}  // namespace opentxs
