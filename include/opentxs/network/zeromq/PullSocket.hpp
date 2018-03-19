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

#ifndef OPENTXS_NETWORK_ZEROMQ_PULLSOCKET_HPP
#define OPENTXS_NETWORK_ZEROMQ_PULLSOCKET_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/Socket.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator+=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator==;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator!=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator<;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator<=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::PullSocket>::operator>=;
%template(OTZMQPullSocket) opentxs::Pimpl<opentxs::network::zeromq::PullSocket>;
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::PullSocket::Factory.*";
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::PullSocket::SetCurve.*";
%rename(ZMQPullSocket) opentxs::network::zeromq::PullSocket;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PullSocket : virtual public Socket
{
public:
    EXPORT static OTZMQPullSocket Factory(
        const Context& context,
        const ListenCallback& callback);
    EXPORT static OTZMQPullSocket Factory(const Context& context);

    EXPORT virtual bool SetCurve(const OTPassword& key) const = 0;

    EXPORT virtual ~PullSocket() = default;

protected:
    PullSocket() = default;

private:
    friend OTZMQPullSocket;

    virtual PullSocket* clone() const = 0;

    PullSocket(const PullSocket&) = delete;
    PullSocket(PullSocket&&) = default;
    PullSocket& operator=(const PullSocket&) = delete;
    PullSocket& operator=(PullSocket&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_PULLSOCKET_HPP
