// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ContactItem.hpp"

#include "internal/ui/UI.hpp"
#include "Row.hpp"

#include "ContactItem.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactItem>;

namespace opentxs
{
ui::implementation::ContactSubsectionRowInternal* Factory::ContactItemWidget(
    const ui::implementation::ContactSubsectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ui::implementation::ContactSubsectionRowID& rowID,
    const ui::implementation::ContactSubsectionSortKey& sortKey,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::ContactItem(
        parent, api, publisher, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactItem::ContactItem(
    const ContactSubsectionInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const ContactSubsectionRowID& rowID,
    const ContactSubsectionSortKey& sortKey,
    const CustomData& custom) noexcept
    : ContactItemRow(parent, api, publisher, rowID, true)
    , item_{new opentxs::ContactItem(
          extract_custom<opentxs::ContactItem>(custom))}
{
    OT_ASSERT(item_);
}

void ContactItem::reindex(
    const ContactSubsectionSortKey&,
    const CustomData& custom) noexcept
{
    eLock lock(shared_lock_);
    item_.reset(
        new opentxs::ContactItem(extract_custom<opentxs::ContactItem>(custom)));

    OT_ASSERT(item_);
}
}  // namespace opentxs::ui::implementation
