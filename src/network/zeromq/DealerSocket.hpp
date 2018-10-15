// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/DealerSocket.hpp"

#include "CurveClient.hpp"
#include "Bidirectional.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class DealerSocket final : virtual public zeromq::DealerSocket,
                           public Socket,
                           CurveClient,
                           Bidirectional
{
public:
    bool Send(opentxs::Data& message) const override;
    bool Send(const std::string& message) const override;
    bool Send(zeromq::Message& message) const override;
    bool SetSocksProxy(const std::string& proxy) const override;
    bool Start(const std::string& endpoint) const override;

    virtual ~DealerSocket();

protected:
    const ListenCallback& callback_;

    DealerSocket(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::DealerSocket;
    typedef Socket ot_super;

    DealerSocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    DealerSocket() = delete;
    DealerSocket(const DealerSocket&) = delete;
    DealerSocket(DealerSocket&&) = delete;
    DealerSocket& operator=(const DealerSocket&) = delete;
    DealerSocket& operator=(DealerSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
