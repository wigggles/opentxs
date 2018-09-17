// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PushSocket.hpp"

#include "CurveClient.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class PushSocket : virtual public zeromq::PushSocket, public Socket, CurveClient
{
public:
    bool Push(const std::string& data) const override;
    bool Push(const opentxs::Data& data) const override;
    bool Push(zeromq::Message& data) const override;
    bool Start(const std::string& endpoint) const override;

    ~PushSocket() = default;

private:
    friend opentxs::network::zeromq::PushSocket;
    typedef Socket ot_super;

    PushSocket* clone() const override;

    PushSocket(
        const zeromq::Context& context,
        const Socket::Direction direction);
    PushSocket() = delete;
    PushSocket(const PushSocket&) = delete;
    PushSocket(PushSocket&&) = delete;
    PushSocket& operator=(const PushSocket&) = delete;
    PushSocket& operator=(PushSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
