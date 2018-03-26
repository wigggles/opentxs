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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUBLISHSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUBLISHSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/PublishSocket.hpp"

#include "CurveServer.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class PublishSocket : virtual public zeromq::PublishSocket,
                      public Socket,
                      CurveServer
{
public:
    bool Publish(const std::string& data) const override;
    bool Publish(const opentxs::Data& data) const override;
    bool Publish(zeromq::Message& data) const override;
    bool SetCurve(const OTPassword& key) const override;
    bool Start(const std::string& endpoint) const override;

    ~PublishSocket() = default;

private:
    friend opentxs::network::zeromq::PublishSocket;
    typedef Socket ot_super;

    PublishSocket* clone() const override;

    PublishSocket(const zeromq::Context& context);
    PublishSocket() = delete;
    PublishSocket(const PublishSocket&) = delete;
    PublishSocket(PublishSocket&&) = delete;
    PublishSocket& operator=(const PublishSocket&) = delete;
    PublishSocket& operator=(PublishSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_PUBLISHSOCKET_HPP
