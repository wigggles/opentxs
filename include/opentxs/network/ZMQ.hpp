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

#include "opentxs/core/Types.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>

// forward declare czmq types
typedef struct _zsock_t zsock_t;
typedef struct _zactor_t zactor_t;
typedef struct _zpoller_t zpoller_t;

namespace opentxs
{

class OT;
class ServerConnection;
class Settings;
class Wallet;

class ZMQ
{
private:
    friend class OT;

    Settings& config_;
    std::atomic<std::chrono::seconds> linger_;
    std::atomic<std::chrono::seconds> receive_timeout_;
    std::atomic<std::chrono::seconds> send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;
    mutable std::atomic<bool> shutdown_;
    mutable std::mutex lock_;
    std::string socks_proxy_;
    std::map<std::string, std::unique_ptr<ServerConnection>>
        server_connections_;

    bool verify_lock(const Lock& lock) const;

    void init(const Lock& lock);

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
    void RefreshConfig();
    std::chrono::seconds SendTimeout();

    bool SetSocksProxy(const std::string& proxy);
    std::string SocksProxy();
    bool SocksProxy(std::string& proxy);

    ServerConnection& Server(const std::string& id);
    ConnectionState Status(const std::string& server) const;

    ~ZMQ();
};
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZMQ_HPP
