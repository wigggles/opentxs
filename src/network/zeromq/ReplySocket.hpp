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

#ifndef OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ReplySocket.hpp"

#include "CurveServer.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplySocket : virtual public zeromq::ReplySocket,
                    public Socket,
                    CurveServer,
                    Receiver
{
public:
    void RegisterCallback(RequestCallback callback) override;
    bool SetCurve(const OTPassword& key) override;
    bool Start(const std::string& endpoint) override;

    ~ReplySocket();

private:
    friend opentxs::network::zeromq::ReplySocket;
    typedef Socket ot_super;

    RequestCallback callback_{nullptr};

    bool have_callback() const override;

    void process_incoming(const Lock& lock, Message& message) override;

    ReplySocket(const zeromq::Context& context);
    ReplySocket() = delete;
    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = delete;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP
