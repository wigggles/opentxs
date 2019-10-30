// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ActivityThreadItemRow =
    Row<ActivityThreadRowInternal,
        ActivityThreadInternalInterface,
        ActivityThreadRowID>;

class ActivityThreadItem : public ActivityThreadItemRow
{
public:
    opentxs::Amount Amount() const noexcept override { return 0; }
    bool Deposit() const noexcept override { return false; }
    std::string DisplayAmount() const noexcept override { return {}; }
    bool Loading() const noexcept final { return loading_.get(); }
    bool MarkRead() const noexcept final;
    std::string Memo() const noexcept override { return {}; }
    bool Pending() const noexcept final { return pending_.get(); }
    std::string Text() const noexcept final;
    std::chrono::system_clock::time_point Timestamp() const noexcept final;
    StorageBox Type() const noexcept final { return box_; }

    void reindex(
        const ActivityThreadSortKey& key,
        const CustomData& custom) noexcept final;

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept final;
#endif

    ~ActivityThreadItem() override = default;

protected:
    const identifier::Nym& nym_id_;
    const std::chrono::system_clock::time_point time_;
    const Identifier& item_id_;
    const StorageBox& box_;
    const Identifier& account_id_;
    std::string text_;
    OTFlag loading_;
    OTFlag pending_;

    ActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom,
        const bool loading,
        const bool pending) noexcept;

private:
    ActivityThreadItem() = delete;
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    ActivityThreadItem& operator=(const ActivityThreadItem&) = delete;
    ActivityThreadItem& operator=(ActivityThreadItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
