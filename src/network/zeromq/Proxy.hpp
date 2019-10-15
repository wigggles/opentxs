// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/Proxy.hpp"

#include <memory>
#include <thread>

namespace opentxs::network::zeromq::implementation
{
class Proxy final : virtual public opentxs::network::zeromq::Proxy
{
public:
    ~Proxy() final;

private:
    friend opentxs::network::zeromq::Proxy;

    const zeromq::Context& context_;
    zeromq::socket::Socket& frontend_;
    zeromq::socket::Socket& backend_;
    OTZMQListenCallback null_callback_;
    OTZMQPairSocket control_listener_;
    OTZMQPairSocket control_sender_;
    std::unique_ptr<std::thread> thread_{nullptr};

    Proxy* clone() const final;
    void proxy() const;

    Proxy(
        const zeromq::Context& context,
        zeromq::socket::Socket& frontend,
        zeromq::socket::Socket& backend);
    Proxy() = delete;
    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = delete;
    Proxy& operator=(const Proxy&) = delete;
    Proxy& operator=(Proxy&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
