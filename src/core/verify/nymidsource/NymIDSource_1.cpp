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

bool NymIDSource_1(
    const nymIDSource serializedNymIDSource,
    const proto::SourceType type)
{
    if (!serializedNymIDSource.has_type()) {
        otErr << "Verify serialized nym source failed: missing type.\n";
        return false;
    }

    if (serializedNymIDSource.type() != type) {
        otErr << "Verify serialized nym source failed: incorrect type ("
                << serializedNymIDSource.type() << ").\n";
        return false;
    }

    switch (type) {
        case SOURCETYPE_SELF :
            if (!serializedNymIDSource.has_raw()) {
                otErr << "Verify serialized nym source failed: missing source.\n";
                return false;
            }

            if (MIN_PLAUSIBLE_SOURCE > serializedNymIDSource.raw().size()) {
                otErr << "Verify serialized nym source failed: invalid source.\n";
                return false;
            }

            break;
        default :
            otErr << "Verify nym source failed: incorrect or unknown type ("
                    << serializedNymIDSource.type() << ").\n";

            return false;
    }

    return true;
}

} // namespace proto
} // namespace opentxs
