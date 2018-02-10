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

#include "opentxs/network/zeromq/implementation/Socket.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"

#include <string>

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace implementation
{

class ReplySocket : virtual public zeromq::ReplySocket, public Socket
{
public:
    MessageReceiveResult ReceiveRequest(BlockMode block) override;
    bool SendReply(const std::string& reply) override;
    bool SendReply(const opentxs::Data& reply) override;
    bool SendReply(zeromq::Message& reply) override;
    bool SetCurve(const OTPassword& key) override;
    bool Start(const std::string& endpoint) override;

    ~ReplySocket() = default;

private:
    friend class Context;
    typedef Socket ot_super;

    ReplySocket(const zeromq::Context& context);
    ReplySocket() = delete;
    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = delete;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = delete;
};
}  // namespace implementation
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_IMPLEMENTATION_REPLYSOCKET_HPP
