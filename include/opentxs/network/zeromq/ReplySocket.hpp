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

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%template(OTZMQReplySocket) opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>;
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::ReplySocket::Factory.*";
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::ReplySocket::SetCurve.*";
%rename(ZMQReplySocket) opentxs::network::zeromq::ReplySocket;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplySocket : virtual public Socket
{
public:
    EXPORT static OTZMQReplySocket Factory(
        const class Context& context,
        const ReplyCallback& callback);

    EXPORT virtual bool SetCurve(const OTPassword& key) const = 0;

    EXPORT virtual ~ReplySocket() = default;

protected:
    EXPORT ReplySocket() = default;

private:
    friend OTZMQReplySocket;

    virtual ReplySocket* clone() const = 0;

    ReplySocket(const ReplySocket&) = delete;
    ReplySocket(ReplySocket&&) = default;
    ReplySocket& operator=(const ReplySocket&) = delete;
    ReplySocket& operator=(ReplySocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_REPLYSOCKET_HPP
