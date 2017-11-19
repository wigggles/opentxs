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

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_HPP

#include "opentxs/Version.hpp"

#include "opentxs/Types.hpp"

#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <tuple>

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;

#ifdef SWIG
// clang-format off
%ignore Socket::operator void*();
%ignore Socket::SetTimeouts(const std::chrono::milliseconds&, std::chrono::milliseconds&, const std::chrono::milliseconds&);
// clang-format on
#endif  // SWIG

class Socket
{
public:
    typedef std::pair<SendResult, std::shared_ptr<Message>> MessageSendResult;
    typedef std::pair<bool, std::shared_ptr<Message>> MessageReceiveResult;

    EXPORT virtual SocketType Type() const = 0;

    EXPORT virtual operator void*() = 0;

    EXPORT virtual bool Close() = 0;
    EXPORT virtual bool SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) = 0;
    EXPORT virtual bool SetTimeouts(
        const std::uint64_t& lingerMilliseconds,
        const std::uint64_t& sendMilliseconds,
        const std::uint64_t& receiveMilliseconds) = 0;
    EXPORT virtual bool Start(const std::string& endpoint) = 0;

    EXPORT virtual ~Socket() = default;

protected:
    Socket() = default;

private:
    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_SOCKET_HPP
