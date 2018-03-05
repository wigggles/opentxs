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

#include "ActivitySummaryItemBlank.hpp"
#include "ActivitySummaryItem.hpp"

#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/Types.hpp"

#define STARTUP_WAIT_MILLISECONDS 100
#define VALID_ITERATORS()                                                      \
    {                                                                          \
        OT_ASSERT(items_.rend() != outer_)                                     \
                                                                               \
        const auto& item = outer_->second;                                     \
                                                                               \
        OT_ASSERT(item.end() != inner_)                                        \
    }

#define OT_METHOD "opentxs::ui::implementation::ActivitySummary::"

namespace opentxs::ui::implementation
{
ActivitySummary::ActivitySummary(
    const network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const Flag& running,
    const Identifier& nymID)
    : zmq_(zmq)
    , activity_(activity)
    , contact_manager_(contact)
    , running_(running)
    , owner_nym_id_(nymID)
    , last_thread_id_()
    , blank_p_(new ActivitySummaryItemBlank)
    , blank_(*blank_p_)
    , have_items_(Flag::Factory(false))
    , start_(Flag::Factory(true))
    , startup_complete_(Flag::Factory(false))
    , items_()
    , names_()
    , outer_(items_.rbegin())
    , inner_(items_.begin()->second.begin())
    , startup_(nullptr)
    , activity_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_thread(message);
          }))
    , activity_subscriber_(
          zmq_.SubscribeSocket(activity_subscriber_callback_.get()))
{
    OT_ASSERT(blank_p_)

    const auto endpoint = activity_.ThreadPublisher(nymID);
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = activity_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ActivitySummary::startup, this));

    OT_ASSERT(startup_)
}

void ActivitySummary::add_item(const std::string& id)
{
    const Identifier threadID(id);
    // It's hypothetically possible for a thread id to not be a contact id
    // However multi-participant threads are not yet implemented yet so this
    // will work most of the time. Even when it doesn't work it should just
    // degrade to an empty string, which is fine for the short delay until the
    // name gets set properly.
    const auto name = contact_manager_.ContactName(threadID);
    add_item(threadID, name, {});
}

void ActivitySummary::add_item(
    const Identifier& threadID,
    const std::string& name,
    const std::chrono::system_clock::time_point& time)
{
    const ItemIndex index{time, name};
    Lock lock(lock_);

    if (0 == names_.count(threadID)) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Thread " << threadID.str()
               << " is not yet instantiated." << std::endl;
        items_[index].emplace(
            threadID,
            new ActivitySummaryItem(
                *this,
                zmq_,
                activity_,
                contact_manager_,
                running_,
                owner_nym_id_,
                threadID));
        names_.emplace(threadID, index);

        OT_ASSERT(1 == items_.count(index))
        OT_ASSERT(1 == names_.count(threadID))

        return;
    }

    const auto& oldIndex = names_.at(threadID);

    if (oldIndex == index) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Thread " << threadID.str()
               << " is already instantiated and does not need to be reindexed."
               << std::endl;

        return;
    }

    reindex_item(lock, threadID, oldIndex, index);
}

/** Returns item reference by the inner_ iterator. Does not increment iterators.
 */
const opentxs::ui::ActivitySummaryItem& ActivitySummary::current(
    const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))
    VALID_ITERATORS()

    const auto & [ threadID, contact ] = *inner_;
    last_thread_id_ = threadID;

    return contact.get();
}

/** Returns first contact, or blank if none exists. Sets up iterators for next
 *  row */
const opentxs::ui::ActivitySummaryItem& ActivitySummary::first(
    const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    have_items_->Set(first_valid_item(lock));
    start_->Set(!have_items_.get());

    if (have_items_.get()) {

        return next(lock);
    } else {
        last_thread_id_ = Identifier();

        return blank_;
    }
}

const opentxs::ui::ActivitySummaryItem& ActivitySummary::First() const
{
    Lock lock(lock_);

    return first(lock);
}

/** Searches for the first name with at least one contact and sets iterators
 *  to match
 *
 *  If this function returns false, then no valid names are present and
 *  the values of outer_ and inner_ are undefined.
 */
