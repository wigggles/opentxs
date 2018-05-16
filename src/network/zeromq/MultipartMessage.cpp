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

#include "MultipartMessage.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"

#include <zmq.h>

template class opentxs::Pimpl<opentxs::network::zeromq::MultipartMessage>;

namespace opentxs::network::zeromq
{
OTZMQMultipartMessage MultipartMessage::Factory()
{
    return OTZMQMultipartMessage(new implementation::MultipartMessage());
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
MultipartMessage::MultipartMessage()
    : messages_{}
{
}

const Message& MultipartMessage::at(const std::size_t index) const
{
    return const_cast<const Message&>(messages_.at(index).get());
}

Message& MultipartMessage::at(const std::size_t index)
{
    return messages_.at(index).get();
}

const FrameIterator MultipartMessage::begin() const
{
    return FrameIterator(this);
}

const FrameIterator MultipartMessage::end() const
{
    return FrameIterator(this, messages_.size());
}

FrameIterator MultipartMessage::begin() { return FrameIterator(this); }

FrameIterator MultipartMessage::end()
{
    return FrameIterator(this, messages_.size());
}

MultipartMessage* MultipartMessage::clone() const
{
    MultipartMessage* multipartMessage = new MultipartMessage();

    OT_ASSERT(nullptr != multipartMessage);

    for (auto& message : messages_) {
        multipartMessage->messages_.emplace_back(message);
    }

    return multipartMessage;
}

std::size_t MultipartMessage::size() const { return messages_.size(); }

Message& MultipartMessage::AddFrame()
{
    OTZMQMessage message = Message::Factory();

    messages_.emplace_back(message);
    return messages_.back().get();
}

Message& MultipartMessage::AddFrame(const opentxs::Data& input)
{
    OTZMQMessage message = Message::Factory(input);

    messages_.emplace_back(message);
    return messages_.back().get();
}

Message& MultipartMessage::AddFrame(const std::string& input)
{
    OTZMQMessage message = Message::Factory(input);

    messages_.emplace_back(message);
    return messages_.back().get();
}

// MultipartMessage::~MultipartMessage()
//{
//}
}  // namespace opentxs::network::zeromq::implementation
