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

#ifndef OPENTXS_NETWORK_DHTCONFIG_HPP
#define OPENTXS_NETWORK_DHTCONFIG_HPP

#include "opentxs/Forward.hpp"

#include <string>

namespace opentxs
{

class DhtConfig
{
public:
    bool enable_dht_ = false;
    const int64_t default_port_ = 4222;
    int64_t listen_port_ = 4222;
    int64_t nym_publish_interval_ = 60 * 5;
    int64_t nym_refresh_interval_ = 60 * 60 * 1;
    int64_t server_publish_interval_ = 60 * 5;
    int64_t server_refresh_interval_ = 60 * 60 * 1;
    int64_t unit_publish_interval_ = 60 * 5;
    int64_t unit_refresh_interval_ = 60 * 60 * 1;
    std::string bootstrap_url_ = "bootstrap.ring.cx";
    std::string bootstrap_port_ = "4222";
};

}  // namespace opentxs
#endif  // OPENTXS_NETWORK_DHTCONFIG_HPP
