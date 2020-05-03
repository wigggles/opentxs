// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: private
// IWYU pragma: friend ".*src/network/zeromq/socket/Publish.cpp"

#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Publish final : public Sender<zeromq::socket::Publish>,
                      public zeromq::curve::implementation::Server
{
public:
    ~Publish();

private:
    friend opentxs::Factory;

    Publish* clone() const noexcept final { return new Publish(context_); }

    Publish(const zeromq::Context& context) noexcept;
    Publish() = delete;
    Publish(const Publish&) = delete;
    Publish(Publish&&) = delete;
    Publish& operator=(const Publish&) = delete;
    Publish& operator=(Publish&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
