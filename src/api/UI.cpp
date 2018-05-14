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

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/UI.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>
#include <tuple>

#include "UI.hpp"

#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs
{
api::UI* Factory::UI(
    const network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const Flag& running)
{
    return new api::implementation::UI(
        zmq, activity, contact, sync, wallet, running);
}
}  // namespace opentxs

namespace opentxs::api::implementation
{
UI::UI(
    const opentxs::network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const api::client::Wallet& wallet,
    const Flag& running)
    : zmq_(zmq)
    , activity_(activity)
    , contact_(contact)
    , sync_(sync)
    , wallet_(wallet)
    , running_(running)
    , activity_summaries_()
    , contacts_()
    , contact_lists_()
    , messagable_lists_()
    , widget_callback_(opentxs::network::zeromq::ReplyCallback::Factory(
          [this](
              const opentxs::network::zeromq::Message& input) -> OTZMQMessage {
              std::string message(input);
              otinfo << OT_METHOD << ": Relaying notification for widget "
                     << message << "..." << std::endl;
              widget_update_publisher_->Publish(message);
              otInfo << "...done" << std::endl;

              return opentxs::network::zeromq::Message::Factory();
          }))
    , widget_update_collector_(zmq_.ReplySocket(widget_callback_))
    , widget_update_publisher_(zmq_.PublishSocket())
{
    widget_update_publisher_->Start(
        opentxs::network::zeromq::Socket::WidgetUpdateEndpoint);
    widget_update_collector_->Start(
        opentxs::network::zeromq::Socket::WidgetUpdateCollectorEndpoint);
}

const ui::ActivitySummary& UI::ActivitySummary(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = activity_summaries_[nymID];

    if (false == bool(output)) {
        output.reset(Factory::ActivitySummary(
            zmq_, activity_, contact_, running_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::ActivityThread& UI::ActivityThread(
    const Identifier& nymID,
    const Identifier& threadID) const
{
    Lock lock(lock_);
    auto& output = activity_threads_[{nymID, threadID}];

    if (false == bool(output)) {
        output.reset(Factory::ActivityThread(
            zmq_, sync_, activity_, contact_, nymID, threadID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::Contact& UI::Contact(const Identifier& contactID) const
{
    Lock lock(lock_);
    auto id = Identifier::Factory(contactID);
    auto it = contacts_.find(id);

    if (contacts_.end() == it) {
        it = contacts_
                 .emplace(
                     std::move(id),
                     Factory::ContactWidget(zmq_, contact_, contactID))
                 .first;
    }

    OT_ASSERT(it->second)

    return *(it->second);
}

const ui::ContactList& UI::ContactList(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = contact_lists_[nymID];

    if (false == bool(output)) {
        output.reset(Factory::ContactList(zmq_, contact_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::MessagableList& UI::MessagableList(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = messagable_lists_[nymID];

    if (false == bool(output)) {
        output.reset(Factory::MessagableList(zmq_, contact_, sync_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::PayableList& UI::PayableList(
    const Identifier& nymID,
    proto::ContactItemType currency) const
{
    Lock lock(lock_);
    auto& output =
        payable_lists_[std::pair<OTIdentifier, proto::ContactItemType>(
            nymID, currency)];

    if (false == bool(output)) {
        output.reset(
            Factory::PayableList(zmq_, contact_, sync_, nymID, currency));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::Profile& UI::Profile(const Identifier& contactID) const
{
    Lock lock(lock_);
    auto id = Identifier::Factory(contactID);
    auto it = profiles_.find(id);

    if (profiles_.end() == it) {
        it = profiles_
                 .emplace(
                     std::move(id),
                     Factory::ProfileWidget(zmq_, contact_, wallet_, contactID))
                 .first;
    }

    OT_ASSERT(it->second)

    return *(it->second);
}

UI::~UI() {}
}  // namespace opentxs::api::implementation
