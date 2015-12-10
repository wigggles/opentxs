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

#ifndef OPENTXS_CORE_CRYPTO_BIP32_HPP
#define OPENTXS_CORE_CRYPTO_BIP32_HPP

#include <string>
#include <opentxs/core/crypto/OTAsymmetricKey.hpp>

namespace opentxs
{
class OTPassword;

class Bip32
{

public:

    virtual serializedAsymmetricKey SeedToPrivateKey(
        const OTPassword& seed) const = 0;
    virtual serializedAsymmetricKey GetChild(
        const proto::AsymmetricKey& parent,
        const uint32_t index) const = 0;
    virtual serializedAsymmetricKey PrivateToPublic(
        const serializedAsymmetricKey& key) const = 0;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_BIP32_HPP
