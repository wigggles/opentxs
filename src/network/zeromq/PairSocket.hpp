// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PairSocket.hpp"

#include "Bidirectional.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class PairSocket final : virtual public zeromq::PairSocket,
                         public Socket,
                         Bidirectional
{
public:
    const std::string& Endpoint() const override;
    bool Send(const std::string& data) const override;
    bool Send(const opentxs::Data& data) const override;
    bool Send(network::zeromq::Message& data) const override;
    bool Start(const std::string& endpoint) const override;

    ~PairSocket();

private:
    friend opentxs::network::zeromq::PairSocket;
    friend Proxy;
    typedef Socket ot_super;

    const ListenCallback& callback_;
    const std::string endpoint_;
    const bool bind_{false};

    PairSocket* clone() const override;
    bool have_callback() const override;
    void process_incoming(const Lock& lock, Message& message) override;

    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint,
        const bool bind,
        const bool startThread);
    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const bool startThread = true);
    PairSocket(
        const zeromq::ListenCallback& callback,
        const zeromq::PairSocket& peer,
        const bool startThread = true);
    PairSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const std::string& endpoint);
    PairSocket() = delete;
    PairSocket(const PairSocket&) = delete;
    PairSocket(PairSocket&&) = delete;
    PairSocket& operator=(const PairSocket&) = delete;
    PairSocket& operator=(PairSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
