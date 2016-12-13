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

#ifndef OPENTXS_NETWORK_ZMQ_HPP
#define OPENTXS_NETWORK_ZMQ_HPP

// IWYU pragma: begin_exports
extern "C" {
    #ifndef __STDC_VERSION__
    #define __STDC_VERSION__ 0
    #endif
    #ifndef _ZMALLOC_PEDANTIC
    #define _ZMALLOC_PEDANTIC 0
    #endif
    #include <czmq.h>
}
// IWYU pragma: end_exports

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <string>

// forward declare czmq types
typedef struct _zsock_t zsock_t;
typedef struct _zactor_t zactor_t;
typedef struct _zpoller_t zpoller_t;

namespace opentxs
{

class App;
class ServerConnection;
class Settings;

class ZMQ
{
private:
    friend class App;

    Settings& config_;

    std::chrono::seconds linger_;
    std::chrono::seconds receive_timeout_;
    std::chrono::seconds send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;

    std::string socks_proxy_;

    std::map<std::string,std::unique_ptr<ServerConnection>> server_connections_;

    void Init();

    ZMQ() = delete;
    ZMQ(Settings& config);
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    ZMQ& operator=(const ZMQ&) = delete;
    ZMQ& operator=(const ZMQ&&) = delete;

public:
    std::chrono::seconds KeepAlive() const;
    void KeepAlive(const std::chrono::seconds duration) const;
    std::chrono::seconds Linger();
    std::chrono::seconds ReceiveTimeout();
    std::chrono::seconds SendTimeout();

    bool SocksProxy(std::string& proxy);

    ServerConnection& Server(const std::string& id);

    ~ZMQ() = default;
};
}  // namespace opentxs
#endif // OPENTXS_NETWORK_ZMQ_HPP
