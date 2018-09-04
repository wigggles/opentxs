// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "CurveClient.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class SubscribeSocket : virtual public zeromq::SubscribeSocket,
                        public Socket,
                        CurveClient,
                        Receiver<zeromq::Message>
{
public:
    bool SetSocksProxy(const std::string& proxy) const override;
    bool Start(const std::string& endpoint) const override;

    virtual ~SubscribeSocket();

protected:
    const ListenCallback& callback_;

    SubscribeSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::SubscribeSocket;
    typedef Socket ot_super;

    SubscribeSocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    SubscribeSocket() = delete;
    SubscribeSocket(const SubscribeSocket&) = delete;
    SubscribeSocket(SubscribeSocket&&) = delete;
    SubscribeSocket& operator=(const SubscribeSocket&) = delete;
    SubscribeSocket& operator=(SubscribeSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
