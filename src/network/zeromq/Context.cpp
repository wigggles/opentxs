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

#include "Context.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/PairSocket.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include <zmq.h>

namespace opentxs::network::zeromq
{
OTZMQContext Context::Factory()
{
    return OTZMQContext(new implementation::Context());
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Context::Context()
    : context_(zmq_ctx_new())
{
    OT_ASSERT(nullptr != context_);
    OT_ASSERT(1 == zmq_has("curve"));
}

Context::operator void*() const { return context_; }

Context* Context::clone() const { return new Context; }

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback) const
{
    return PairSocket::Factory(*this, callback);
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const opentxs::network::zeromq::PairSocket& peer) const
{
    return PairSocket::Factory(callback, peer);
}

OTZMQPairSocket Context::PairSocket(
    const opentxs::network::zeromq::ListenCallback& callback,
    const std::string& endpoint) const
{
    return PairSocket::Factory(*this, callback, endpoint);
}

OTZMQPublishSocket Context::PublishSocket() const
{
    return PublishSocket::Factory(*this);
}

OTZMQPullSocket Context::PullSocket() const
{
    return PullSocket::Factory(*this);
}

OTZMQPullSocket Context::PullSocket(const ListenCallback& callback) const
{
    return PullSocket::Factory(*this, callback);
}

OTZMQPushSocket Context::PushSocket() const
{
    return PushSocket::Factory(*this);
}

OTZMQReplySocket Context::ReplySocket(const ReplyCallback& callback) const
{
    return ReplySocket::Factory(*this, callback);
}

OTZMQRequestSocket Context::RequestSocket() const
{
    return RequestSocket::Factory(*this);
}

OTZMQSubscribeSocket Context::SubscribeSocket(
    const ListenCallback& callback) const
{
    return SubscribeSocket::Factory(*this, callback);
}

Context::~Context()
{
    if (nullptr != context_) {
        zmq_ctx_shutdown(context_);
    }
}
}  // namespace opentxs::network::zeromq::implementation
