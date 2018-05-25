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

#ifndef OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP
#define OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP

#include "Internal.hpp"

#include "opentxs/Types.hpp"

#include <mutex>

namespace opentxs::network::zeromq::implementation
{
class CurveClient
{
protected:
    bool set_curve(const ServerContract& contract) const;

    CurveClient(std::mutex& lock, void* socket);
    ~CurveClient();

private:
    std::mutex& curve_lock_;
    // Not owned by this class
    void* curve_socket_{nullptr};

    bool set_local_keys(const Lock& lock) const;
    bool set_remote_key(const Lock& lock, const ServerContract& contract) const;

    CurveClient() = delete;
    CurveClient(const CurveClient&) = delete;
    CurveClient(CurveClient&&) = delete;
    CurveClient& operator=(const CurveClient&) = delete;
    CurveClient& operator=(CurveClient&&) = delete;
};
}  // namespace opentxs::network::zeromq::implementation
#endif  // OPENTXS_NETWORK_ZEROMQ_CURVECLIENT_IMPLEMENTATION_HPP
