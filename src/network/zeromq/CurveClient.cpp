// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "CurveClient.hpp"

#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include "Socket.hpp"

#include <zmq.h>

#include <array>

#define OT_METHOD "opentxs::network::zeromq::implementation::CurveClient::"

namespace opentxs::network::zeromq::implementation
{
CurveClient::CurveClient(std::mutex& lock, void* socket)
    : curve_lock_(lock)
    , curve_socket_(socket)
{
}

bool CurveClient::set_curve(const ServerContract& contract) const
{
    Lock lock(curve_lock_);

    if (false == set_remote_key(lock, contract)) { return false; }

    return set_local_keys(lock);
}

bool CurveClient::set_local_keys(const Lock&) const
{
    OT_ASSERT(nullptr != curve_socket_);

    std::array<char, CURVE_KEY_Z85_BYTES + 1> publicKey{};
    std::array<char, CURVE_KEY_Z85_BYTES + 1> secretKey{};
    auto* pubkey = &publicKey[0];
    auto* privkey = &secretKey[0];
    auto set = zmq_curve_keypair(pubkey, privkey);

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to generate keypair."
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        curve_socket_, ZMQ_CURVE_PUBLICKEY, pubkey, publicKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set public key."
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        curve_socket_, ZMQ_CURVE_SECRETKEY, privkey, secretKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

bool CurveClient::set_remote_key(const Lock&, const ServerContract& contract)
    const
{
    OT_ASSERT(nullptr != curve_socket_);

    const auto& key = contract.TransportKey();

    if (CURVE_KEY_BYTES != key.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server key."
              << std::endl;

        return false;
    }

    const auto set = zmq_setsockopt(
        curve_socket_, ZMQ_CURVE_SERVERKEY, key.data(), key.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set server key."
              << std::endl;

        return false;
    }

    return true;
}

CurveClient::~CurveClient() { curve_socket_ = nullptr; }
}  // namespace opentxs::network::zeromq::implementation
