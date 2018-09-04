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
    : client_curve_lock_(lock)
    , client_curve_socket_(socket)
{
}

bool CurveClient::SetServerPubkey(const ServerContract& contract) const
{
    return set_public_key(contract);
}

bool CurveClient::SetServerPubkey(const Data& key) const
{
    return set_public_key(key);
}

bool CurveClient::set_public_key(const ServerContract& contract) const
{
    Lock lock(client_curve_lock_);

    const auto& key = contract.TransportKey();

    if (CURVE_KEY_BYTES != key.GetSize()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid server key."
              << std::endl;

        return false;
    }

    return set_public_key(key);
}

bool CurveClient::set_public_key(const Data& key) const
{
    if (false == set_remote_key(key)) { return false; }

    return set_local_keys();
}

bool CurveClient::set_local_keys() const
{
    OT_ASSERT(nullptr != client_curve_socket_);

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
        client_curve_socket_, ZMQ_CURVE_PUBLICKEY, pubkey, publicKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set public key."
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        client_curve_socket_, ZMQ_CURVE_SECRETKEY, privkey, secretKey.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set private key."
              << std::endl;

        return false;
    }

    return true;
}

bool CurveClient::set_remote_key(const Data& key) const
{
    OT_ASSERT(nullptr != client_curve_socket_);

    const auto set = zmq_setsockopt(
        client_curve_socket_, ZMQ_CURVE_SERVERKEY, key.data(), key.size());

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set server key."
              << std::endl;

        return false;
    }

    return true;
}

CurveClient::~CurveClient() { client_curve_socket_ = nullptr; }
}  // namespace opentxs::network::zeromq::implementation
