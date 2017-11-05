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

#include "opentxs/api/OT.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/ZMQ.hpp"

#define CLIENT_SEND_TIMEOUT_SECONDS 20
#define CLIENT_RECV_TIMEOUT_SECONDS 40
#define CLIENT_SOCKET_LINGER_SECONDS 10
#define CLIENT_SEND_TIMEOUT CLIENT_SEND_TIMEOUT_SECONDS
#define CLIENT_RECV_TIMEOUT CLIENT_RECV_TIMEOUT_SECONDS
#define KEEP_ALIVE_SECONDS 30

#define OT_METHOD "opentxs::api::ZMQ::"

namespace opentxs::api
{

ZMQ::ZMQ(api::Settings& config)
    : config_(config)
    , linger_(std::chrono::seconds(CLIENT_SOCKET_LINGER_SECONDS))
    , receive_timeout_(std::chrono::seconds(CLIENT_RECV_TIMEOUT))
    , send_timeout_(std::chrono::seconds(CLIENT_SEND_TIMEOUT))
    , keep_alive_(std::chrono::seconds(0))
    , shutdown_(false)
    , lock_()
    , socks_proxy_()
    , server_connections_()
{
    Lock lock(lock_);

    init(lock);
}

void ZMQ::init(const Lock& lock)
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

std::chrono::seconds ZMQ::Linger() { return linger_.load(); }

std::chrono::seconds ZMQ::ReceiveTimeout() { return receive_timeout_.load(); }

void ZMQ::RefreshConfig()
{
    Lock lock(lock_);

    return init(lock);
}

std::chrono::seconds ZMQ::SendTimeout() { return send_timeout_.load(); }

ServerConnection& ZMQ::Server(const std::string& id)
{
    Lock lock(lock_);
    auto& connection = server_connections_[id];

    if (!connection) {
        connection.reset(new ServerConnection(
            id, socks_proxy_, shutdown_, keep_alive_, *this, config_));
    }

    OT_ASSERT(connection);

    return *connection;
}

bool ZMQ::SetSocksProxy(const std::string& proxy)
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
        OT_ASSERT(it.second);

        auto& connection = *it.second;

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

bool ZMQ::SocksProxy(std::string& proxy)
{
    Lock lock(lock_);
    proxy = socks_proxy_;

    return (!socks_proxy_.empty());
}

std::string ZMQ::SocksProxy()
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

ZMQ::~ZMQ()
{
    shutdown_.store(true);
    server_connections_.clear();
}
}  // namespace opentxs::api
