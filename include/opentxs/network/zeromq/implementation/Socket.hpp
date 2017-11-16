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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_SOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_SOCKET_HPP

#include "opentxs/Version.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#include <map>
#include <mutex>

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;

namespace implementation
{

class Socket : virtual public zeromq::Socket
{
public:
    SocketType Type() const override;

    operator void*() override;

    bool Close() override;
    bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) override;

    virtual ~Socket();

protected:
    const zeromq::Context& context_;
    void* socket_{nullptr};
    std::mutex lock_{};

    explicit Socket(const zeromq::Context& context, const SocketType type);

private:
    static const std::map<SocketType, int> types_;

    const SocketType type_{SocketType::Error};

    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;
};
}  // namespace implementation
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_SOCKET_HPP
