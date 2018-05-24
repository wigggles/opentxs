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

#include "ListenCallback.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::ListenCallback::"

namespace opentxs::network::zeromq
{
OTZMQListenCallback ListenCallback::Factory(
    zeromq::ListenCallback::ReceiveCallback callback)
{
    return OTZMQListenCallback(new implementation::ListenCallback(callback));
}

OTZMQListenCallback ListenCallback::Factory()
{
    return OTZMQListenCallback(
        new implementation::ListenCallback([](const Message&) -> void {}));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ListenCallback::ListenCallback(zeromq::ListenCallback::ReceiveCallback callback)
    : callback_(callback)
{
}

ListenCallback* ListenCallback::clone() const
{
    return new ListenCallback(callback_);
}

void ListenCallback::Process(const zeromq::Message& message) const
{
    callback_(message);
}

ListenCallback::~ListenCallback() {}
}  // namespace opentxs::network::zeromq::implementation
