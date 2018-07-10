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

#ifndef OPENTXS_CORE_CRYPTO_ASYMMETRICKEYED25519_HPP
#define OPENTXS_CORE_CRYPTO_ASYMMETRICKEYED25519_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/Proto.hpp"

namespace opentxs
{

class String;

class AsymmetricKeyEd25519 : public AsymmetricKeyEC
{
private:
    typedef AsymmetricKeyEC ot_super;
    friend class OTAsymmetricKey;  // For the factory.
    friend class LowLevelKeyGenerator;

    AsymmetricKeyEd25519();
    explicit AsymmetricKeyEd25519(const proto::KeyRole role);
    explicit AsymmetricKeyEd25519(const proto::AsymmetricKey& serializedKey);
    explicit AsymmetricKeyEd25519(const String& publicKey);

public:
    const crypto::EcdsaProvider& ECDSA() const override;
    const crypto::AsymmetricProvider& engine() const override;
    bool hasCapability(const NymCapability& capability) const override;
    void Release_AsymmetricKeyEd25519() {}
    void Release() override;

    virtual ~AsymmetricKeyEd25519();
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_ASYMMETRICKEYED25519_HPP
