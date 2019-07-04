// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Frame.hpp"

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Frame>;

namespace opentxs::network::zeromq
{
OTZMQFrame Frame::Factory() { return OTZMQFrame(new implementation::Frame()); }

OTZMQFrame Frame::Factory(const Data& input)
{
    return OTZMQFrame(new implementation::Frame(input));
}

OTZMQFrame Frame::Factory(const std::string& input)
{
    return OTZMQFrame(new implementation::Frame(input));
}

OTZMQFrame Frame::Factory(const ProtobufType& input)
{
    return OTZMQFrame(new implementation::Frame(input));
}
}  // namespace opentxs::network::zeromq

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
    : zeromq::Frame()
    , message_()
{
    const auto init = zmq_msg_init_size(&message_, bytes);

    OT_ASSERT(0 == init);
}

Frame::Frame(const ProtobufType& input)
    : Frame(input.ByteSize())
{
    input.SerializeToArray(zmq_msg_data(&message_), zmq_msg_size(&message_));
}

Frame::Frame(const Data& input)
    : Frame(input.size())
{
    if (0 < input.size()) {
        OTPassword::safe_memcpy(
            zmq_msg_data(&message_),
            zmq_msg_size(&message_),
            input.data(),
            input.size(),
            false);
    }
}

Frame::Frame(const std::string& input)
    : Frame(input.size())
{
    if (0 < input.size()) {
        OTPassword::safe_memcpy(
            zmq_msg_data(&message_),
            zmq_msg_size(&message_),
            input.data(),
            input.size(),
            false);
    }
}

Frame::operator zmq_msg_t*() { return &message_; }

Frame::operator std::string() const
{
    std::string output(static_cast<const char*>(data()), size());

    return output;
}

Frame* Frame::clone() const { return new Frame(std::string(*this)); }

const void* Frame::data() const { return zmq_msg_data(&message_); }

std::size_t Frame::size() const { return zmq_msg_size(&message_); }

Frame::~Frame() { zmq_msg_close(&message_); }
}  // namespace opentxs::network::zeromq::implementation
