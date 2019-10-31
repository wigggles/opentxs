// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/Types.hpp"

#include "network/zeromq/socket/Socket.hpp"

#include <mutex>

namespace opentxs::network::zeromq::curve::implementation
{
class Client : virtual public zeromq::curve::Client
{
public:
    bool SetKeysZ85(
        const std::string& serverPublic,
        const std::string& clientPrivate,
        const std::string& clientPublic) const noexcept final;
    bool SetServerPubkey(const contract::Server& contract) const noexcept final;
    bool SetServerPubkey(const Data& key) const noexcept final;

protected:
    bool set_public_key(const contract::Server& contract) const noexcept;
    bool set_public_key(const Data& key) const noexcept;

    Client(zeromq::socket::implementation::Socket& socket) noexcept;

    ~Client() override = default;

private:
    zeromq::socket::implementation::Socket& parent_;

    bool set_local_keys() const noexcept;
    bool set_local_keys(
        const std::string& privateKey,
        const std::string& publicKey) const noexcept;
    bool set_local_keys(
        const void* privateKey,
        const std::size_t privateKeySize,
        const void* publicKey,
        const std::size_t publicKeySize) const noexcept;
    bool set_remote_key(const void* key, const std::size_t size) const noexcept;

    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;
};
}  // namespace opentxs::network::zeromq::curve::implementation
