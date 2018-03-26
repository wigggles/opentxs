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

#include "opentxs/Internal.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Socket.hpp"

#include <map>
#include <mutex>

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40

namespace opentxs::network::zeromq::implementation
{
class Socket : virtual public zeromq::Socket, public Lockable
{
public:
    SocketType Type() const override;

    operator void*() const override;

    bool Close() const override;
    const zeromq::Context& Context() const override { return context_; }
    bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const override;
    bool SetTimeouts(
        const std::uint64_t& lingerMilliseconds,
        const std::uint64_t& sendMilliseconds,
        const std::uint64_t& receiveMilliseconds) const override;

    virtual ~Socket();

protected:
    const zeromq::Context& context_;
    void* socket_{nullptr};

    bool bind(const std::string& endpoint) const;
    bool connect(const std::string& endpoint) const;
    bool set_socks_proxy(const std::string& proxy) const;
    bool start_client(const std::string& endpoint) const;

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
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_SOCKET_HPP
