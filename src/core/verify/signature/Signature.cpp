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
    const Signature serializedSignature,
    const uint32_t minVersion,
    const uint32_t maxVersion,
    const std::string selfID,
    const std::string masterID,
    uint32_t& selfPublic,
    uint32_t& selfPrivate,
    uint32_t& masterPublic)
{
    if (!serializedSignature.has_version()) {
        otErr << "Verify serialized signature failed: missing version.\n";
        return false;
    }

    uint32_t version = serializedSignature.version();

    if ((version < minVersion) || (version > maxVersion)) {
        otErr << "Verify serialized signature failed: incorrect version ("
              << serializedSignature.version() << ").\n";
        return false;
    }

    switch (version) {
        case 1 :

            return Signature_1(serializedSignature, selfID, masterID, selfPublic, selfPrivate, masterPublic);
        default :
            otErr << "Verify serialized signature failed: unknown version ("
                  << serializedSignature.version() << ").\n";

            return false;
    }
    return true;
}

} // namespace proto
} // namespace opentxs
