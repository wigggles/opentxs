// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Frame.hpp"

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <zmq.h>

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
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Frame::Frame()
    : message_(new zmq_msg_t)
{
    OT_ASSERT(nullptr != message_);

    const auto init = zmq_msg_init(message_);

    OT_ASSERT(0 == init);
}

Frame::Frame(const Data& input)
    : message_(new zmq_msg_t)
{
    OT_ASSERT(nullptr != message_);

    const auto init = zmq_msg_init_size(message_, input.size());

    if (0 < input.size()) {
        OTPassword::safe_memcpy(
            zmq_msg_data(message_),
            zmq_msg_size(message_),
            input.data(),
            input.size(),
            false);
    }

    OT_ASSERT(0 == init);
}

Frame::Frame(const std::string& input)
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

Frame::operator zmq_msg_t*() { return message_; }

Frame::operator std::string() const
{
    std::string output(static_cast<const char*>(data()), size());

    return output;
}

Frame* Frame::clone() const { return new Frame(std::string(*this)); }

const void* Frame::data() const
{
    OT_ASSERT(nullptr != message_);

    return zmq_msg_data(message_);
}

std::size_t Frame::size() const
{
    OT_ASSERT(nullptr != message_);

    return zmq_msg_size(message_);
}

Frame::~Frame()
{
    if (nullptr != message_) { zmq_msg_close(message_); }
}
}  // namespace opentxs::network::zeromq::implementation
