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

#ifndef OPENTXS_API_NETWORK_ZMQ_HPP
#define OPENTXS_API_NETWORK_ZMQ_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Types.hpp"

#include <chrono>
#include <string>

namespace opentxs
{
class ServerConnection;

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace api
{
namespace network
{

class ZMQ
{
public:
    virtual const opentxs::network::zeromq::Context& Context() const = 0;
    virtual std::chrono::seconds KeepAlive() const = 0;
    virtual void KeepAlive(const std::chrono::seconds duration) const = 0;
    virtual std::chrono::seconds Linger() = 0;
    virtual std::chrono::seconds ReceiveTimeout() = 0;
    virtual void RefreshConfig() = 0;
    virtual std::chrono::seconds SendTimeout() = 0;

    virtual ServerConnection& Server(const std::string& id) = 0;
    virtual bool SetSocksProxy(const std::string& proxy) = 0;
    virtual std::string SocksProxy() = 0;
    virtual bool SocksProxy(std::string& proxy) = 0;
    virtual ConnectionState Status(const std::string& server) const = 0;

    virtual ~ZMQ() = default;

protected:
    ZMQ() = default;

private:
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    ZMQ& operator=(const ZMQ&) = delete;
    ZMQ& operator=(const ZMQ&&) = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_NETWORK_ZMQ_HPP
