// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/socket/Router.cpp"

#pragma once

#include <map>
#include <ostream>
#include <string>

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Bidirectional.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Router;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Router final : public Bidirectional<zeromq::socket::Router>,
                     public zeromq::curve::implementation::Client,
                     public zeromq::curve::implementation::Server
{
public:
    bool SetSocksProxy(const std::string& proxy) const noexcept final
    {
        return set_socks_proxy(proxy);
    }

    virtual ~Router();

protected:
    const ListenCallback& callback_;

    Router(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback) noexcept;

private:
    friend opentxs::Factory;

    Router* clone() const noexcept final;
    bool have_callback() const noexcept final { return true; }

    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Router() = delete;
    Router(const Router&) = delete;
    Router(Router&&) = delete;
    Router& operator=(const Router&) = delete;
    Router& operator=(Router&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
