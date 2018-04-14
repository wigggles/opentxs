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

#ifndef OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP
#define OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP

#include "opentxs/Forward.hpp"

#include <memory>
#include <string>

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::Context::operator void*() const;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator+=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator==;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator!=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator<;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator<=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::Context>::operator>=;
%template(OTZMQContext) opentxs::Pimpl<opentxs::network::zeromq::Context>;
%rename(ZMQContext) opentxs::network::zeromq::Context;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context
{
public:
    EXPORT static Pimpl<opentxs::network::zeromq::Context> Factory();

    EXPORT virtual operator void*() const = 0;

    EXPORT virtual Pimpl<network::zeromq::SubscribeSocket> PairEventListener(
        const PairEventCallback& callback) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PairSocket> PairSocket(
        const ListenCallback& callback) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PairSocket> PairSocket(
        const ListenCallback& callback,
        const class PairSocket& peer) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PairSocket> PairSocket(
        const ListenCallback& callback,
        const std::string& endpoint) const = 0;
    EXPORT virtual Pimpl<network::zeromq::Proxy> Proxy(
        Socket& frontend,
        Socket& backend) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PublishSocket> PublishSocket()
        const = 0;
    EXPORT virtual Pimpl<network::zeromq::PullSocket> PullSocket(
        const bool client) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PullSocket> PullSocket(
        const ListenCallback& callback,
        const bool client) const = 0;
    EXPORT virtual Pimpl<network::zeromq::PushSocket> PushSocket(
        const bool client) const = 0;
    EXPORT virtual Pimpl<network::zeromq::ReplySocket> ReplySocket(
        const ReplyCallback& callback) const = 0;
    EXPORT virtual Pimpl<network::zeromq::RequestSocket> RequestSocket()
        const = 0;
    EXPORT virtual Pimpl<network::zeromq::SubscribeSocket> SubscribeSocket(
        const ListenCallback& callback) const = 0;

    EXPORT virtual ~Context() = default;

protected:
    Context() = default;

private:
    friend OTZMQContext;

    EXPORT virtual Context* clone() const = 0;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_CONTEXT_HPP
