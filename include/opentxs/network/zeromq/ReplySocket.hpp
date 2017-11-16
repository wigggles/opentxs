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

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP

#include "opentxs/Version.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#include <string>

namespace opentxs
{
class Data;
class OTPassword;

namespace network
{
namespace zeromq
{
class Message;

class ReplySocket : virtual public Socket
{
public:
    EXPORT virtual MessageReceiveResult ReceiveRequest(BlockMode block) = 0;
    EXPORT virtual bool SendReply(const std::string& reply) = 0;
    EXPORT virtual bool SendReply(const opentxs::Data& reply) = 0;
    EXPORT virtual bool SendReply(Message& reply) = 0;
    EXPORT virtual bool SetCurve(const OTPassword& key) = 0;

    EXPORT virtual ~ReplySocket() = default;

protected:
    EXPORT ReplySocket() = default;

private:
    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = default;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP
