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

#include "Message.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::Message>;

namespace opentxs::network::zeromq
{
OTZMQMessage Message::Factory()
{
    return OTZMQMessage(new implementation::Message());
}

OTZMQMessage Message::Factory(const Data& input)
{
    return OTZMQMessage(new implementation::Message(input));
}

OTZMQMessage Message::Factory(const std::string& input)
{
    return OTZMQMessage(new implementation::Message(input));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Message::Message()
    : message_(new zmq_msg_t)
{
    OT_ASSERT(nullptr != message_);

    const auto init = zmq_msg_init(message_);

    OT_ASSERT(0 == init);
}

Message::Message(const Data& input)
    : message_(new zmq_msg_t)
{
    OT_ASSERT(nullptr != message_);

    const auto init = zmq_msg_init_size(message_, input.GetSize());
    OTPassword::safe_memcpy(
        zmq_msg_data(message_),
        zmq_msg_size(message_),
        input.GetPointer(),
        input.GetSize(),
        false);

    OT_ASSERT(0 == init);
}

Message::Message(const std::string& input)
    : message_(new zmq_msg_t)
{
    OT_ASSERT(nullptr != message_);

    const auto init = zmq_msg_init_size(message_, input.size());
    OTPassword::safe_memcpy(
        zmq_msg_data(message_),
        zmq_msg_size(message_),
        input.data(),
        input.size(),
        false);

    OT_ASSERT(0 == init);
}

Message::operator zmq_msg_t*() { return message_; }

Message::operator std::string() const
{
    std::string output(static_cast<const char*>(data()), size());

    return output;
}

Message* Message::clone() const { return new Message(std::string(*this)); }

const void* Message::data() const
{
    OT_ASSERT(nullptr != message_);

    return zmq_msg_data(message_);
}

std::size_t Message::size() const
{
    OT_ASSERT(nullptr != message_);

    return zmq_msg_size(message_);
}

Message::~Message()
{
    if (nullptr != message_) {
        zmq_msg_close(message_);
    }
}
}  // namespace opentxs::network::zeromq::implementation
