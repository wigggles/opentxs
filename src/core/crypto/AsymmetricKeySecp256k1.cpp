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

#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"

#include "opentxs/core/app/App.hpp"
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1()
    : ot_super(proto::AKEYTYPE_SECP256K1, proto::KEYROLE_ERROR)
{
}

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1(const proto::KeyRole role)
    : ot_super(proto::AKEYTYPE_SECP256K1, role)
{
}

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1(
    const proto::AsymmetricKey& serializedKey)
        : ot_super(serializedKey)
{
}

AsymmetricKeySecp256k1::AsymmetricKeySecp256k1(const String& publicKey)
    : ot_super(proto::AKEYTYPE_SECP256K1, publicKey)
{
}

Ecdsa& AsymmetricKeySecp256k1::ECDSA() const
{
    return static_cast<Libsecp256k1&>(engine());
}

CryptoAsymmetric& AsymmetricKeySecp256k1::engine() const

{
    return App::Me().Crypto().SECP256K1();
}

void AsymmetricKeySecp256k1::Release()
{
    Release_AsymmetricKeySecp256k1(); // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

AsymmetricKeySecp256k1::~AsymmetricKeySecp256k1()
{
    Release_AsymmetricKeySecp256k1();

    ReleaseKeyLowLevel_Hook();
}
} // namespace opentxs
