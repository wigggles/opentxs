// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/curve/Server.hpp"

#include "network/zeromq/socket/Socket.hpp"

#include <mutex>

namespace opentxs::network::zeromq::curve::implementation
{
class Server : virtual public zeromq::curve::Server
{
public:
    bool SetDomain(const std::string& domain) const noexcept final;
    bool SetPrivateKey(const OTPassword& key) const noexcept final;
    bool SetPrivateKey(const std::string& z85) const noexcept final;

protected:
    bool set_private_key(const void* key, const std::size_t keySize) const
        noexcept;

    Server(zeromq::socket::implementation::Socket& socket) noexcept;

    ~Server() override = default;

private:
    zeromq::socket::implementation::Socket& parent_;

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace opentxs::network::zeromq::curve::implementation
