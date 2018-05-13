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

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIRSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIRSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::PairSocket::Factory;
%template(OTZMQPairSocket) opentxs::Pimpl<opentxs::network::zeromq::PairSocket>;
%rename(ZMQPairSocket) opentxs::network::zeromq::PairSocket;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PairSocket : virtual public Socket
{
public:
    /** Construct a Pair socket
     *
     *  Sockets created via this function will bind to an automatically
     *  determined endpoint.
     *
     *  The generated endpoint will be available via the Endpoint() function
     */
    EXPORT static OTZMQPairSocket Factory(
        const class Context& context,
        const ListenCallback& callback);

    /** Construct a Pair socket
     *
     *  Sockets created via this function will connect to the same endpoint
     *  as the provided peer socket which must be bound to that endpoint.
     */
    EXPORT static OTZMQPairSocket Factory(
        const ListenCallback& callback,
        const PairSocket& peer);

    /** Construct a Pair socket
     *
     *  Sockets created via this function will connect to provided endpoint
     *  which must not be an empty string.
     */
    EXPORT static OTZMQPairSocket Factory(
        const class Context& context,
        const ListenCallback& callback,
        const std::string& endpoint);

    EXPORT virtual const std::string& Endpoint() const = 0;
    EXPORT virtual bool Send(const std::string& data) const = 0;
    EXPORT virtual bool Send(const opentxs::Data& data) const = 0;
    EXPORT virtual bool Send(network::zeromq::Message& data) const = 0;

    EXPORT virtual ~PairSocket() = default;

protected:
    EXPORT PairSocket() = default;

private:
    friend OTZMQPairSocket;

    virtual PairSocket* clone() const = 0;

    PairSocket(const PairSocket&) = delete;
    PairSocket(PairSocket&&) = default;
    PairSocket& operator=(const PairSocket&) = delete;
    PairSocket& operator=(PairSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_PAIRSOCKET_HPP
