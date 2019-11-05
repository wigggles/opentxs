// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_TPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SEND_TPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
template <typename Input>
OPENTXS_EXPORT bool Sender::Send(const Input& data) const noexcept
{
    return send(Context().Message(data));
}
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
