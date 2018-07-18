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

#ifndef OPENTXS_NETWORK_ZEROMQ_ROUTERSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_ROUTERSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::RouterSocket::Factory;
%ignore opentxs::network::zeromq::RouterSocket::SetCurve;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>::Pimpl(opentxs::network::zeromq::RouterSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>::operator opentxs::network::zeromq::RouterSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>::operator const opentxs::network::zeromq::RouterSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::RouterSocket&);
%rename(ZMQRouterSocket) opentxs::network::zeromq::RouterSocket;
%template(OTZMQRouterSocket) opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class RouterSocket : virtual public Socket
{
public:
    EXPORT static OTZMQRouterSocket Factory(
        const class Context& context,
        const bool client,
        const ListenCallback& callback);

    EXPORT virtual bool Send(opentxs::Data& message) const = 0;
    EXPORT virtual bool Send(const std::string& message) const = 0;
    EXPORT virtual bool Send(
        opentxs::network::zeromq::Message& message) const = 0;
    EXPORT virtual bool SetCurve(const ServerContract& contract) const = 0;
    EXPORT virtual bool SetSocksProxy(const std::string& proxy) const = 0;

    EXPORT virtual ~RouterSocket() = default;

protected:
    RouterSocket() = default;

private:
    friend OTZMQRouterSocket;

    virtual RouterSocket* clone() const = 0;

    RouterSocket(const RouterSocket&) = delete;
    RouterSocket(RouterSocket&&) = default;
    RouterSocket& operator=(const RouterSocket&) = delete;
    RouterSocket& operator=(RouterSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_ROUTERSOCKET_HPP
