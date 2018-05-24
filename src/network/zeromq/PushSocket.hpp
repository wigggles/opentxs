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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUSHSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUSHSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PushSocket.hpp"

#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class PushSocket : virtual public zeromq::PushSocket, public Socket
{
public:
    bool Push(const std::string& data) const override;
    bool Push(const opentxs::Data& data) const override;
    bool Push(zeromq::Message& data) const override;
    bool Start(const std::string& endpoint) const override;

    ~PushSocket() = default;

private:
    friend opentxs::network::zeromq::PushSocket;
    typedef Socket ot_super;

    const bool client_{false};

    PushSocket* clone() const override;

    PushSocket(const zeromq::Context& context, const bool client);
    PushSocket() = delete;
    PushSocket(const PushSocket&) = delete;
    PushSocket(PushSocket&&) = delete;
    PushSocket& operator=(const PushSocket&) = delete;
    PushSocket& operator=(PushSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUSHSOCKET_HPP
