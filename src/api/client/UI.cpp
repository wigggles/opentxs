// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountSummary.hpp"
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

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs
{
api::client::UI* Factory::UI(
    const api::client::Sync& sync,
    const api::Wallet& wallet,
    const api::client::Workflow& workflow,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::client::Activity& activity,
    const api::client::Contacts& contact,
    const api::Core& core,
    const network::zeromq::Context& zmq,
    const Flag& running)
{
    return new api::client::implementation::UI(
        sync,
        wallet,
        workflow,
        connection,
        storage,
        activity,
        contact,
        core,
        zmq,
        running);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
UI::UI(
    const api::client::Sync& sync,
    const api::Wallet& wallet,
    const api::client::Workflow& workflow,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::client::Activity& activity,
    const api::client::Contacts& contact,
    const api::Core& core,
    const opentxs::network::zeromq::Context& zmq,
    const Flag& running)
    : sync_(sync)
    , wallet_(wallet)
    , workflow_(workflow)
    , connection_(connection)
    , storage_(storage)
    , activity_(activity)
    , contact_(contact)
    , core_(core)
    , zmq_(zmq)
    , running_(running)
    , accounts_()
    , accounts_summaries_()
    , activity_summaries_()
    , contacts_()
    , contact_lists_()
    , messagable_lists_()
    , widget_update_publisher_(zmq_.PublishSocket())
{
    widget_update_publisher_->Start(
        opentxs::network::zeromq::Socket::WidgetUpdateEndpoint);
}

const ui::AccountActivity& UI::AccountActivity(
    const Identifier& nymID,
    const Identifier& accountID) const
{
    Lock lock(lock_);
    const AccountKey key(
        Identifier::Factory(nymID), Identifier::Factory(accountID));
    auto& output = accounts_[key];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::AccountActivity(
            zmq_,
            widget_update_publisher_,
            sync_,
            workflow_,
            contact_,
            storage_,
            core_,
            nymID,
            accountID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::AccountSummary& UI::AccountSummary(
    const Identifier& nymID,
    const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const AccountSummaryKey key{Identifier::Factory(nymID), currency};
    auto& output = accounts_summaries_[key];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::AccountSummary(
            zmq_,
            widget_update_publisher_,
            connection_,
            storage_,
            contact_,
            core_,
            nymID,
            currency));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::ActivitySummary& UI::ActivitySummary(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = activity_summaries_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::ActivitySummary(
            zmq_,
            widget_update_publisher_,
            activity_,
            contact_,
            running_,
            nymID));
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
        output.reset(opentxs::Factory::ActivityThread(
            zmq_,
            widget_update_publisher_,
            sync_,
            activity_,
            contact_,
            nymID,
            threadID));
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
                     opentxs::Factory::ContactWidget(
                         zmq_, widget_update_publisher_, contact_, contactID))
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
        output.reset(opentxs::Factory::ContactList(
            zmq_, widget_update_publisher_, contact_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::MessagableList& UI::MessagableList(const Identifier& nymID) const
{
    Lock lock(lock_);
    auto& output = messagable_lists_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::MessagableList(
            zmq_, widget_update_publisher_, contact_, sync_, nymID));
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
        output.reset(opentxs::Factory::PayableList(
            zmq_, widget_update_publisher_, contact_, sync_, nymID, currency));
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
                     opentxs::Factory::ProfileWidget(
                         zmq_,
                         widget_update_publisher_,
                         contact_,
                         wallet_,
                         contactID))
                 .first;
    }

    OT_ASSERT(it->second)

    return *(it->second);
}

UI::~UI() {}
}  // namespace opentxs::api::client::implementation
