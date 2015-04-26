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

#ifndef OPENTXS_SERVER_MESSAGEPROCESSOR_HPP
#define OPENTXS_SERVER_MESSAGEPROCESSOR_HPP

#include <string>
#include <memory>
#include <czmq.h>

// forward declare czmq types
typedef struct _zsock_t zsock_t;
typedef struct _zactor_t zactor_t;
typedef struct _zpoller_t zpoller_t;

namespace opentxs
{

class ServerLoader;
class OTServer;

class MessageProcessor
{
public:
    EXPORT explicit MessageProcessor(ServerLoader& loader);
    ~MessageProcessor();
    EXPORT void run();

private:
    void init(int port, zcert_t* transportKey);
    bool processMessage(const std::string& messageString, std::string& reply);
    void processSocket();

private:
    OTServer* server_;
    zsock_t* zmqSocket_;
    zactor_t* zmqAuth_;
    zpoller_t* zmqPoller_;
};

} // namespace opentxs

#endif // OPENTXS_SERVER_MESSAGEPROCESSOR_HPP
