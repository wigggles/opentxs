// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/RouterSocket.hpp"

#include "CurveClient.hpp"
#include "CurveServer.hpp"
#include "Bidirectional.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class RouterSocket : virtual public zeromq::RouterSocket,
                     public Socket,
                     CurveClient,
                     CurveServer,
                     Bidirectional
{
public:
    bool Send(opentxs::Data& message) const override;
    bool Send(const std::string& message) const override;
    bool Send(zeromq::Message& message) const override;
    bool SetSocksProxy(const std::string& proxy) const override;
    bool Start(const std::string& endpoint) const override;

    virtual ~RouterSocket();

protected:
    const ListenCallback& callback_;

    RouterSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::RouterSocket;
    typedef Socket ot_super;

    RouterSocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    RouterSocket() = delete;
    RouterSocket(const RouterSocket&) = delete;
    RouterSocket(RouterSocket&&) = delete;
    RouterSocket& operator=(const RouterSocket&) = delete;
    RouterSocket& operator=(RouterSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
