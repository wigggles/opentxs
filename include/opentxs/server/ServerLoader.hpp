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

#ifndef OPENTXS_SERVER_SERVERLOADER_HPP
#define OPENTXS_SERVER_SERVERLOADER_HPP

#include "opentxs/network/ZMQ.hpp"

#include <map>
#include <string>

#define SERVER_CONFIG_KEY "server"

namespace opentxs
{

class OTServer;

class ServerLoader
{
private:
    OTServer* server_{nullptr};

    ServerLoader() = delete;
    ServerLoader(const ServerLoader&) = delete;
    ServerLoader(ServerLoader&&) = delete;
    ServerLoader& operator=(const ServerLoader&) = delete;
    ServerLoader& operator=(ServerLoader&&) = delete;

public:
    ServerLoader(std::map<std::string, std::string>& args);

    int getPort() const;
    zcert_t* getTransportKey() const;

    OTServer* getServer();

    ~ServerLoader();
};
} // namespace opentxs

#endif // OPENTXS_SERVER_SERVERLOADER_HPP
