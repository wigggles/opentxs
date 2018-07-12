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

#ifndef IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SECP256K1_HPP
#define IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SECP256K1_HPP

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "EllipticCurve.hpp"

namespace opentxs::crypto::key::implementation
{
class Secp256k1 final : virtual public key::Secp256k1, public EllipticCurve
{
public:
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;

    ~Secp256k1() = default;

private:
    using ot_super = EllipticCurve;

    friend key::Asymmetric;
    friend opentxs::Factory;

    Secp256k1();
    explicit Secp256k1(const proto::KeyRole role);
    explicit Secp256k1(const proto::AsymmetricKey& serializedKey);
    explicit Secp256k1(const String& publicKey);
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#endif  // IMPLEMENTATION_OPENTXS_CRYPTO_KEY_SECP256K1_HPP
