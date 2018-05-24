/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "ListenCallbackSwig.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallbackSwig.hpp"

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::ListenCallbackSwig::"

namespace opentxs::network::zeromq
{
OTZMQListenCallback ListenCallback::Factory(
    opentxs::ListenCallbackSwig* callback)
{
    return OTZMQListenCallback(
        new implementation::ListenCallbackSwig(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ListenCallbackSwig::ListenCallbackSwig(opentxs::ListenCallbackSwig* callback)
    : callback_(callback)
{
    if (nullptr == callback_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid callback pointer"
              << std::endl;

        OT_FAIL;
    }
}

ListenCallbackSwig* ListenCallbackSwig::clone() const
{
    return new ListenCallbackSwig(callback_);
}

void ListenCallbackSwig::Process(const zeromq::Message& message) const
{
    OT_ASSERT(nullptr != callback_)

    callback_->Process(message);
}

ListenCallbackSwig::~ListenCallbackSwig() {}
}  // namespace opentxs::network::zeromq::implementation
