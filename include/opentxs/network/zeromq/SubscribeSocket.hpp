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

#ifndef OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%template(OTZMQSubscribeSocket) opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>;
%ignore opentxs::network::zeromq::SubscribeSocket::Factory;
%ignore opentxs::network::zeromq::SubscribeSocket::SetCurve;
%rename(ZMQSubscribeSocket) opentxs::network::zeromq::SubscribeSocket;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class SubscribeSocket : virtual public Socket
{
public:
    EXPORT static OTZMQSubscribeSocket Factory(
        const class Context& context,
        const ListenCallback& callback);

    EXPORT virtual bool SetCurve(const ServerContract& contract) const = 0;
    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~SubscribeSocket() = default;

protected:
    SubscribeSocket() = default;

private:
    friend OTZMQSubscribeSocket;

    virtual SubscribeSocket* clone() const = 0;

    SubscribeSocket(const SubscribeSocket&) = delete;
    SubscribeSocket(SubscribeSocket&&) = default;
    SubscribeSocket& operator=(const SubscribeSocket&) = delete;
    SubscribeSocket& operator=(SubscribeSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_SUBSCRIBESOCKET_HPP
