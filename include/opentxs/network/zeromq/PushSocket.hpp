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

#ifndef OPENTXS_NETWORK_ZEROMQ_PUSHSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PUSHSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::PushSocket::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::Pimpl(opentxs::network::zeromq::PushSocket const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::operator opentxs::network::zeromq::PushSocket&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PushSocket>::operator const opentxs::network::zeromq::PushSocket &;
%rename(assign) operator=(const opentxs::network::zeromq::PushSocket&);
%rename(ZMQPushSocket) opentxs::network::zeromq::PushSocket;
%template(OTZMQPushSocket) opentxs::Pimpl<opentxs::network::zeromq::PushSocket>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PushSocket : virtual public Socket
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::PushSocket> Factory(
        const opentxs::network::zeromq::Context& context,
        const bool client);

    EXPORT virtual bool Push(const std::string& data) const = 0;
    EXPORT virtual bool Push(const opentxs::Data& data) const = 0;
    EXPORT virtual bool Push(network::zeromq::MultipartMessage& data) const = 0;

    EXPORT virtual ~PushSocket() = default;

protected:
    EXPORT PushSocket() = default;

private:
    friend OTZMQPushSocket;

    virtual PushSocket* clone() const = 0;

    PushSocket(const PushSocket&) = delete;
    PushSocket(PushSocket&&) = default;
    PushSocket& operator=(const PushSocket&) = delete;
    PushSocket& operator=(PushSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_PUSHSOCKET_HPP
