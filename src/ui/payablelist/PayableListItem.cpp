// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "ui/payablelist/PayableListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

namespace opentxs::factory
{
auto PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::client::internal::Manager& api,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const std::string& paymentcode,
    const proto::ContactItemType& currency) noexcept
    -> std::shared_ptr<ui::implementation::PayableListRowInternal>
{
    using ReturnType = ui::implementation::PayableListItem;

    return std::make_shared<ReturnType>(
        parent, api, rowID, key, paymentcode, currency);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PayableListItem::PayableListItem(
    const PayableInternalInterface& parent,
    const api::client::internal::Manager& api,
    const PayableListRowID& rowID,
    const PayableListSortKey& key,
    const std::string& paymentcode,
    const proto::ContactItemType& currency) noexcept
    : ot_super(parent, api, rowID, key)
    , payment_code_(paymentcode)
    , currency_(currency)
{
}

auto PayableListItem::PaymentCode() const noexcept -> std::string
{
    Lock lock(lock_);

    return payment_code_;
}

#if OT_QT
QVariant PayableListItem::qt_data(const int column, int role) const noexcept
{
    if (0 == column) { return ContactListItem::qt_data(column, role); }

    switch (role) {
        case Qt::DisplayRole: {

            return PaymentCode().c_str();
        }
        default: {
            return {};
        }
    }
}
#endif

void PayableListItem::reindex(
    const ContactListSortKey& key,
    CustomData& custom) noexcept
{
    ot_super::reindex(key, custom);
    const auto contact = api_.Contacts().Contact(row_id_);

    OT_ASSERT(contact);

    payment_code_ = contact->PaymentCode(currency_);
}
}  // namespace opentxs::ui::implementation
