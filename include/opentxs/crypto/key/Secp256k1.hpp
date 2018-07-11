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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP

#include "opentxs/Forward.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/Proto.hpp"

namespace opentxs
{
class LowLevelKeyGenerator;

namespace crypto
{
namespace key
{
class Secp256k1 : public EllipticCurve
{
private:
    typedef EllipticCurve ot_super;

    friend Asymmetric;
    friend opentxs::LowLevelKeyGenerator;

    Secp256k1();
    explicit Secp256k1(const proto::KeyRole role);
    explicit Secp256k1(const proto::AsymmetricKey& serializedKey);
    explicit Secp256k1(const String& publicKey);

public:
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;
    void Release_Secp256k1(){};
    void Release() override;

    virtual ~Secp256k1();
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP
