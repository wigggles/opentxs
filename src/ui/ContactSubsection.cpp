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

#include "stdafx.hpp"

#include "opentxs/api/ContactManager.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/ContactSubsection.hpp"

#include "ContactSectionParent.hpp"
#include "ContactSubsectionParent.hpp"
#include "ContactItemBlank.hpp"
#include "List.hpp"
#include "RowType.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ContactSubsection.hpp"

//#define OT_METHOD "opentxs::ui::implementation::ContactSubsection::"

namespace opentxs
{
ui::ContactSubsection* Factory::ContactSubsectionWidget(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const ui::implementation::ContactSectionParent& parent,
    const ContactGroup& group)
{
    return new ui::implementation::ContactSubsection(
        zmq, contact, parent, group);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactSubsection::ContactSubsection(
    const network::zeromq::Context& zmq,
    const api::ContactManager& contact,
    const ContactSectionParent& parent,
    const opentxs::ContactGroup& group)
    : ContactSubsectionType(
          zmq,
          contact,
          Identifier::Factory(),
          Identifier::Factory(parent.ContactID()),
          new ContactItemBlank)
    , ContactSubsectionRowType(parent, {parent.Type(), group.Type()}, true)
{
    init();
    startup_.reset(new std::thread(&ContactSubsection::startup, this, group));

    OT_ASSERT(startup_)
}

void ContactSubsection::construct_item(
    const ContactSubsectionIDType& id,
    const ContactSubsectionSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactItemWidget(
            zmq_, contact_manager_, *this, recover(custom[0])));
}

std::string ContactSubsection::Name(const std::string& lang) const
{
    return proto::TranslateItemType(id_.second, lang);
}

void ContactSubsection::process_group(const opentxs::ContactGroup& group)
{
    OT_ASSERT(id_.second == group.Type())

    Lock lock(lock_);
    names_.clear();
    items_.clear();
    init();
    lock.unlock();

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        add_item(id, sort_key(id), {claim.get()});
    }

    UpdateNotify();
}

const opentxs::ContactItem& ContactSubsection::recover(const void* input)
{
    OT_ASSERT(nullptr != input)

    return *static_cast<const opentxs::ContactItem*>(input);
}

int ContactSubsection::sort_key(const ContactSubsectionIDType) const
{
    return static_cast<int>(items_.size());
}

void ContactSubsection::startup(const opentxs::ContactGroup group)
{
    process_group(group);
    startup_complete_->On();
}

void ContactSubsection::Update(const opentxs::ContactGroup& group)
{
    process_group(group);
}
}  // namespace opentxs::ui::implementation
