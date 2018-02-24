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

#ifndef OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_HPP

#include "opentxs/Forward.hpp"

#include <functional>

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator+=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator==;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator!=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator<;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator<=;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator>;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>::operator>=;
%template(OTZMQListenCallback) opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>;
%rename($ignore, regextarget=1, fullname=1) "opentxs::network::zeromq::ListenCallback::Factory.*";
%rename(ZMQListenCallback) opentxs::network::zeromq::ListenCallback;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ListenCallback
{
public:
    using ReceiveCallback = std::function<void(const Message&)>;

    EXPORT static OTZMQListenCallback Factory(ReceiveCallback callback);

    EXPORT virtual void Process(const Message& message) const = 0;

    EXPORT virtual ~ListenCallback() = default;

protected:
    ListenCallback() = default;

private:
    friend OTZMQListenCallback;

    virtual ListenCallback* clone() const = 0;

    ListenCallback(const ListenCallback&) = delete;
    ListenCallback(ListenCallback&&) = default;
    ListenCallback& operator=(const ListenCallback&) = delete;
    ListenCallback& operator=(ListenCallback&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_HPP
