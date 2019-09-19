// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs::network::zeromq::socket::implementation
{
class Dealer final : public Bidirectional<zeromq::socket::Dealer>,
                     public zeromq::curve::implementation::Client
{
public:
    bool SetSocksProxy(const std::string& proxy) const noexcept final
    {
        return set_socks_proxy(proxy);
    }

    virtual ~Dealer();

protected:
    const ListenCallback& callback_;

    Dealer(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback) noexcept;

private:
    friend opentxs::Factory;

    Dealer* clone() const noexcept final;
    bool have_callback() const noexcept final { return true; }

    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Dealer() = delete;
    Dealer(const Dealer&) = delete;
    Dealer(Dealer&&) = delete;
    Dealer& operator=(const Dealer&) = delete;
    Dealer& operator=(Dealer&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
