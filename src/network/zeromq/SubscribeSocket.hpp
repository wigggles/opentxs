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

#ifndef OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/SubscribeSocket.hpp"

#include "CurveClient.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class SubscribeSocket : virtual public zeromq::SubscribeSocket,
                        public Socket,
                        CurveClient,
                        Receiver
{
public:
    bool SetCurve(const ServerContract& contract) const override;
    bool SetSocksProxy(const std::string& proxy) const override;
    bool Start(const std::string& endpoint) const override;

    virtual ~SubscribeSocket();

protected:
    const ListenCallback& callback_;

    SubscribeSocket(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback);

private:
    friend opentxs::network::zeromq::SubscribeSocket;
    typedef Socket ot_super;

    SubscribeSocket* clone() const override;
    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    SubscribeSocket() = delete;
    SubscribeSocket(const SubscribeSocket&) = delete;
    SubscribeSocket(SubscribeSocket&&) = delete;
    SubscribeSocket& operator=(const SubscribeSocket&) = delete;
    SubscribeSocket& operator=(SubscribeSocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_IMPLEMENTATION_HPP
