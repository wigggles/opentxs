// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/Frame.hpp"

#include <zmq.h>

namespace opentxs::network::zeromq::implementation
{
class Frame final : virtual public zeromq::Frame
{
public:
    operator std::string() const noexcept final;

    ReadView Bytes() const noexcept final;
    const void* data() const noexcept final { return zmq_msg_data(&message_); }
    std::size_t size() const noexcept final { return zmq_msg_size(&message_); }

    operator zmq_msg_t*() noexcept final { return &message_; }

    ~Frame() final;

private:
    friend opentxs::Factory;
    friend network::zeromq::Frame;

    mutable zmq_msg_t message_;

    Frame* clone() const noexcept final;

    Frame() noexcept;
    explicit Frame(const ProtobufType& input) noexcept;
    explicit Frame(const std::size_t bytes) noexcept;
    Frame(const void* data, const std::size_t bytes) noexcept;
    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
    Frame& operator=(Frame&&) = delete;
    Frame& operator=(const Frame&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
