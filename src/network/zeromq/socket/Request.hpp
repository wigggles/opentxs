// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/socket/Request.cpp"

#pragma once

#include <string>

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Request final : virtual public zeromq::socket::Request,
                      public Socket,
                      public zeromq::curve::implementation::Client
{
public:
    bool SetSocksProxy(const std::string& proxy) const noexcept final;

    ~Request() final;

private:
    friend opentxs::Factory;

    Request* clone() const noexcept final;
    SendResult send_request(zeromq::Message& message) const noexcept final;
    bool wait(const Lock& lock) const noexcept;

    Request(const zeromq::Context& context) noexcept;
    Request() = delete;
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
