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

#include "opentxs/network/ZMQ.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace opentxs
{
namespace server
{

class Server;

class MessageProcessor
{
public:
    EXPORT explicit MessageProcessor(
        Server& server,
        std::atomic<bool>& shutdown);

    EXPORT void Cleanup();
    EXPORT void Init(int port, zcert_t* transportKey);
    EXPORT void Start();

    EXPORT ~MessageProcessor();

private:
    Server& server_;
    std::atomic<bool>& shutdown_;
    zsock_t* zmqSocket_{nullptr};
    zactor_t* zmqAuth_{nullptr};
    zpoller_t* zmqPoller_{nullptr};
    std::unique_ptr<std::thread> thread_{nullptr};

    bool processMessage(const std::string& messageString, std::string& reply);
    void processSocket();
    void run();
};
}  // namespace server
}  // namespace opentxs

#endif  // OPENTXS_SERVER_MESSAGEPROCESSOR_HPP
