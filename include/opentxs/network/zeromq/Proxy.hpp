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

#ifndef OPENTXS_NETWORK_ZEROMQ_PROXY_HPP
#define OPENTXS_NETWORK_ZEROMQ_PROXY_HPP

#include "opentxs/Forward.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::Proxy::Factory;
%template(OTZMQProxy) opentxs::Pimpl<opentxs::network::zeromq::Proxy>;
%rename(ZMQProxy) opentxs::network::zeromq::Proxy;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Proxy
{
public:
    static OTZMQProxy Factory(
        const Context& context,
        Socket& frontend,
        Socket& backend);

    EXPORT virtual ~Proxy() = default;

protected:
    Proxy() = default;

private:
    friend OTZMQProxy;

    virtual Proxy* clone() const = 0;

    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = default;
    Proxy& operator=(const Proxy&) = delete;
    Proxy& operator=(Proxy&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif  // OPENTXS_NETWORK_ZEROMQ_PROXY_HPP
