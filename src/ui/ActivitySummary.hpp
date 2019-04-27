// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivitySummary.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

namespace opentxs::ui::implementation
{
using ActivitySummaryList = List<
    ActivitySummaryExternalInterface,
    ActivitySummaryInternalInterface,
    ActivitySummaryRowID,
    ActivitySummaryRowInterface,
    ActivitySummaryRowInternal,
    ActivitySummaryRowBlank,
    ActivitySummarySortKey,
    ActivitySummaryPrimaryID>;

class ActivitySummary final : public ActivitySummaryList
{
public:
#if OT_QT
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const override;
#endif
    ~ActivitySummary();

private:
    friend api::client::implementation::UI;

    const ListenerDefinitions listeners_;
    const Flag& running_;

    static const proto::StorageThreadItem& newest_item(
        const proto::StorageThread& thread,
        CustomData& custom);

    void construct_row(
        const ActivitySummaryRowID& id,
        const ActivitySummarySortKey& index,
        const CustomData& custom) const override;
    std::string display_name(const proto::StorageThread& thread) const;

    void process_thread(const std::string& threadID);
    void process_thread(const network::zeromq::Message& message);
    void startup();

    ActivitySummary(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const Flag& running,
        const identifier::Nym& nymID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
    );
    ActivitySummary() = delete;
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    ActivitySummary& operator=(const ActivitySummary&) = delete;
    ActivitySummary& operator=(ActivitySummary&&) = delete;
};
}  // namespace opentxs::ui::implementation
