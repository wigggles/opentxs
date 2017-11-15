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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/Libsodium.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
AsymmetricKeyEd25519::AsymmetricKeyEd25519()
    : ot_super(proto::AKEYTYPE_ED25519, proto::KEYROLE_ERROR)
{
}

AsymmetricKeyEd25519::AsymmetricKeyEd25519(const proto::KeyRole role)
    : ot_super(proto::AKEYTYPE_ED25519, role)
{
}

AsymmetricKeyEd25519::AsymmetricKeyEd25519(
    const proto::AsymmetricKey& serializedKey)
    : ot_super(serializedKey)
{
}

AsymmetricKeyEd25519::AsymmetricKeyEd25519(const String& publicKey)
    : ot_super(proto::AKEYTYPE_ED25519, publicKey)
{
}

Ecdsa& AsymmetricKeyEd25519::ECDSA() const
{
    return static_cast<Libsodium&>(engine());
}

CryptoAsymmetric& AsymmetricKeyEd25519::engine() const
{
    return OT::App().Crypto().ED25519();
}

void AsymmetricKeyEd25519::Release()
{
    Release_AsymmetricKeyEd25519();  // My own cleanup is performed here.

    // Next give the base class a chance to do the same...
    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

AsymmetricKeyEd25519::~AsymmetricKeyEd25519()
{
    Release_AsymmetricKeyEd25519();

    ReleaseKeyLowLevel_Hook();
}

bool AsymmetricKeyEd25519::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::AUTHENTICATE_CONNECTION): {

            return true;
        }
        default: {
            return ot_super::hasCapability(capability);
        }
    }
}
}  // namespace opentxs
