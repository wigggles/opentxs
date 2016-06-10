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

#include "CmdNewNymECDSA.hpp"

#include "opentxs/client/OTAPI.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/core/Log.hpp"

using namespace opentxs;
using namespace std;

CmdNewNymECDSA::CmdNewNymECDSA()
{
    command = "newnymecdsa";
    args[0] = "--label <label>";
    args[2] = "[--source <source>]";
    args[3] = "[--location <location>]";
    category = catNyms;
    help = "create a new secp256k1 nym.";
}

CmdNewNymECDSA::~CmdNewNymECDSA()
{
}

int32_t CmdNewNymECDSA::runWithOptions()
{
    return run(getOption("label"), getOption("source"));
}

// FYI, a source can be a URL, a Bitcoin address, a Namecoin address,
// a public key, or the unique DN info from a traditionally-issued cert.
// Hashing the source should produce the NymID. Also, the source should
// always (somehow) validate the credential IDs, if they are to be trusted
// for their purported Nym.
//
// NOTE: If you leave the source BLANK, then OT will just generate a public
// key to serve as the source. The public key will be hashed to form the
// NymID, and all credentials for that Nym will need to be signed by the
// corresponding private key. That's the only way they can be 'verified by
// their source.'

int32_t CmdNewNymECDSA::run(
    string label,
    string source)
{
    if (!checkMandatory("label", label)) {
        return -1;
    }

    OT_ME ot_me;
    string mynym = ot_me.create_nym_ecdsa(source);
    if ("" == mynym) {
        otOut << "Error: cannot create new nym.\n";
        return -1;
    }

    cout << "New nym: " << mynym << "\n";

    if (!OTAPI_Wrap::SetNym_Name(mynym, mynym, label)) {
        otOut << "Error: cannot set new nym name.\n";
        return -1;
    }
    return 1;
}
