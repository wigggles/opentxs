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

#include "ActivitySummary.hpp"

#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "ActivitySummaryItemBlank.hpp"
#include "ActivitySummaryItem.hpp"

template class opentxs::Pimpl<opentxs::ui::ActivitySummary>;

#define OT_METHOD "opentxs::ui::implementation::ActivitySummary::"

namespace opentxs::ui::implementation
{
ActivitySummary::ActivitySummary(
    const network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const Flag& running,
    const Identifier& nymID)
    : ActivitySummaryType(
          zmq,
          contact,
          blank_id(),
          nymID,
          new ActivitySummaryItemBlank)
    , activity_(activity)
    , running_(running)
    , activity_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_thread(message);
          }))
    , activity_subscriber_(
          zmq_.SubscribeSocket(activity_subscriber_callback_.get()))
{
    OT_ASSERT(blank_p_)

    init();
    const auto endpoint = activity_.ThreadPublisher(nymID);
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = activity_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ActivitySummary::startup, this));

    OT_ASSERT(startup_)
}

ActivitySummaryID ActivitySummary::blank_id() const
{
    return Identifier::Factory();
}

void ActivitySummary::construct_item(
    const ActivitySummaryID& id,
    const ActivitySummarySortKey& index,
    void*) const
{
    items_[index].emplace(
        id,
        new ActivitySummaryItem(
            *this, zmq_, activity_, contact_manager_, running_, nym_id_, id));
    names_.emplace(id, index);
}

ActivitySummaryOuter::const_reverse_iterator ActivitySummary::outer_first()
    const
{
    return items_.rbegin();
}

ActivitySummaryOuter::const_reverse_iterator ActivitySummary::outer_end() const
{
    return items_.rend();
}

void ActivitySummary::process_thread(const std::string& id)
{
    const Identifier threadID(id);
    // It's hypothetically possible for a thread id to not be a contact id
    // However multi-participant threads are not yet implemented yet so this
    // will work most of the time. Even when it doesn't work it should just
    // degrade to an empty string, which is fine for the short delay until the
    // name gets set properly.
    const auto name = contact_manager_.ContactName(threadID);
    const ActivitySummarySortKey index{{}, name};
    add_item(threadID, index);
}

void ActivitySummary::process_thread(const network::zeromq::Message& message)
{
    wait_for_startup();
    const std::string id(message);
    const Identifier threadID(id);

    OT_ASSERT(false == threadID.empty())

    auto existing = names_.count(threadID);

    if (0 == existing) {
        process_thread(id);
    }
}

void ActivitySummary::startup()
{
    const auto threads = activity_.Threads(nym_id_, false);
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << threads.size()
           << " threads." << std::endl;

    for (const auto & [ id, alias ] : threads) {
        [[maybe_unused]] const auto& notUsed = alias;
        process_thread(id);
    }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
