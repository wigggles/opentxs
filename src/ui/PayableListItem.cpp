// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/PayableListItem.hpp"

#include "internal/ui/UI.hpp"
#include "Row.hpp"

#include "PayableListItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::PayableListItem>;

namespace opentxs
{
ui::internal::PayableListItem* Factory::PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const std::string& paymentcode,
    const proto::ContactItemType& currency)
{
    return new ui::implementation::PayableListItem(
        parent, api, publisher, rowID, key, paymentcode, currency);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PayableListItem::PayableListItem(
    const PayableInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const PayableListRowID& rowID,
    const PayableListSortKey& key,
    const std::string& paymentcode,
    const proto::ContactItemType& currency)
    : ot_super(parent, api, publisher, rowID, key)
    , payment_code_(paymentcode)
    , currency_(currency)
{
}

std::string PayableListItem::PaymentCode() const
{
    Lock lock(lock_);

    return payment_code_;
}

void PayableListItem::reindex(
    const ContactListSortKey& key,
    const CustomData& custom)
{
    ot_super::reindex(key, custom);
    const auto contact = api_.Contacts().Contact(row_id_);

    OT_ASSERT(contact);

    payment_code_ = contact->PaymentCode(currency_);
}
}  // namespace opentxs::ui::implementation
