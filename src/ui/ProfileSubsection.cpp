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

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/ProfileSubsection.hpp"

#include "List.hpp"
#include "ProfileSectionParent.hpp"
#include "ProfileSubsectionParent.hpp"
#include "ProfileItemBlank.hpp"
#include "RowType.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ProfileSubsection.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ProfileSubsection::"

namespace opentxs
{
ui::ProfileSubsection* Factory::ProfileSubsectionWidget(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const ui::implementation::ProfileSectionParent& parent,
    const ContactGroup& group)
{
    return new ui::implementation::ProfileSubsection(
        zmq, contact, wallet, parent, group);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ProfileSubsection::ProfileSubsection(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const api::client::Wallet& wallet,
    const ProfileSectionParent& parent,
    const opentxs::ContactGroup& group)
    : ProfileSubsectionType(
          zmq,
          contact,
          Identifier::Factory(),
          Identifier::Factory(parent.NymID()),
          new ProfileItemBlank)
    , ProfileSubsectionRowType(parent, {parent.Type(), group.Type()}, true)
    , wallet_(wallet)
{
    OT_ASSERT(blank_p_)

    init();
    startup_.reset(new std::thread(&ProfileSubsection::startup, this, group));

    OT_ASSERT(startup_)
}

bool ProfileSubsection::AddItem(
    const std::string& value,
    const bool primary,
    const bool active) const
{
    return parent_.AddClaim(id_.second, value, primary, active);
}

void ProfileSubsection::construct_item(
    const ProfileSubsectionIDType& id,
    const ProfileSubsectionSortKey& index,
    void* custom) const
{
    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ProfileItemWidget(
            zmq_, contact_manager_, wallet_, *this, recover(custom)));
}

bool ProfileSubsection::Delete(const std::string& claimID) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) {

        return false;
    }

    return claim.Delete();
}

std::string ProfileSubsection::Name(const std::string& lang) const
{
    return proto::TranslateItemType(id_.second, lang);
}

void ProfileSubsection::process_group(const opentxs::ContactGroup& group)
{
    OT_ASSERT(id_.second == group.Type())

    Lock lock(lock_);
    names_.clear();
    items_.clear();
    init();
    lock.unlock();

    for (const auto & [ id, claim ] : group) {
        OT_ASSERT(claim)

        add_item(id, sort_key(id), claim.get());
    }

    UpdateNotify();
}

const opentxs::ContactItem& ProfileSubsection::recover(const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const opentxs::ContactItem*>(input);
}

bool ProfileSubsection::SetActive(const std::string& claimID, const bool active)
    const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) {

        return false;
    }

    return claim.SetActive(active);
}

bool ProfileSubsection::SetPrimary(
    const std::string& claimID,
    const bool primary) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) {

        return false;
    }

    return claim.SetPrimary(primary);
}

bool ProfileSubsection::SetValue(
    const std::string& claimID,
    const std::string& value) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) {

        return false;
    }

    return claim.SetValue(value);
}

int ProfileSubsection::sort_key(const ProfileSubsectionIDType) const
{
    return static_cast<int>(items_.size());
}

void ProfileSubsection::startup(const opentxs::ContactGroup group)
{
    process_group(group);
    startup_complete_->On();
}

void ProfileSubsection::Update(const opentxs::ContactGroup& group)
{
    process_group(group);
}
}  // namespace opentxs::ui::implementation
