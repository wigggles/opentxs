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

#ifndef OPENTXS_CRYPTO_LIBRARY_SODIUM_HPP
#define OPENTXS_CRYPTO_LIBRARY_SODIUM_HPP

#include "Internal.hpp"

#include "opentxs/api/crypto/Util.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

namespace opentxs::crypto
{
class Sodium : virtual public api::crypto::Util
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public AsymmetricProvider,
               virtual public EcdsaProvider
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public HashingProvider,
               virtual public SymmetricProvider
{
public:
    virtual ~Sodium() = default;

protected:
    Sodium() = default;

private:
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    Sodium& operator=(const Sodium&) = delete;
    Sodium& operator=(Sodium&&) = delete;
};
}  // namespace opentxs::crypto
#endif  // OPENTXS_CRYPTO_LIBRARY_SODIUM_HPP
