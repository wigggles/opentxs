// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::socket::implementation
{
class Router final : public Bidirectional<zeromq::socket::Router>,
                     public zeromq::curve::implementation::Client,
                     public zeromq::curve::implementation::Server
{
public:
    bool SetSocksProxy(const std::string& proxy) const noexcept final
    {
        return set_socks_proxy(proxy);
    }

    virtual ~Router();

protected:
    const ListenCallback& callback_;

    Router(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback) noexcept;

private:
    friend opentxs::Factory;

    Router* clone() const noexcept final;
    bool have_callback() const noexcept final { return true; }

    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Router() = delete;
    Router(const Router&) = delete;
    Router(Router&&) = delete;
    Router& operator=(const Router&) = delete;
    Router& operator=(Router&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
