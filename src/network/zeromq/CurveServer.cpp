// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "CurveServer.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Log.hpp"

#include "Socket.hpp"

#include <zmq.h>

#include <array>

#define OT_METHOD "opentxs::network::zeromq::implementation::CurveServer::"

namespace opentxs::network::zeromq::implementation
{
CurveServer::CurveServer(std::mutex& lock, void* socket)
    : server_curve_lock_(lock)
    , server_curve_socket_(socket)
{
}

bool CurveServer::SetDomain(const std::string& domain) const
{
    auto set = zmq_setsockopt(
        server_curve_socket_, ZMQ_ZAP_DOMAIN, domain.data(), domain.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set domain."
              << std::endl;

        return false;
    }

    return true;
}

bool CurveServer::SetPrivateKey(const OTPassword& key) const
{
    if (CURVE_KEY_BYTES != key.getMemorySize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid private key."
              << std::endl;

        return false;
    }

    return set_private_key(key.getMemory(), key.getMemorySize());
}

bool CurveServer::SetPrivateKey(const std::string& z85) const
{
    if (CURVE_KEY_Z85_BYTES > z85.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid private key size ("
              << z85.size() << ")" << std::endl;

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> key;
    ::zmq_z85_decode(key.data(), z85.data());

    return set_private_key(key.data(), key.size());
}

bool CurveServer::set_private_key(const void* key, const std::size_t keySize)
    const
{
    OT_ASSERT(nullptr != server_curve_socket_);

    Lock lock(server_curve_lock_);

    const int server{1};
    auto set = zmq_setsockopt(
        server_curve_socket_, ZMQ_CURVE_SERVER, &server, sizeof(server));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_CURVE_SERVER"
              << std::endl;

        return false;
    }

    set =
        zmq_setsockopt(server_curve_socket_, ZMQ_CURVE_SECRETKEY, key, keySize);

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

CurveServer::~CurveServer() { server_curve_socket_ = nullptr; }
}  // namespace opentxs::network::zeromq::implementation
