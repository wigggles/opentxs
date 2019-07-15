// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#include "Message.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Message>;

namespace opentxs
{
network::zeromq::Message* Factory::ZMQMessage()
{
    using ReturnType = opentxs::network::zeromq::implementation::Message;

    return new ReturnType();
}

network::zeromq::Message* Factory::ZMQMessage(
    const void* data,
    const std::size_t size)
{
    using ReturnType = opentxs::network::zeromq::implementation::Message;
    auto output = new ReturnType();

    if (nullptr != output) { output->AddFrame(data, size); }

    return output;
}

network::zeromq::Message* Factory::ZMQMessage(const ProtobufType& data)
{
    using ReturnType = opentxs::network::zeromq::implementation::Message;
    auto output = new ReturnType();

    if (nullptr != output) { output->AddFrame(data); }

    return output;
}
}  // namespace opentxs

namespace opentxs::network::zeromq
{
OTZMQMessage Message::Factory()
{
    return OTZMQMessage{new implementation::Message()};
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Message::Message()
    : messages_()
{
}

Message::Message(const Message& rhs)
    : zeromq::Message()
    , messages_()
{
    for (auto& message : rhs.messages_) { messages_.emplace_back(message); }
}

Frame& Message::AddFrame()
{
    messages_.emplace_back(Factory::ZMQFrame());

    return messages_.back().get();
}

Frame& Message::AddFrame(const void* input, const std::size_t size)
{
    messages_.emplace_back(Factory::ZMQFrame(input, size));

    return messages_.back().get();
}

Frame& Message::AddFrame(const ProtobufType& input)
{
    messages_.emplace_back(Factory::ZMQFrame(input));

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
    auto position = body_position();

    return FrameSection(this, position, messages_.size() - position);
}

const Frame& Message::Body_at(const std::size_t index) const
{
    return Body().at(index);
}

FrameIterator Message::Body_begin() const { return Body().begin(); }

FrameIterator Message::Body_end() const { return Body().end(); }

std::size_t Message::body_position() const
{
    std::size_t position{0};

    if (true == hasDivider()) { position = findDivider() + 1; }

    return position;
}

FrameIterator Message::end() const
{
    return FrameIterator(this, messages_.size());
}

// This function is only called by RouterSocket.  It makes sure that if a
// message has two or more frames, and no delimiter, then a delimiter is
// inserted after the first frame.
void Message::EnsureDelimiter()
{
    if (1 < messages_.size() && !hasDivider()) {
        auto it = messages_.begin();
        messages_.emplace(++it, Factory::ZMQFrame());
    }
    // These cases should never happen.  When this function is called, there
    // should always be at least two frames.
    else if (0 < messages_.size() && !hasDivider()) {
        messages_.emplace(messages_.begin(), Factory::ZMQFrame());
    } else if (!hasDivider()) {
        messages_.emplace_back(Factory::ZMQFrame());
    }
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
               messages_.begin(),
               messages_.end(),
               [](const OTZMQFrame& msg) -> bool {
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

void Message::PrependEmptyFrame()
{
    OTZMQFrame message{Factory::ZMQFrame()};

    auto it = messages_.emplace(messages_.begin(), message);

    OT_ASSERT(messages_.end() != it);
}

bool Message::set_field(const std::size_t position, const zeromq::Frame& input)
{
    const auto effectivePosition = body_position() + position;

    if (effectivePosition >= messages_.size()) { return false; }

    messages_[effectivePosition] = input;

    return true;
}

std::size_t Message::size() const { return messages_.size(); }
}  // namespace opentxs::network::zeromq::implementation