bool ActivitySummary::first_valid_item(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    if (0 == items_.size()) {
        outer_ = items_.rbegin();
        inner_ = items_.begin()->second.begin();

        return false;
    }

    outer_ = items_.rbegin();

    while (items_.rend() != outer_) {
        const auto& item = outer_->second;

        if (0 < item.size()) {
            inner_ = item.begin();
            VALID_ITERATORS()

            return true;
        }

        ++outer_;
    }

    return false;
}

/** Increment iterators to the next valid contact, or loop back to beginning */
void ActivitySummary::increment_inner(const Lock& lock) const
{
    VALID_ITERATORS()

    const auto& item = outer_->second;

    ++inner_;

    if (item.end() != inner_) {
        VALID_ITERATORS()

        return;
    }

    // The previous position was the last item for this index.
    increment_outer(lock);
}

/** Move to the next valid intex, or loop back to beginning
 *
 *  inner_ is an invalid iterator at this point
 */
bool ActivitySummary::increment_outer(const Lock& lock) const
{
    OT_ASSERT(items_.rend() != outer_)

    bool searching{true};

    while (searching) {
        ++outer_;

        if (items_.rend() == outer_) {
            // End of the list. Both iterators are invalid at this point
            start_->On();
            have_items_->Set(first_valid_item(lock));

            if (have_items_.get()) {
                VALID_ITERATORS()
            }

            return false;
        }

        const auto& item = outer_->second;

        if (0 < item.size()) {
            searching = false;
            inner_ = item.begin();
        }
    }

    VALID_ITERATORS()

    return true;
}

bool ActivitySummary::last(const Identifier& id) const
{
    Lock lock(lock_);

    return (start_.get() && (id == last_thread_id_));
}

void ActivitySummary::process_thread(const network::zeromq::Message& message)
{
    while (false == startup_complete_.get()) {
        Log::Sleep(std::chrono::milliseconds(STARTUP_WAIT_MILLISECONDS));
    }

    const std::string id(message);
    const Identifier threadID(id);

    OT_ASSERT(false == threadID.empty())

    auto existing = names_.count(threadID);

    if (0 == existing) {
        add_item(id);
    }
}

void ActivitySummary::reindex_item(
    const Identifier& threadID,
    const ItemIndex& newIndex) const
{
    Lock lock(lock_);
    const auto& oldIndex = names_.at(threadID);
    reindex_item(lock, threadID, oldIndex, newIndex);
}

void ActivitySummary::reindex_item(
    const Lock& lock,
    const Identifier& threadID,
    const ItemIndex& oldIndex,
    const ItemIndex& newIndex) const
{
    OT_ASSERT(verify_lock(lock));
    OT_ASSERT(1 == items_.count(oldIndex))

    auto index = items_.find(oldIndex);

    OT_ASSERT(items_.end() != index);

    auto& itemMap = index->second;
    auto item = itemMap.find(threadID);

    OT_ASSERT(itemMap.end() != item);

    // I'm about to delete this row. Make sure iterators are not pointing to it
    if (inner_ == item) {
        increment_inner(lock);
    }

    OTUIActivitySummaryItem row = std::move(item->second);
    const auto deleted = itemMap.erase(threadID);

    OT_ASSERT(1 == deleted)

    if (0 == itemMap.size()) {
        items_.erase(index);
    }

    names_[threadID] = newIndex;
    items_[newIndex].emplace(std::move(threadID), std::move(row));
}

/** Returns the next item and increments iterators */
const opentxs::ui::ActivitySummaryItem& ActivitySummary::next(
    const Lock& lock) const
{
    const auto& output = current(lock);
    increment_inner(lock);

    return output;
}

const opentxs::ui::ActivitySummaryItem& ActivitySummary::Next() const
{
    Lock lock(lock_);

    if (start_.get()) {

        return first(lock);
    }

    return next(lock);
}

void ActivitySummary::startup()
{
    const auto threads = activity_.Threads(owner_nym_id_, false);
    otWarn << OT_METHOD << __FUNCTION__ << ": Starting " << threads.size()
           << " threads." << std::endl;

    for (const auto & [ id, alias ] : threads) {
        [[maybe_unused]] const auto& notUsed = alias;
        add_item(id);
    }

    startup_complete_->On();
}

ActivitySummary::~ActivitySummary()
{
    if (startup_ && startup_->joinable()) {
        startup_->join();
        startup_.reset();
    }
}
}  // namespace opentxs::ui::implementation
