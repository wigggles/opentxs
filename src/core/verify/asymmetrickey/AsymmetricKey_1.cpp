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

bool AsymmetricKey_1(
    const AsymmetricKey serializedAsymmetricKey,
    const proto::CredentialType type,
    const proto::KeyMode mode,
    const proto::KeyRole role)
{
    switch (type) {
        case CREDTYPE_LEGACY :
            if (!serializedAsymmetricKey.has_type()) {
                otErr << "Verify serialized asymmetric key failed: missing key type.\n";
                return false;
            }

            if ((AKEYTYPE_LEGACY != serializedAsymmetricKey.type()) && (AKEYTYPE_SECP256K1 != serializedAsymmetricKey.type())) {
                otErr << "Verify serialized asymmetric key failed: incorrect key type ("
                << serializedAsymmetricKey.type() << ").\n";
                return false;
            }

            if (!serializedAsymmetricKey.has_mode()) {
                otErr << "Verify serialized asymmetric key failed: missing key mode.\n";
                return false;
            }

            if (serializedAsymmetricKey.mode() != mode) {
                otErr << "Verify serialized asymmetric key failed: incorrect key mode ("
                << serializedAsymmetricKey.mode() << ").\n";
                return false;
            }

            if (!serializedAsymmetricKey.has_role()) {
                otErr << "Verify serialized asymmetric key failed: missing key role.\n";
                return false;
            }

            if (serializedAsymmetricKey.role() != role) {
                otErr << "Verify serialized asymmetric key failed: incorrect key role ("
                << serializedAsymmetricKey.role() << ").\n";
                return false;
            }

            if (!serializedAsymmetricKey.has_key()) {
                otErr << "Verify serialized asymmetric key failed: missing key.\n";
                return false;
            }

            if (MIN_PLAUSIBLE_KEYSIZE > serializedAsymmetricKey.key().size()) {
                otErr << "Verify serialized asymmetric key failed: invalid key ("
                << serializedAsymmetricKey.key() << ").\n";
                return false;
            }

            if (serializedAsymmetricKey.has_chaincode()) {
                otErr << "Verify serialized asymmetric key failed: chaincode not allowed for this key version.\n";
                return false;
            }

            if (serializedAsymmetricKey.has_path()) {
                otErr << "Verify serialized asymmetric key failed: path not allowed for this key version.\n";
                return false;
            }

            break;
        default :
            otErr << "Verify asymmetric key failed: incorrect or unknown credential; type ("
                    << serializedAsymmetricKey.type() << ").\n";

            return false;
    }

    return true;
}

} // namespace proto
} // namespace opentxs
