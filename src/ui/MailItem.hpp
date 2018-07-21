// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MAILITEM_IMPLEMENTATION_HPP
#define OPENTXS_UI_MAILITEM_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class MailItem final : public ActivityThreadItem
{
public:
    ~MailItem();

private:
    friend Factory;

    std::unique_ptr<std::thread> load_{nullptr};

    void load();

    MailItem(
        const ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom,
        const api::Activity& activity,
        const bool loading,
        const bool pending);
    MailItem(
        const ActivityThreadInternalInterface& parent,
        const network::zeromq::Context& zmq,
        const network::zeromq::PublishSocket& publisher,
        const api::ContactManager& contact,
        const Identifier& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom,
        const api::Activity& activity);
    MailItem() = delete;
    MailItem(const MailItem&) = delete;
    MailItem(MailItem&&) = delete;
    MailItem& operator=(const MailItem&) = delete;
    MailItem& operator=(MailItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
#endif  // OPENTXS_UI_MAILITEM_IMPLEMENTATION_HPP
