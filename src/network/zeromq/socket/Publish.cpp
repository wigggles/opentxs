// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/curve/Server.hpp"
#include "Sender.tpp"

#include "Publish.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Publish>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::Publish::"

namespace opentxs
{
network::zeromq::socket::Publish* Factory::PublishSocket(
    const opentxs::network::zeromq::Context& context)
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
