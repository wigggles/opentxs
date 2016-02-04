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

#include "CmdNewAsset.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/Log.hpp>

using namespace opentxs;
using namespace std;

CmdNewAsset::CmdNewAsset()
{
    command = "newasset";
    args[0] = "--mynym <nym>";
    category = catAdmin;
    help = "Create a new asset contract.";
}

CmdNewAsset::~CmdNewAsset()
{
}

int32_t CmdNewAsset::runWithOptions()
{
    return run(getOption("mynym"));
}

int32_t CmdNewAsset::run(string mynym)
{
    if (!checkNym("mynym", mynym)) {
        return -1;
    }

    string input = inputText("an asset contract");
    if ("" == input) {
        return -1;
    }

    string assetType = OTAPI_Wrap::CreateUnitDefinition(mynym, input);
    if ("" == assetType) {
        otOut << "Error: cannot create asset contract.\n";
        return -1;
    }

    cout << "New instrument definition ID : " << assetType << "\n";

    string contract = OTAPI_Wrap::GetAssetType_Contract(assetType);
    if ("" == contract) {
        otOut << "Error: cannot load asset contract.\n";
        return -1;
    }

    cout << contract << "\n";

    return 1;
}
