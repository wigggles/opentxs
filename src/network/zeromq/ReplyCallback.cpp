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

#include "stdafx.hpp"

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "ReplyCallback.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::ReplyCallback::"

namespace opentxs::network::zeromq
{
OTZMQReplyCallback ReplyCallback::Factory(
    zeromq::ReplyCallback::ReceiveCallback callback)
{
    return OTZMQReplyCallback(new implementation::ReplyCallback(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ReplyCallback::ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback)
    : callback_(callback)
{
}

ReplyCallback* ReplyCallback::clone() const
{
    return new ReplyCallback(callback_);
}

OTZMQMessage ReplyCallback::Process(const zeromq::Message& message) const
{
    return callback_(message);
}

ReplyCallback::~ReplyCallback() {}
}  // namespace opentxs::network::zeromq::implementation
