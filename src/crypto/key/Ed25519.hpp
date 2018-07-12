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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_ED25519_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_ED25519_HPP

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "EllipticCurve.hpp"

namespace opentxs::crypto::key::implementation
{
class Ed25519 final : virtual public key::Ed25519,
                      public implementation::EllipticCurve
{
public:
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;
    bool hasCapability(const NymCapability& capability) const override;

    ~Ed25519() = default;

private:
    using ot_super = implementation::EllipticCurve;

    friend opentxs::Factory;
    friend LowLevelKeyGenerator;

    Ed25519();
    explicit Ed25519(const proto::KeyRole role);
    explicit Ed25519(const proto::AsymmetricKey& serializedKey);
    explicit Ed25519(const String& publicKey);
    Ed25519(const Ed25519&) = delete;
    Ed25519(Ed25519&&) = delete;
    Ed25519& operator=(const Ed25519&) = delete;
    Ed25519& operator=(Ed25519&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_ED25519_HPP
