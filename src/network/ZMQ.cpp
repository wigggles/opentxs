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

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Settings.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/ServerConnection.hpp"
#include "opentxs/network/ZMQ.hpp"

#define CLIENT_SOCKET_LINGER 1000
#define CLIENT_SEND_TIMEOUT 1000
#define CLIENT_RECV_TIMEOUT 10000

namespace opentxs
{

ZMQ::ZMQ(Settings& config)
    : config_(config)
    , linger_(CLIENT_SOCKET_LINGER)
    , receive_timeout_(CLIENT_RECV_TIMEOUT)
    , send_timeout_(CLIENT_SEND_TIMEOUT)
{
    Init();
    config_.Save();
}

void ZMQ::Init()
{
    bool notUsed = false;

    int64_t linger;
    config_.CheckSet_long(
        "latency",
        "linger",
        CLIENT_SOCKET_LINGER,
        linger,
        notUsed);
    linger_ = std::chrono::seconds(linger);

    int64_t send;
    config_.CheckSet_long(
        "latency",
        "send_timeout",
        CLIENT_SEND_TIMEOUT,
        send,
        notUsed);
    send_timeout_ = std::chrono::seconds(send);

    int64_t receive;
    config_.CheckSet_long(
        "latency",
        "recv_timeout",
        CLIENT_RECV_TIMEOUT,
        receive,
        notUsed);
    receive_timeout_ = std::chrono::seconds(receive);

    String socks;
    bool haveSocksConfig = false;
    const bool configChecked = config_.Check_str(
        "Connection",
        "socks_proxy",
        socks,
        haveSocksConfig);

    if (configChecked && haveSocksConfig && socks.Exists()) {
        socks_proxy_ = socks.Get();
    }
}

std::chrono::seconds ZMQ::Linger()
{
    Init();

    return linger_;
}

std::chrono::seconds ZMQ::ReceiveTimeout()
{
    Init();

    return receive_timeout_;
}

std::chrono::seconds ZMQ::SendTimeout()
{
    Init();

    return send_timeout_;
}

ServerConnection& ZMQ::Server(const std::string& id)
{
    auto& connection = server_connections_[id];

    if (!connection) {
        connection.reset(new ServerConnection(id));
    }

    OT_ASSERT(connection);

    return *connection;
}

bool ZMQ::SocksProxy(std::string& proxy)
{
    Init();
    proxy = socks_proxy_;

    return (!socks_proxy_.empty());
}
}  // namespace opentxs
