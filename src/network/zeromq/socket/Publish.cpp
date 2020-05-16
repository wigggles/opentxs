// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/socket/Publish.hpp"  // IWYU pragma: associated

#include "Factory.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Sender.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

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
}  // namespace opentxs

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Publish>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::Publish::"

namespace opentxs
{
auto Factory::PublishSocket(const opentxs::network::zeromq::Context& context)
    -> network::zeromq::socket::Publish*
{
    using ReturnType = network::zeromq::socket::implementation::Publish;

    return new ReturnType(context);
}
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
Publish::Publish(const zeromq::Context& context) noexcept
    : Socket(context, SocketType::Publish, Socket::Direction::Bind)
    , Sender()
    , Server(this->get())
{
    init();
}

Publish::~Publish() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
