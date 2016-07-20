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

#ifndef OPENTXS_CORE_TYPES_HPP
#define OPENTXS_CORE_TYPES_HPP

#include <cstdint>
#include <list>
#include <set>
#include <tuple>
#include <string>

namespace opentxs
{
typedef bool CredentialIndexModeFlag;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_ONLY_IDS = true;
static const CredentialIndexModeFlag CREDENTIAL_INDEX_MODE_FULL_CREDS = false;

/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
typedef std::tuple<
    std::string,             // claim identifier
    std::uint32_t,           // section
    std::uint32_t,           // type
    std::string,             // value
    std::int64_t,            // start time
    std::int64_t,            // end time
    std::set<std::uint32_t>> // attributes
        Claim;

/** C++11 representation of all contact data associated with a nym, aggregating
 *  each the nym's contact credentials in the event it has more than one.
 */
typedef std::set<Claim> ClaimSet;

/** A list of object IDs and their associated aliases
    *  * string: id of the stored object
    *  * string: alias of the stored object
    */
typedef std::list<std::pair<std::string, std::string>> ObjectList;

enum class ClaimPolarity : std::uint8_t {
    NEUTRAL  = 0,
    POSITIVE = 1,
    NEGATIVE = 2
};

enum class StorageBox : std::uint8_t {
    SENTPEERREQUEST  = 0,
    INCOMINGPEERREQUEST = 1,
    SENTPEERREPLY = 2,
    INCOMINGPEERREPLY = 3,
    FINISHEDPEERREQUEST = 4,
    FINISHEDPEERREPLY = 5,
    PROCESSEDPEERREQUEST = 6,
    PROCESSEDPEERREPLY = 7
};

enum class Bip43Purpose : std::uint32_t {
    PAYCODE = 47,                                           // BIP-47
    NYM = 0x4f544e4d                                        // OTNM
};

enum class Bip44Type : std::uint32_t {
    BITCOIN = 0
};

enum class Bip32Child : std::uint32_t {
    AUTH_KEY = 0x41555448,
    ENCRYPT_KEY = 0x454e4352,
    SIGN_KEY = 0x5349474e,
    HARDENED = 0x80000000,
};

enum class EcdsaCurve : std::uint8_t {
    ERROR = 0,
    SECP256K1 = 1,
    ED25519 = 2,
};

} // namespace opentxs
#endif // OPENTXS_CORE_TYPES_HPP
