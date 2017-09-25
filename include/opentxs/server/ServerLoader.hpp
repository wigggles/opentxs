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

#define OT_SERVER_OPTION_BACKUP "backup"
#define OT_SERVER_OPTION_BINDIP "bindip"
#define OT_SERVER_OPTION_COMMANDPORT "commandport"
#define OT_SERVER_OPTION_EEP "eep"
#define OT_SERVER_OPTION_GC "gc"
#define OT_SERVER_OPTION_EXTERNALIP "externalip"
#define OT_SERVER_OPTION_LISTENCOMMAND "listencommand"
#define OT_SERVER_OPTION_LISTENNOTIFY "listennotify"
#define OT_SERVER_OPTION_NAME "name"
#define OT_SERVER_OPTION_NOTIFICATIONPORT "notificationport"
#define OT_SERVER_OPTION_ONION "onion"
#define OT_SERVER_OPTION_STORAGE "storage"
#define OT_SERVER_OPTION_TERMS "terms"

#define SERVER_CONFIG_KEY "server"

namespace opentxs
{

class OTServer;

class ServerLoader
{
private:
    static OTServer* server_;

    ServerLoader() = delete;
    ServerLoader(const ServerLoader&) = delete;
    ServerLoader(ServerLoader&&) = delete;
    ServerLoader& operator=(const ServerLoader&) = delete;
    ServerLoader& operator=(ServerLoader&&) = delete;

public:
    ServerLoader(std::map<std::string, std::string>& args);

    int getPort() const;
    zcert_t* getTransportKey() const;

    static OTServer* getServer();

    ~ServerLoader();
};
}  // namespace opentxs

#endif  // OPENTXS_SERVER_SERVERLOADER_HPP
