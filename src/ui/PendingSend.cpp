// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"

#include "internal/ui/UI.hpp"
#include "Row.hpp"

#include <memory>
#include <thread>

#include "PendingSend.hpp"

//#define OT_METHOD "opentxs::ui::implementation::PendingSend::"

namespace opentxs
{
ui::implementation::ActivityThreadRowInternal* Factory::PendingSend(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::PendingSend(
        parent, api, publisher, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
PendingSend::PendingSend(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const network::zeromq::socket::Publish& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    const CustomData& custom) noexcept
    : ActivityThreadItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          false,
          true)
    , amount_(extract_custom<opentxs::Amount>(custom, 1))
    , display_amount_(extract_custom<std::string>(custom, 2))
    , memo_(extract_custom<std::string>(custom, 3))
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
}
}  // namespace opentxs::ui::implementation
