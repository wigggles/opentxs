// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <string>

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Subscribe;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Subscribe : public Receiver<zeromq::socket::Subscribe>,
                  public zeromq::curve::implementation::Client
{
public:
    bool SetSocksProxy(const std::string& proxy) const noexcept final;

    ~Subscribe() override;

protected:
    const ListenCallback& callback_;

    Subscribe(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback) noexcept;

private:
    friend opentxs::Factory;

    Subscribe* clone() const noexcept override;
    bool have_callback() const noexcept final;

    void init() noexcept final;
    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Subscribe() = delete;
    Subscribe(const Subscribe&) = delete;
    Subscribe(Subscribe&&) = delete;
    Subscribe& operator=(const Subscribe&) = delete;
    Subscribe& operator=(Subscribe&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
