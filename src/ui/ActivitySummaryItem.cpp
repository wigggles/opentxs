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

#include "ActivitySummaryItem.hpp"

#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "ActivitySummary.hpp"

#include <set>
#include <sstream>

#define GET_TEXT_MILLISECONDS 10

#define OT_METHOD "opentxs::ui::implementation::ActivitySummaryItem::"

namespace opentxs::ui::implementation
{
ActivitySummaryItem::ActivitySummaryItem(
    const ActivitySummary& parent,
    const network::zeromq::Context& zmq,
    const api::Activity& activity,
    const api::ContactManager& contact,
    const Flag& running,
    const Identifier& nymID,
    const Identifier& threadID)
    : ActivitySummaryItemType(parent, zmq, contact, threadID, true)
    , activity_(activity)
    , running_(running)
    , nym_id_(nymID)
    , thread_()
    , display_name_("")
    , text_("")
    , type_(StorageBox::UNKNOWN)
    , time_()
    , startup_(nullptr)
    , newest_item_thread_(nullptr)
    , newest_item_()
    , activity_subscriber_callback_(network::zeromq::ListenCallback::Factory(
          [this](const network::zeromq::Message& message) -> void {
              this->process_thread(message);
          }))
    , activity_subscriber_(
          zmq_.SubscribeSocket(activity_subscriber_callback_.get()))
{
    const auto endpoint = activity_.ThreadPublisher(nymID);
    otWarn << OT_METHOD << __FUNCTION__ << ": Connecting to " << endpoint
           << std::endl;
    const auto listening = activity_subscriber_->Start(endpoint);

    OT_ASSERT(listening)

    startup_.reset(new std::thread(&ActivitySummaryItem::startup, this));

    OT_ASSERT(startup_)

    newest_item_thread_.reset(
        new std::thread(&ActivitySummaryItem::get_text, this));

    OT_ASSERT(newest_item_thread_)
}

bool ActivitySummaryItem::check_thread(const proto::StorageThread& thread) const
{
    if (1 > thread.item_size()) {

        return false;
    }

    if (1 != thread.participant_size()) {

        OT_FAIL
    }

    if (thread.id() != thread.participant(0)) {

        OT_FAIL
    }

    return true;
}

std::string ActivitySummaryItem::display_name(
    const proto::StorageThread& thread) const
{
    std::set<std::string> names{};

    for (const auto& participant : thread.participant()) {
        auto name = contact_.ContactName(Identifier(participant));

        if (name.empty()) {
            names.emplace(participant);
        } else {
            names.emplace(std::move(name));
        }
    }

    if (names.empty()) {

        return thread.id();
    }

    std::stringstream stream{};

    for (const auto& name : names) {
        stream << name << ", ";
    }

    std::string output = stream.str();

    if (0 < output.size()) {
        output.erase(output.size() - 2, 2);
    }

    return output;
}

std::string ActivitySummaryItem::DisplayName() const
{
    sLock lock(shared_lock_);

    if (display_name_.empty()) {

        return contact_.ContactName(id_);
    }

    return display_name_;
}

std::string ActivitySummaryItem::find_text(const ItemLocator& locator) const
{
    const auto & [ itemID, box, accountID ] = locator;
    [[maybe_unused]] const auto& notUsed = accountID;

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            auto mail = activity_.MailText(nym_id_, itemID, box);

            if (mail) {

                return *mail;
            }
        } break;
        default: {
            OT_FAIL
        }
    }

    return {};
}

void ActivitySummaryItem::get_text()
{
    sLock lock(shared_lock_, std::defer_lock);
    Identifier taskID{};
    ItemLocator locator{};

    while (running_) {
        if (newest_item_.Pop(taskID, locator)) {
            const auto text = find_text(locator);
            lock.lock();
            text_ = text;
            lock.unlock();
        }

        Log::Sleep(std::chrono::milliseconds(GET_TEXT_MILLISECONDS));
    }
}

std::string ActivitySummaryItem::ImageURI() const
{
    // TODO

    return {};
}

const proto::StorageThreadItem& ActivitySummaryItem::newest_item(
    const proto::StorageThread& thread) const
{
    const proto::StorageThreadItem* output{nullptr};

    for (const auto& item : thread.item()) {
        if (nullptr == output) {
            output = &item;

            continue;
        }

        if (item.time() > output->time()) {
            output = &item;

            continue;
        }
    }

    OT_ASSERT(nullptr != output)

    return *output;
}

void ActivitySummaryItem::process_thread(
    const network::zeromq::Message& message)
{
    const std::string id(message);
    otWarn << OT_METHOD << __FUNCTION__ << ": Thread " << id << " has updated.."
           << std::endl;
    const Identifier threadID(id);

    OT_ASSERT(false == threadID.empty())

    if (id_ != threadID) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Update not relevant to me ("
               << id_.str() << ")" << std::endl;

        return;
    }

    startup();
}

void ActivitySummaryItem::startup()
{
    auto thread = activity_.Thread(nym_id_, id_);

    OT_ASSERT(thread);

    update(*thread);
}

std::string ActivitySummaryItem::Text() const
{
    sLock lock(shared_lock_);

    return text_;
}

std::string ActivitySummaryItem::ThreadID() const { return id_.str(); }

std::chrono::system_clock::time_point ActivitySummaryItem::Timestamp() const
{
    sLock lock(shared_lock_);

    return time_;
}

StorageBox ActivitySummaryItem::Type() const
{
    sLock lock(shared_lock_);

    return type_;
}

void ActivitySummaryItem::update(const proto::StorageThread& thread)
{
    eLock lock(shared_lock_, std::defer_lock);
    const bool haveItems = check_thread(thread);
    const auto displayName = display_name(thread);
    lock.lock();
    display_name_ = displayName;
    lock.unlock();

    if (false == haveItems) {

        return;
    }

    const auto& item = newest_item(thread);
    const auto time = std::chrono::system_clock::time_point(
        std::chrono::seconds(item.time()));
    const auto box = static_cast<StorageBox>(item.box());
    lock.lock();
    time_ = time;
    text_ = "";
    type_ = box;
    lock.unlock();
    newest_item_.Push(
        Identifier::Random(),
        {Identifier(item.id()), box, Identifier(item.account())});
    parent_.reindex_item(id_, {time, displayName});
}

ActivitySummaryItem::~ActivitySummaryItem()
{
    if (newest_item_thread_ && newest_item_thread_->joinable()) {
        newest_item_thread_->join();
        newest_item_thread_.reset();
    }

    if (startup_ && startup_->joinable()) {
        startup_->join();
        startup_.reset();
    }
}
}  // namespace opentxs::ui::implementation
