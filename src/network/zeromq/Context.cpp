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

#include "opentxs/network/zeromq/implementation/Context.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/implementation/Message.hpp"
#include "opentxs/network/zeromq/implementation/ReplySocket.hpp"
#include "opentxs/network/zeromq/implementation/RequestSocket.hpp"

#include <zmq.h>

namespace opentxs::network::zeromq::implementation
{
Context::Context()
    : context_(zmq_ctx_new())
{
    OT_ASSERT(nullptr != context_);
    OT_ASSERT(1 == zmq_has("curve"));
}

Context::operator void*() const { return context_; }

std::unique_ptr<zeromq::Message> Context::NewMessage() const
{
    std::unique_ptr<zeromq::Message> output{nullptr};
    output.reset(new zeromq::implementation::Message());

    return output;
}

std::unique_ptr<zeromq::Message> Context::NewMessage(const Data& input) const
{
    std::unique_ptr<zeromq::Message> output{nullptr};
    output.reset(new zeromq::implementation::Message(input));

    return output;
}

std::unique_ptr<zeromq::Message> Context::NewMessage(
    const std::string& input) const
{
    std::unique_ptr<zeromq::Message> output{nullptr};
    output.reset(new zeromq::implementation::Message(input));

    return output;
}

std::unique_ptr<zeromq::ReplySocket> Context::NewReplySocket() const
{
    std::unique_ptr<zeromq::ReplySocket> output(new ReplySocket(*this));

    return output;
}

std::unique_ptr<zeromq::RequestSocket> Context::NewRequestSocket() const
{
    std::unique_ptr<zeromq::RequestSocket> output(new RequestSocket(*this));

    return output;
}

Context::~Context()
{
    if (nullptr != context_) {
        zmq_ctx_shutdown(context_);
    }
}
}  // namespace opentxs::network::zeromq::implementation
