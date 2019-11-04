// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivitySummaryItemRow =
    Row<ActivitySummaryRowInternal,
        ActivitySummaryInternalInterface,
        ActivitySummaryRowID>;

class ActivitySummaryItem final : public ActivitySummaryItemRow
{
public:
    std::string DisplayName() const noexcept final;
    std::string ImageURI() const noexcept final;
    std::string Text() const noexcept final;
    std::string ThreadID() const noexcept final;
    std::chrono::system_clock::time_point Timestamp() const noexcept final;
    StorageBox Type() const noexcept final;

    void reindex(
        const ActivitySummarySortKey& key,
        const CustomData& custom) noexcept final;

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept final;
#endif

    ~ActivitySummaryItem() final;

private:
    friend opentxs::Factory;
    // id, box, account
    using ItemLocator = std::tuple<std::string, StorageBox, std::string>;

    const Flag& running_;
    const OTNymID nym_id_;
    ActivitySummarySortKey key_;
    std::shared_ptr<proto::StorageThread> thread_;
    std::string& display_name_;
    std::string text_{""};
    StorageBox type_{StorageBox::UNKNOWN};
    std::chrono::system_clock::time_point time_;
    std::unique_ptr<std::thread> newest_item_thread_;
    UniqueQueue<ItemLocator> newest_item_;
    std::atomic<int> next_task_id_;
    std::atomic<bool> break_;

    std::string find_text(
        const PasswordPrompt& reason,
        const ItemLocator& locator) const noexcept;

    void get_text() noexcept;
    void startup(
        const CustomData& custom,
        UniqueQueue<ItemLocator>& queue) noexcept;

    ActivitySummaryItem(
        const ActivitySummaryInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ActivitySummaryRowID& rowID,
        const ActivitySummarySortKey& sortKey,
        const CustomData& custom,
        const Flag& running) noexcept;
    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    ActivitySummaryItem& operator=(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem& operator=(ActivitySummaryItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
