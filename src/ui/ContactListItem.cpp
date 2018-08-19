// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactListItem.hpp"

#include <locale>

#include "InternalUI.hpp"
#include "Row.hpp"

#include "ContactListItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactListItem>;

//#define OT_METHOD "opentxs::ui::implementation::ContactListItem::"

namespace opentxs
{
ui::internal::ContactListItem* Factory::ContactListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key)
{
    return new ui::implementation::ContactListItem(
        parent, api, publisher, rowID, key);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactListItem::ContactListItem(
    const ContactListInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ContactListRowID& rowID,
    const ContactListSortKey& key)
    : ContactListItemRow(parent, api, publisher, rowID, true)
    , key_(key)
{
}

std::string ContactListItem::ContactID() const { return row_id_->str(); }

std::string ContactListItem::DisplayName() const
{
    Lock lock(lock_);

    return key_;
}

std::string ContactListItem::ImageURI() const
{
    // TODO

    return {};
}

std::string ContactListItem::Section() const
{
    Lock lock(lock_);

    if (row_id_ == parent_.ID()) { return {"ME"}; }

    if (key_.empty()) { return {" "}; }

    std::locale locale;
    std::string output{" "};
    output[0] = std::toupper(key_[0], locale);

    return output;
}

void ContactListItem::reindex(const ContactListSortKey& key, const CustomData&)
{
    Lock lock(lock_);
    key_ = key;
}
}  // namespace opentxs::ui::implementation
