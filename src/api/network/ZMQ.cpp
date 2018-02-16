/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/stdafx.hpp"

#include "ZMQ.hpp"

#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/implementation/Context.hpp"
#include "opentxs/network/ServerConnection.hpp"

#define CLIENT_SEND_TIMEOUT_SECONDS 20
#define CLIENT_RECV_TIMEOUT_SECONDS 40
#define CLIENT_SOCKET_LINGER_SECONDS 10
#define CLIENT_SEND_TIMEOUT CLIENT_SEND_TIMEOUT_SECONDS
#define CLIENT_RECV_TIMEOUT CLIENT_RECV_TIMEOUT_SECONDS
#define KEEP_ALIVE_SECONDS 30

#define OT_METHOD "opentxs::api::ZMQ::"

namespace opentxs::api::network::implementation
{

ZMQ::ZMQ(
    const opentxs::network::zeromq::Context& context,
    const api::Settings& config,
    const Flag& running)
    : context_(context)
    , config_(config)
    , running_(running)
    , linger_(std::chrono::seconds(CLIENT_SOCKET_LINGER_SECONDS))
    , receive_timeout_(std::chrono::seconds(CLIENT_RECV_TIMEOUT))
    , send_timeout_(std::chrono::seconds(CLIENT_SEND_TIMEOUT))
    , keep_alive_(std::chrono::seconds(0))
    , lock_()
    , socks_proxy_()
    , server_connections_()
{
    Lock lock(lock_);

    init(lock);
}

const opentxs::network::zeromq::Context& ZMQ::Context() const
{
    return context_;
}

proto::AddressType ZMQ::DefaultAddressType() const
{
    bool changed{false};
    const std::int64_t defaultType{
        static_cast<std::int64_t>(proto::ADDRESSTYPE_IPV4)};
    std::int64_t configuredType{
        static_cast<std::int64_t>(proto::ADDRESSTYPE_ERROR)};
    config_.CheckSet_long(
        "Connection",
        "preferred_address_type",
        defaultType,
        configuredType,
        changed);

    if (changed) {
        config_.Save();
    }

    return static_cast<proto::AddressType>(configuredType);
}

void ZMQ::init(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock));

    bool notUsed{false};
    std::int64_t linger{0};
    config_.CheckSet_long(
        "latency", "linger", CLIENT_SOCKET_LINGER_SECONDS, linger, notUsed);
    linger_.store(std::chrono::seconds(linger));
    std::int64_t send{0};
    config_.CheckSet_long(
        "latency", "send_timeout", CLIENT_SEND_TIMEOUT, send, notUsed);
    send_timeout_.store(std::chrono::seconds(send));
    std::int64_t receive{0};
    config_.CheckSet_long(
        "latency", "recv_timeout", CLIENT_RECV_TIMEOUT, receive, notUsed);
    receive_timeout_.store(std::chrono::seconds(receive));
    String socks{};
    bool haveSocksConfig{false};
    const bool configChecked =
        config_.Check_str("Connection", "socks_proxy", socks, haveSocksConfig);
    std::int64_t keepAlive{0};
    config_.CheckSet_long(
        "Connection", "keep_alive", KEEP_ALIVE_SECONDS, keepAlive, notUsed);
    keep_alive_.store(std::chrono::seconds(keepAlive));

    if (configChecked && haveSocksConfig && socks.Exists()) {
        socks_proxy_ = socks.Get();
    }

    config_.Save();
}

std::chrono::seconds ZMQ::KeepAlive() const { return keep_alive_.load(); }

void ZMQ::KeepAlive(const std::chrono::seconds duration) const
{
    keep_alive_.store(duration);
}

std::chrono::seconds ZMQ::Linger() const { return linger_.load(); }

std::shared_ptr<opentxs::network::zeromq::Context> ZMQ::NewContext() const
{
    std::shared_ptr<opentxs::network::zeromq::Context> output{nullptr};
    output.reset(new opentxs::network::zeromq::implementation::Context());

    return output;
}

std::chrono::seconds ZMQ::ReceiveTimeout() const
{
    return receive_timeout_.load();
}

void ZMQ::RefreshConfig() const
{
    Lock lock(lock_);

    return init(lock);
}

const Flag& ZMQ::Running() const { return running_; }

std::chrono::seconds ZMQ::SendTimeout() const { return send_timeout_.load(); }

opentxs::network::ServerConnection& ZMQ::Server(const std::string& id) const
{
    Lock lock(lock_);
    auto existing = server_connections_.find(id);

    if (server_connections_.end() != existing) {

        return existing->second;
    }

    auto[it, created] = server_connections_.emplace(
        id, opentxs::network::ServerConnection::Factory(*this, id));

    OT_ASSERT(created);

    return it->second;
}

bool ZMQ::SetSocksProxy(const std::string& proxy) const
{
    bool notUsed{false};
    bool set =
        config_.Set_str("Connection", "socks_proxy", String{proxy}, notUsed);

    if (false == set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to set socks proxy."
              << std::endl;

        return false;
    }

    if (false == config_.Save()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to set save config."
              << std::endl;

        return false;
    }

    Lock lock(lock_);
    socks_proxy_ = proxy;

    for (auto& it : server_connections_) {
        opentxs::network::ServerConnection& connection = it.second;

        if (proxy.empty()) {
            set &= connection.ClearProxy();
        } else {
            set &= connection.EnableProxy();
        }
    }

    if (false == set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Unable to reset connection."
              << std::endl;
    }

    return set;
}

bool ZMQ::SocksProxy(std::string& proxy) const
{
    Lock lock(lock_);
    proxy = socks_proxy_;

    return (!socks_proxy_.empty());
}

std::string ZMQ::SocksProxy() const
{
    std::string output{};
    SocksProxy(output);

    return output;
}

ConnectionState ZMQ::Status(const std::string& server) const
{
    Lock lock(lock_);
    const auto it = server_connections_.find(server);
    const bool haveConnection = it != server_connections_.end();
    lock.unlock();

    if (haveConnection) {
        if (it->second->Status()) {

            return ConnectionState::ACTIVE;
        } else {

            return ConnectionState::STALLED;
        }
    }

    return ConnectionState::NOT_ESTABLISHED;
}

bool ZMQ::verify_lock(const Lock& lock) const
{
    if (lock.mutex() != &lock_) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mutex." << std::endl;

        return false;
    }

    if (false == lock.owns_lock()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Lock not owned." << std::endl;

        return false;
    }

    return true;
}

ZMQ::~ZMQ() { server_connections_.clear(); }
}  // namespace opentxs::api::network::implementation
