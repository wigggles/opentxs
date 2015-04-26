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

#include "CmdNewServer.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdNewServer::CmdNewServer()
{
    command = "newserver";
    args[0] = "--mynym <nym>";
    category = catAdmin;
    help = "Create a new server contract.";
}

CmdNewServer::~CmdNewServer()
{
}

int32_t CmdNewServer::runWithOptions()
{
    return run(getOption("mynym"));
}

int32_t CmdNewServer::run(string mynym)
{
    if (!checkNym("mynym", mynym)) {
        return -1;
    }

    string input = inputText("a server contract");
    if ("" == input) {
        return -1;
    }

    string server = OTAPI_Wrap::CreateServerContract(mynym, input);
    if ("" == server) {
        otOut << "Error: cannot create server contract.\n";
        return -1;
    }

    cout << "New server ID : " << server << "\n";

    string contract = OTAPI_Wrap::GetServer_Contract(server);
    if ("" == contract) {
        otOut << "Error: cannot load server contract.\n";
        return -1;
    }

    cout << contract << "\n";

    return 1;
}
