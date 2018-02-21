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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/RequestSocket.hpp"

#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class RequestSocket : virtual public zeromq::RequestSocket, public Socket
{
public:
    MessageSendResult SendRequest(opentxs::Data& message) override;
    MessageSendResult SendRequest(std::string& message) override;
    MessageSendResult SendRequest(zeromq::Message& message) override;
    bool SetCurve(const ServerContract& contract) override;
    bool SetSocksProxy(const std::string& proxy) override;
    bool Start(const std::string& endpoint) override;

    ~RequestSocket() = default;

private:
    friend opentxs::network::zeromq::RequestSocket;
    typedef Socket ot_super;

    bool set_local_keys(const Lock& lock);
    bool set_remote_key(const Lock& lock, const ServerContract& contract);

    RequestSocket(const zeromq::Context& context);
    RequestSocket() = delete;
    RequestSocket(const RequestSocket&) = delete;
    RequestSocket(RequestSocket&&) = delete;
    RequestSocket& operator=(const RequestSocket&) = delete;
    RequestSocket& operator=(RequestSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REQUESTSOCKET_HPP
