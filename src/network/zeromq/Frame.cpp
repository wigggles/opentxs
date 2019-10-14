// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Frame.hpp"

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Frame>;

namespace opentxs
{
network::zeromq::Frame* Factory::ZMQFrame()
{
    using ReturnType = network::zeromq::implementation::Frame;

    return new ReturnType();
}

network::zeromq::Frame* Factory::ZMQFrame(
    const void* data,
    const std::size_t size)
{
    using ReturnType = network::zeromq::implementation::Frame;

    return new ReturnType(data, size);
}

network::zeromq::Frame* Factory::ZMQFrame(const ProtobufType& data)
{
    using ReturnType = network::zeromq::implementation::Frame;

    return new ReturnType(data);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::implementation
{
Frame::Frame()
    : zeromq::Frame()
    , message_()
{
    const auto init = zmq_msg_init(&message_);

    OT_ASSERT(0 == init);
}

Frame::Frame(const std::size_t bytes)
    : Frame()
{
    const auto init = zmq_msg_init_size(&message_, bytes);

    OT_ASSERT(0 == init);
}

Frame::Frame(const ProtobufType& input)
    : Frame(input.ByteSize())
{
    input.SerializeToArray(zmq_msg_data(&message_), zmq_msg_size(&message_));
}

Frame::Frame(const void* data, const std::size_t bytes)
    : Frame(bytes)
{
    std::memcpy(zmq_msg_data(&message_), data, zmq_msg_size(&message_));
}

Frame::operator zmq_msg_t*() { return &message_; }

Frame::operator std::string() const
{
    std::string output(static_cast<const char*>(data()), size());

    return output;
}

Frame* Frame::clone() const
{
    return new Frame(zmq_msg_data(&message_), zmq_msg_size(&message_));
}

const void* Frame::data() const { return zmq_msg_data(&message_); }

std::size_t Frame::size() const { return zmq_msg_size(&message_); }

Frame::~Frame() { zmq_msg_close(&message_); }
}  // namespace opentxs::network::zeromq::implementation
