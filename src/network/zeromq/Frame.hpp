// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_FRAME_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_FRAME_HPP

#include "Internal.hpp"

#include "opentxs/network/zeromq/Frame.hpp"

namespace opentxs::network::zeromq::implementation
{
class Frame : virtual public zeromq::Frame
{
public:
    operator std::string() const override;

    const void* data() const override;
    std::size_t size() const override;

    operator zmq_msg_t*() override;

    ~Frame();

private:
    friend network::zeromq::Frame;

    zmq_msg_t* message_{nullptr};

    Frame* clone() const override;

    Frame();
    explicit Frame(const Data& input);
    explicit Frame(const std::string& input);
    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
    Frame& operator=(Frame&&) = delete;
    Frame& operator=(const Frame&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_FRAME_HPP
