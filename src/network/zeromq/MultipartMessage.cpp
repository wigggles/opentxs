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

OTZMQMultipartMessage MultipartMessage::Factory(const Data& input)
{
    auto multipartMessage = new implementation::MultipartMessage();

    OT_ASSERT(nullptr != multipartMessage);

    multipartMessage->AddFrame(input);

    return OTZMQMultipartMessage(multipartMessage);
}

OTZMQMultipartMessage MultipartMessage::Factory(const std::string& input)
{
    auto multipartMessage = new implementation::MultipartMessage();

    OT_ASSERT(nullptr != multipartMessage);

    multipartMessage->AddFrame(input);

    return OTZMQMultipartMessage(multipartMessage);
}

OTZMQMultipartMessage MultipartMessage::ReplyFactory(
    const MultipartMessage& request)
{
    auto output = new implementation::MultipartMessage();

    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) { output->AddFrame(frame); }

        output->AddFrame();
    }

    return OTZMQMultipartMessage(output);
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
MultipartMessage::MultipartMessage()
    : messages_{}
{
}

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

const Message& MultipartMessage::at(const std::size_t index) const
{
    OT_ASSERT(messages_.size() > index);

    return const_cast<const Message&>(messages_.at(index).get());
}

Message& MultipartMessage::at(const std::size_t index)
{
    OT_ASSERT(messages_.size() > index);

    return messages_.at(index).get();
}

FrameIterator MultipartMessage::begin() const { return FrameIterator(this); }

const FrameSection MultipartMessage::Body() const
{
    auto position = 0;

    if (true == hasDivider()) { position = findDivider() + 1; }

    return FrameSection(this, position, messages_.size() - position);
}

const Message& MultipartMessage::Body_at(const std::size_t index) const
{
    return Body().at(index);
}

FrameIterator MultipartMessage::Body_begin() const { return Body().begin(); }

FrameIterator MultipartMessage::Body_end() const { return Body().end(); }

MultipartMessage* MultipartMessage::clone() const
{
    MultipartMessage* multipartMessage = new MultipartMessage();

    OT_ASSERT(nullptr != multipartMessage);

    for (auto& message : messages_) {
        multipartMessage->messages_.emplace_back(message);
    }

    return multipartMessage;
}

FrameIterator MultipartMessage::end() const
{
    return FrameIterator(this, messages_.size());
}

std::size_t MultipartMessage::findDivider() const
{
    std::size_t divider = 0;

    for (auto& message : messages_) {
        if (0 == message->size()) { break; }
        ++divider;
    }

    return divider;
}

bool MultipartMessage::hasDivider() const
{
    return std::find_if(
               messages_.begin(),
               messages_.end(),
               [](OTZMQMessage msg) -> bool { return 0 == msg->size(); }) !=
           messages_.end();
}

const Message& MultipartMessage::Header_at(const std::size_t index) const
{
    return Header().at(index);
}

const FrameSection MultipartMessage::Header() const
{
    auto size = 0;

    if (true == hasDivider()) { size = findDivider(); }

    return FrameSection(this, 0, size);
}

FrameIterator MultipartMessage::Header_begin() const
{
    return Header().begin();
}

FrameIterator MultipartMessage::Header_end() const { return Header().end(); }

std::size_t MultipartMessage::size() const { return messages_.size(); }

// MultipartMessage::~MultipartMessage()
//{
//}
}  // namespace opentxs::network::zeromq::implementation
