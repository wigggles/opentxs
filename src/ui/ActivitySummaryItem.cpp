// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "ui/ActivitySummaryItem.hpp"  // IWYU pragma: associated

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/Widget.hpp"

#define GET_TEXT_MILLISECONDS 10

#define OT_METHOD "opentxs::ui::implementation::ActivitySummaryItem::"

namespace opentxs::factory
{
auto ActivitySummaryItem(
    const ui::implementation::ActivitySummaryInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivitySummaryRowID& rowID,
    const ui::implementation::ActivitySummarySortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const Flag& running) noexcept
    -> std::shared_ptr<ui::implementation::ActivitySummaryRowInternal>
{
    using ReturnType = ui::implementation::ActivitySummaryItem;

    return std::make_shared<ReturnType>(
        parent, api, publisher, nymID, rowID, sortKey, custom, running);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ActivitySummaryItem::ActivitySummaryItem(
    const ActivitySummaryInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ActivitySummaryRowID& rowID,
    const ActivitySummarySortKey& sortKey,
    const CustomData& custom,
    const Flag& running) noexcept
    : ActivitySummaryItemRow(parent, api, publisher, rowID, true)
    , running_(running)
    , nym_id_(nymID)
    , key_{sortKey}
    , thread_()
    , display_name_{std::get<1>(key_)}
    , text_("")
    , type_(extract_custom<StorageBox>(custom, 1))
    , time_(extract_custom<Time>(custom, 3))
    , newest_item_thread_(nullptr)
    , newest_item_()
    , next_task_id_(0)
    , break_(false)
{
    startup(custom, newest_item_);
    newest_item_thread_.reset(
        new std::thread(&ActivitySummaryItem::get_text, this));

    OT_ASSERT(newest_item_thread_)
}

auto ActivitySummaryItem::DisplayName() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    if (display_name_.empty()) { return api_.Contacts().ContactName(row_id_); }

    return display_name_;
}

auto ActivitySummaryItem::find_text(
    const PasswordPrompt& reason,
    const ItemLocator& locator) const noexcept -> std::string
{
    const auto& [itemID, box, accountID] = locator;

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            auto text = api_.Activity().MailText(
                nym_id_, Identifier::Factory(itemID), box, reason);

            if (text) {

                return *text;
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Mail item does not exist.")
                    .Flush();
            }
        } break;
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            auto text = api_.Activity().PaymentText(nym_id_, itemID, accountID);

            if (text) {

                return *text;
            } else {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Cheque item does not exist.")
                    .Flush();
            }
        } break;
        default: {
            OT_FAIL
        }
    }

    return {};
}

void ActivitySummaryItem::get_text() noexcept
{
    auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);
    sLock lock(shared_lock_, std::defer_lock);
    ItemLocator locator{};

    while (running_) {
        if (break_.load()) { return; }

        int taskID{0};

        if (newest_item_.Pop(taskID, locator)) {
            const auto text = find_text(reason, locator);
            lock.lock();
            text_ = text;
            lock.unlock();
            UpdateNotify();
        }

        Sleep(std::chrono::milliseconds(GET_TEXT_MILLISECONDS));
    }
}

auto ActivitySummaryItem::ImageURI() const noexcept -> std::string
{
    // TODO

    return {};
}

#if OT_QT
QVariant ActivitySummaryItem::qt_data(const int column, int role) const noexcept
{
    switch (column) {
        case 0: {
            return ThreadID().c_str();
        }
        case 1: {
            return DisplayName().c_str();
        }
        case 2: {
            return ImageURI().c_str();
        }
        case 3: {
            return Text().c_str();
        }
        case 4: {
            QDateTime qdatetime;
            qdatetime.setSecsSinceEpoch(
                std::chrono::system_clock::to_time_t(Timestamp()));
            return qdatetime;
        }
        case 5: {
            return static_cast<int>(Type());
        }
        default: {
            return {};
        }
    }
}
#endif

void ActivitySummaryItem::reindex(
    const ActivitySummarySortKey& key,
    const CustomData& custom) noexcept
{
    eLock lock(shared_lock_);
    key_ = key;
    lock.unlock();
    startup(custom, newest_item_);
}

void ActivitySummaryItem::startup(
    const CustomData& custom,
    UniqueQueue<ItemLocator>& queue) noexcept
{
    const auto id = extract_custom<std::string>(custom, 0);
    const auto account = extract_custom<std::string>(custom, 2);
    ItemLocator locator{id, type_, account};
    queue.Push(++next_task_id_, locator);
}

auto ActivitySummaryItem::Text() const noexcept -> std::string
{
    sLock lock(shared_lock_);

    return text_;
}

auto ActivitySummaryItem::ThreadID() const noexcept -> std::string
{
    return row_id_->str();
}

auto ActivitySummaryItem::Timestamp() const noexcept
    -> std::chrono::system_clock::time_point
{
    sLock lock(shared_lock_);

    return time_;
}

auto ActivitySummaryItem::Type() const noexcept -> StorageBox
{
    sLock lock(shared_lock_);

    return type_;
}

ActivitySummaryItem::~ActivitySummaryItem()
{
    break_.store(true);

    if (newest_item_thread_ && newest_item_thread_->joinable()) {
        newest_item_thread_->join();
        newest_item_thread_.reset();
    }
}
}  // namespace opentxs::ui::implementation
