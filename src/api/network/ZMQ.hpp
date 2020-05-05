// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/network/ZMQ.cpp"

#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/protobuf/ContractEnums.pb.h"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class Factory;
class Flag;
}  // namespace opentxs

namespace opentxs::api::network::implementation
{
class ZMQ final : virtual public opentxs::api::network::ZMQ
{
public:
    const opentxs::network::zeromq::Context& Context() const final;
    proto::AddressType DefaultAddressType() const final;
    std::chrono::seconds KeepAlive() const final;
    void KeepAlive(const std::chrono::seconds duration) const final;
    std::chrono::seconds Linger() const final;
    std::chrono::seconds ReceiveTimeout() const final;
    void RefreshConfig() const final;
    const Flag& Running() const final;
    std::chrono::seconds SendTimeout() const final;

    opentxs::network::ServerConnection& Server(
        const std::string& id) const final;
    bool SetSocksProxy(const std::string& proxy) const final;
    std::string SocksProxy() const final;
    bool SocksProxy(std::string& proxy) const final;
    ConnectionState Status(const std::string& server) const final;

    ~ZMQ() final;

private:
    friend opentxs::Factory;

    const api::internal::Core& api_;
    const Flag& running_;
    mutable std::atomic<std::chrono::seconds> linger_;
    mutable std::atomic<std::chrono::seconds> receive_timeout_;
    mutable std::atomic<std::chrono::seconds> send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;
    mutable std::mutex lock_;
    mutable std::string socks_proxy_;
    mutable std::map<std::string, OTServerConnection> server_connections_;
    OTZMQPublishSocket status_publisher_;

    bool verify_lock(const Lock& lock) const;

    void init(const Lock& lock) const;

    ZMQ(const api::internal::Core& api, const Flag& running);
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    ZMQ& operator=(const ZMQ&) = delete;
    ZMQ& operator=(const ZMQ&&) = delete;
};
}  // namespace opentxs::api::network::implementation
