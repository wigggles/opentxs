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

#ifndef OPENTXS_API_NETWORK_IMPLEMENTATION_ZMQ_HPP
#define OPENTXS_API_NETWORK_IMPLEMENTATION_ZMQ_HPP

#include "opentxs/Internal.hpp"

#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/core/Flag.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

namespace opentxs::api::network::implementation
{
class ZMQ : virtual public opentxs::api::network::ZMQ
{
public:
    const opentxs::network::zeromq::Context& Context() const override;
    proto::AddressType DefaultAddressType() const override;
    std::chrono::seconds KeepAlive() const override;
    void KeepAlive(const std::chrono::seconds duration) const override;
    std::chrono::seconds Linger() const override;
    OTZMQContext NewContext() const override;
    std::chrono::seconds ReceiveTimeout() const override;
    void RefreshConfig() const override;
    const Flag& Running() const override;
    std::chrono::seconds SendTimeout() const override;

    opentxs::network::ServerConnection& Server(
        const std::string& id) const override;
    bool SetSocksProxy(const std::string& proxy) const override;
    std::string SocksProxy() const override;
    bool SocksProxy(std::string& proxy) const override;
    ConnectionState Status(const std::string& server) const override;

    ~ZMQ();

private:
    friend class opentxs::api::implementation::Native;

    const opentxs::network::zeromq::Context& context_;
    const api::Settings& config_;
    const Flag& running_;
    mutable std::atomic<std::chrono::seconds> linger_;
    mutable std::atomic<std::chrono::seconds> receive_timeout_;
    mutable std::atomic<std::chrono::seconds> send_timeout_;
    mutable std::atomic<std::chrono::seconds> keep_alive_;
    mutable std::mutex lock_;
    mutable std::string socks_proxy_;
    mutable std::map<std::string, OTServerConnection> server_connections_;

    bool verify_lock(const Lock& lock) const;

    void init(const Lock& lock) const;

    ZMQ(const opentxs::network::zeromq::Context& context,
        const api::Settings& config,
        const Flag& running);
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    ZMQ& operator=(const ZMQ&) = delete;
    ZMQ& operator=(const ZMQ&&) = delete;
};
}  // namespace opentxs::api::network::implementation
#endif  // OPENTXS_API_NETWORK_IMPLEMENTATION_ZMQ_HPP
