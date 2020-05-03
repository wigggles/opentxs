// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/socket/Router.cpp"

#pragma once

#include <map>
#include <ostream>

#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Reply;
}  // namespace socket

class Context;
class Message;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Reply final : public Receiver<zeromq::socket::Reply>,
                    public zeromq::curve::implementation::Server
{
public:
    ~Reply() final;

private:
    friend opentxs::Factory;

    const ReplyCallback& callback_;

    Reply* clone() const noexcept final;
    bool have_callback() const noexcept final;

    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Reply(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const ReplyCallback& callback) noexcept;
    Reply() = delete;
    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
