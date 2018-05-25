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

#include "Message.hpp"

#include "stdafx.hpp"

#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"

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
    auto multipartMessage = new implementation::Message();

    OT_ASSERT(nullptr != multipartMessage);

    multipartMessage->AddFrame(input);

    return OTZMQMessage(multipartMessage);
}

OTZMQMessage Message::Factory(const std::string& input)
{
    auto multipartMessage = new implementation::Message();

    OT_ASSERT(nullptr != multipartMessage);

    multipartMessage->AddFrame(input);

    return OTZMQMessage(multipartMessage);
}

OTZMQMessage Message::ReplyFactory(const Message& request)
{
    auto output = new implementation::Message();

    if (0 < request.Header().size()) {
        for (const auto& frame : request.Header()) { output->AddFrame(frame); }

        output->AddFrame();
    }

    return OTZMQMessage(output);
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Message::Message()
    : messages_{}
{
}

Frame& Message::AddFrame()
{
    OTZMQFrame message = Frame::Factory();

    messages_.emplace_back(message);
    return messages_.back().get();
}

Frame& Message::AddFrame(const opentxs::Data& input)
{
    OTZMQFrame message = Frame::Factory(input);

    messages_.emplace_back(message);
    return messages_.back().get();
}

Frame& Message::AddFrame(const std::string& input)
{
    OTZMQFrame message = Frame::Factory(input);

    messages_.emplace_back(message);
    return messages_.back().get();
}

const Frame& Message::at(const std::size_t index) const
{
    OT_ASSERT(messages_.size() > index);

    return const_cast<const Frame&>(messages_.at(index).get());
}

Frame& Message::at(const std::size_t index)
{
    OT_ASSERT(messages_.size() > index);

    return messages_.at(index).get();
}

FrameIterator Message::begin() const { return FrameIterator(this); }

const FrameSection Message::Body() const
{
    auto position = 0;

    if (true == hasDivider()) { position = findDivider() + 1; }

    return FrameSection(this, position, messages_.size() - position);
}

const Frame& Message::Body_at(const std::size_t index) const
{
    return Body().at(index);
}

FrameIterator Message::Body_begin() const { return Body().begin(); }

FrameIterator Message::Body_end() const { return Body().end(); }

Message* Message::clone() const
{
    Message* multipartMessage = new Message();

    OT_ASSERT(nullptr != multipartMessage);

    for (auto& message : messages_) {
        multipartMessage->messages_.emplace_back(message);
    }

    return multipartMessage;
}

FrameIterator Message::end() const
{
    return FrameIterator(this, messages_.size());
}

std::size_t Message::findDivider() const
{
    std::size_t divider = 0;

    for (auto& message : messages_) {
        if (0 == message->size()) { break; }
        ++divider;
    }

    return divider;
}

bool Message::hasDivider() const
{
    return std::find_if(
               messages_.begin(), messages_.end(), [](OTZMQFrame msg) -> bool {
                   return 0 == msg->size();
               }) != messages_.end();
}

const Frame& Message::Header_at(const std::size_t index) const
{
    return Header().at(index);
}

const FrameSection Message::Header() const
{
    auto size = 0;

    if (true == hasDivider()) { size = findDivider(); }

    return FrameSection(this, 0, size);
}

FrameIterator Message::Header_begin() const { return Header().begin(); }

FrameIterator Message::Header_end() const { return Header().end(); }

std::size_t Message::size() const { return messages_.size(); }

// Message::~Message()
//{
//}
}  // namespace opentxs::network::zeromq::implementation
