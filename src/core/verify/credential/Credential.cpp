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

#include <opentxs/core/verify/Verify.hpp>

#include <opentxs/core/Log.hpp>

namespace opentxs { namespace proto
{

bool Verify(
    const Credential serializedCred,
    const proto::CredentialRole role,
    const bool withSigs)
{
    if (!serializedCred.has_version()) {
        otErr << "Verify serialized credential failed: missing version.\n";
        return false;
    }

    switch (serializedCred.version()) {
        case 1 :

            return Credential_1(serializedCred, role, withSigs);
        default :
            otErr << "Verify serialized credential failed: unknown version ("
                  << serializedCred.version() << ").\n";

            return false;
    }
    return true;
}

} // namespace proto
} // namespace opentxs
