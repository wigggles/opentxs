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

#include "opentxs/Version.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1

#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/Proto.hpp"

namespace opentxs
{

class String;

class AsymmetricKeySecp256k1 : public AsymmetricKeyEC
{
private:
    typedef AsymmetricKeyEC ot_super;
    friend class OTAsymmetricKey;  // For the factory.
    friend class LowLevelKeyGenerator;

    AsymmetricKeySecp256k1();
    explicit AsymmetricKeySecp256k1(const proto::KeyRole role);
    explicit AsymmetricKeySecp256k1(const proto::AsymmetricKey& serializedKey);
    explicit AsymmetricKeySecp256k1(const String& publicKey);

public:
    Ecdsa& ECDSA() const override;
    CryptoAsymmetric& engine() const override;
    void Release_AsymmetricKeySecp256k1(){};
    void Release() override;

    virtual ~AsymmetricKeySecp256k1();
};
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYSECP256K1_HPP
