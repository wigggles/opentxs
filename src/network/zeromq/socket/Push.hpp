// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/socket/Push.cpp"

#pragma once

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Sender.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Push final : public Sender<zeromq::socket::Push>,
                   public zeromq::curve::implementation::Client
{
public:
    ~Push();

private:
    friend opentxs::Factory;

    Push* clone() const noexcept final
    {
        return new Push(context_, direction_);
    }

    Push(
        const zeromq::Context& context,
        const Socket::Direction direction) noexcept;
    Push() = delete;
    Push(const Push&) = delete;
    Push(Push&&) = delete;
    Push& operator=(const Push&) = delete;
    Push& operator=(Push&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
