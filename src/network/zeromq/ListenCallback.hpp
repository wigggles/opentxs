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

#ifndef OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_IMPLEMENTATION_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/network/zeromq/ListenCallback.hpp"

namespace opentxs::network::zeromq::implementation
{
class ListenCallback : virtual public zeromq::ListenCallback
{
public:
    void Process(const zeromq::Message& message) const override;

    ~ListenCallback();

private:
    friend zeromq::ListenCallback;

    const zeromq::ListenCallback::ReceiveCallback callback_;

    ListenCallback* clone() const override;

    ListenCallback(zeromq::ListenCallback::ReceiveCallback callback);
    ListenCallback() = delete;
    ListenCallback(const ListenCallback&) = delete;
    ListenCallback(ListenCallback&&) = delete;
    ListenCallback& operator=(const ListenCallback&) = delete;
    ListenCallback& operator=(ListenCallback&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_IMPLEMENTATION_HPP
